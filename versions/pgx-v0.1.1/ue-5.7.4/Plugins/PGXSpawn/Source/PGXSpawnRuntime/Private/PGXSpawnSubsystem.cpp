// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSpawnSubsystem.h"

#include "PGXSpawnConfig.h"
#include "PGXSpawnSettings.h"
#include "PGXWaveDefinition.h"
#include "PGXSpawnPoint.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Utils/PGXConfigResolution.h"
#include "HAL/PlatformTime.h"
#include "NativeGameplayTags.h"

namespace
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGX_Spawn_Condition_PlayerDistance, "PGX.Spawn.Condition.PlayerDistance");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGX_Spawn_Condition_MaxConcurrent, "PGX.Spawn.Condition.MaxConcurrent");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGX_Spawn_Condition_TimeOfDay, "PGX.Spawn.Condition.TimeOfDay");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGX_Spawn_Condition_GameplayTag, "PGX.Spawn.Condition.GameplayTag");
}

void UPGXSpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// EN: Settings-first resolution via PGX::ResolveSingleConfig<T>().
	//     Replaces the missing-Settings class gap with the canonical pattern.
	//     Falls back to a transient default (EnsureRuntimeObjects) if Settings
	//     has no ActiveConfig assigned — preserves existing test/dev behavior.
	// ES: Resolucion Settings-first via PGX::ResolveSingleConfig<T>().
	//     Reemplaza el gap de clase Settings faltante con el patron canonico.
	//     Cae a un default transient (EnsureRuntimeObjects) si Settings no
	//     tiene ActiveConfig asignado — preserva el comportamiento test/dev.
	// Add Settings class + adopt.
	if (const UPGXSpawnSettings* Settings = GetDefault<UPGXSpawnSettings>())
	{
		if (UPGXSpawnConfig* Resolved = PGX::ResolveSingleConfig<UPGXSpawnConfig>(
				Settings->ActiveConfig, TEXT("Spawn")))
		{
			SpawnConfig = Resolved;
		}
	}

	EnsureRuntimeObjects();
	SpawnRecords.Reset();
	LastCleanedSpawnRecords.Reset();
}

void UPGXSpawnSubsystem::Deinitialize()
{
	if (WaveTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(WaveTickerHandle);
		WaveTickerHandle.Reset();
	}
	ActiveWaves.Reset();

	// pool implementation: destroy pooled actors to avoid world leaks
	for (TPair<TSubclassOf<AActor>, TArray<TWeakObjectPtr<AActor>>>& Pair : ObjectPool)
	{
		for (TWeakObjectPtr<AActor>& WeakActor : Pair.Value)
		{
			if (AActor* Actor = WeakActor.Get())
			{
				Actor->Destroy();
			}
		}
	}
	ObjectPool.Reset();

	SpawnRecords.Reset();
	LastCleanedSpawnRecords.Reset();
	SpawnConfig = nullptr;
	Super::Deinitialize();
}

FPGXSpawnResult UPGXSpawnSubsystem::ValidateSpawnRequest(const FPGXSpawnRequest& Request) const
{
	EnsureRuntimeObjects();

	UClass* SpawnClass = Request.SpawnClass.Get();
	if (!SpawnClass)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: request rejected because SpawnClass is missing"));
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidSpawnClass, EPGXSpawnRequestStatus::Failed, TEXT("Spawn class is missing."));
	}

	if (!SpawnClass->IsChildOf(AActor::StaticClass()) || SpawnClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: request rejected because SpawnClass '%s' is not spawnable"), *GetNameSafe(SpawnClass));
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidSpawnClass, EPGXSpawnRequestStatus::Failed, TEXT("Spawn class is not a concrete actor class."));
	}

	if (Request.Transform.ContainsNaN() || Request.Transform.GetScale3D().IsNearlyZero())
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: request rejected because Transform is invalid"));
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidTransform, EPGXSpawnRequestStatus::Failed, TEXT("Spawn transform is invalid."));
	}

	// condition implementation: Condition evaluation (per-config GlobalConditions)
	if (SpawnConfig && SpawnConfig->GlobalConditions.Num() > 0)
	{
		if (!EvaluateConditions(Request, SpawnConfig->GlobalConditions))
		{
			PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: request rejected because GlobalConditions evaluation failed for class '%s'"),
				*SpawnClass->GetName());
			return FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidRequest, EPGXSpawnRequestStatus::Failed, TEXT("Spawn condition evaluation failed."));
		}
	}

	if (GetActiveSpawnCount() >= GetMaxConcurrentSpawnBudget())
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: request rejected because active spawn budget is exhausted"));
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::BudgetExceeded, EPGXSpawnRequestStatus::Failed, TEXT("Spawn budget exhausted."));
	}

	return FPGXSpawnResult::Success(FPGXSpawnRequestHandle(), EPGXSpawnRequestStatus::Queued, nullptr, TEXT("Spawn request validated."));
}

FPGXSpawnResult UPGXSpawnSubsystem::ExecuteSpawnRequest(const FPGXSpawnRequest& Request)
{
	EnsureRuntimeObjects();

	// Hard cap check (pool implementation)
	if (GetActiveSpawnCount() >= GetMaxConcurrentSpawnBudget())
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: ExecuteSpawnRequest rejected because budget is full (%d/%d)."),
			GetActiveSpawnCount(), GetMaxConcurrentSpawnBudget());
		UpdateBudgetTracking();
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::BudgetExceeded, EPGXSpawnRequestStatus::Failed, TEXT("Active spawn budget is full."));
	}

	// Try pool first (pool implementation)
	AActor* PooledActor = AcquireFromPool(Request.SpawnClass);
	if (PooledActor)
	{
		PGX_LOG_VERBOSE(LogPGX, TEXT("PGXSpawn: ExecuteSpawnRequest reused pooled actor '%s' for class '%s'"),
			*PooledActor->GetName(), *Request.SpawnClass->GetName());

		// Register the reused actor (need to teleport to Request.Transform manually since UWorld::SpawnActor path skipped)
		PooledActor->SetActorTransform(Request.Transform, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);

		const FPGXSpawnResult RegisterResult = RegisterSpawnRecord(Request, PooledActor);
		if (RegisterResult.bSuccess)
		{
			UpdateBudgetTracking();
			return CompleteSpawnRecord(RegisterResult.Handle, EPGXSpawnResultCode::Success, PooledActor, TEXT("Spawn from pool (reused)."));
		}
		// Registration failed — return actor to pool for next attempt
		ReleaseToPool(PooledActor);
		return RegisterResult;
	}

	// Fallback: spawn new (existing path)
	const FPGXSpawnResult Result = ExecuteSpawnRequestInWorld(GetWorld(), Request);
	UpdateBudgetTracking();
	return Result;
}

FPGXSpawnResult UPGXSpawnSubsystem::RegisterSpawnRecord(const FPGXSpawnRequest& Request, AActor* SpawnedActor)
{
	const FPGXSpawnResult ValidationResult = ValidateSpawnRequest(Request);
	if (!ValidationResult.bSuccess)
	{
		return ValidationResult;
	}

	const FPGXSpawnRequestHandle Handle = FPGXSpawnRequestHandle::NewHandle();
	FPGXSpawnRecord Record;
	Record.Handle = Handle;
	Record.Request = Request;
	Record.Status = EPGXSpawnRequestStatus::Running;
	Record.ResultCode = EPGXSpawnResultCode::Success;
	Record.SpawnedActor = SpawnedActor;
	Record.Message = SpawnedActor ? TEXT("Spawn actor registered.") : TEXT("Spawn request registered.");
	Record.CreatedTimeSeconds = FPlatformTime::Seconds();
	AppendLifecycleEvent(Record, EPGXSpawnLifecycleEventType::Requested, EPGXSpawnResultCode::Success, TEXT("Spawn request registered."));
	if (SpawnedActor)
	{
		AppendLifecycleEvent(Record, EPGXSpawnLifecycleEventType::Spawned, EPGXSpawnResultCode::Success, TEXT("Spawn actor attached to record."));
	}
	SpawnRecords.Add(Handle.Id, Record);
	UpdateBudgetTracking();

	return FPGXSpawnResult::Success(Handle, Record.Status, SpawnedActor, Record.Message);
}

FPGXSpawnResult UPGXSpawnSubsystem::CompleteSpawnRecord(FPGXSpawnRequestHandle Handle, EPGXSpawnResultCode ResultCode, AActor* SpawnedActor, FString Message)
{
	if (!Handle.IsValid())
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::RecordNotFound, EPGXSpawnRequestStatus::Failed, TEXT("Spawn handle is invalid."), Handle);
	}

	FPGXSpawnRecord* Record = SpawnRecords.Find(Handle.Id);
	if (!Record)
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::RecordNotFound, EPGXSpawnRequestStatus::Failed, TEXT("Spawn record was not found."), Handle);
	}

	if (!IsRecordActive(*Record))
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::AlreadyCompleted, Record->Status, TEXT("Spawn record is already inactive."), Handle);
	}

	Record->ResultCode = ResultCode;
	Record->Status = ResultCode == EPGXSpawnResultCode::Success ? EPGXSpawnRequestStatus::Completed : EPGXSpawnRequestStatus::Failed;
	Record->SpawnedActor = SpawnedActor ? SpawnedActor : Record->SpawnedActor;
	Record->Message = Message.IsEmpty() ? TEXT("Spawn record completed.") : MoveTemp(Message);
	Record->CompletedTimeSeconds = FPlatformTime::Seconds();
	AppendLifecycleEvent(*Record,
		Record->Status == EPGXSpawnRequestStatus::Completed ? EPGXSpawnLifecycleEventType::Completed : EPGXSpawnLifecycleEventType::Failed,
		ResultCode,
		Record->Message);

	if (Record->Status == EPGXSpawnRequestStatus::Completed)
	{
		UpdateBudgetTracking();
		return FPGXSpawnResult::Success(Handle, Record->Status, Record->SpawnedActor.Get(), Record->Message);
	}
	UpdateBudgetTracking();
	return FPGXSpawnResult::Failure(ResultCode, Record->Status, Record->Message, Handle);
}

FPGXSpawnResult UPGXSpawnSubsystem::CancelSpawnRecord(FPGXSpawnRequestHandle Handle, FString Message)
{
	if (!Handle.IsValid())
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::RecordNotFound, EPGXSpawnRequestStatus::Failed, TEXT("Spawn handle is invalid."), Handle);
	}

	FPGXSpawnRecord* Record = SpawnRecords.Find(Handle.Id);
	if (!Record)
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::RecordNotFound, EPGXSpawnRequestStatus::Failed, TEXT("Spawn record was not found."), Handle);
	}

	if (!IsRecordActive(*Record))
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::AlreadyCompleted, Record->Status, TEXT("Spawn record is already inactive."), Handle);
	}

	Record->Status = EPGXSpawnRequestStatus::Cancelled;
	Record->ResultCode = EPGXSpawnResultCode::Success;
	Record->Message = Message.IsEmpty() ? TEXT("Spawn record cancelled.") : MoveTemp(Message);
	Record->CompletedTimeSeconds = FPlatformTime::Seconds();
	AppendLifecycleEvent(*Record, EPGXSpawnLifecycleEventType::Cancelled, EPGXSpawnResultCode::Success, Record->Message);
	return FPGXSpawnResult::Success(Handle, Record->Status, Record->SpawnedActor.Get(), Record->Message);
}

int32 UPGXSpawnSubsystem::CleanupInactiveSpawnRecords()
{
	LastCleanedSpawnRecords.Reset();
	const int32 BeforeCount = SpawnRecords.Num();
	for (auto It = SpawnRecords.CreateIterator(); It; ++It)
	{
		if (!IsRecordActive(It.Value()))
		{
			AppendLifecycleEvent(It.Value(), EPGXSpawnLifecycleEventType::Cleanup, It.Value().ResultCode, TEXT("Spawn record cleaned up."));
			LastCleanedSpawnRecords.Add(It.Value());
			It.RemoveCurrent();
		}
	}
	return BeforeCount - SpawnRecords.Num();
}

bool UPGXSpawnSubsystem::HasSpawnRecord(FPGXSpawnRequestHandle Handle) const
{
	return Handle.IsValid() && SpawnRecords.Contains(Handle.Id);
}

int32 UPGXSpawnSubsystem::GetActiveSpawnCount() const
{
	int32 ActiveCount = 0;
	for (const TPair<FGuid, FPGXSpawnRecord>& Pair : SpawnRecords)
	{
		if (IsRecordActive(Pair.Value))
		{
			++ActiveCount;
		}
	}
	return ActiveCount;
}

int32 UPGXSpawnSubsystem::GetTotalSpawnRecordCount() const
{
	return SpawnRecords.Num();
}

TArray<FPGXSpawnRecord> UPGXSpawnSubsystem::GetSpawnRecordsSnapshot() const
{
	TArray<FPGXSpawnRecord> Snapshot;
	Snapshot.Reserve(SpawnRecords.Num());
	for (const TPair<FGuid, FPGXSpawnRecord>& Pair : SpawnRecords)
	{
		Snapshot.Add(Pair.Value);
	}
	Snapshot.Sort([](const FPGXSpawnRecord& Left, const FPGXSpawnRecord& Right)
	{
		return Left.CreatedTimeSeconds < Right.CreatedTimeSeconds;
	});
	return Snapshot;
}

TArray<FPGXSpawnRecord> UPGXSpawnSubsystem::GetLastCleanedSpawnRecordsSnapshot() const
{
	return LastCleanedSpawnRecords;
}

UPGXSpawnConfig* UPGXSpawnSubsystem::GetActiveSpawnConfig() const
{
	EnsureRuntimeObjects();
	return SpawnConfig;
}

#if WITH_DEV_AUTOMATION_TESTS
void UPGXSpawnSubsystem::InjectTestSpawnConfig(UPGXSpawnConfig* InConfig)
{
	SpawnConfig = InConfig;
	EnsureRuntimeObjects();
}

void UPGXSpawnSubsystem::ClearSpawnRecordsForTesting()
{
	SpawnRecords.Reset();
	LastCleanedSpawnRecords.Reset();
}

FPGXSpawnResult UPGXSpawnSubsystem::ExecuteSpawnRequestForTesting(UWorld* World, const FPGXSpawnRequest& Request)
{
	return ExecuteSpawnRequestInWorld(World, Request);
}

void UPGXSpawnSubsystem::SetForceNextSpawnActorFailureForTesting(bool bInForceFailure)
{
	bForceNextSpawnActorFailureForTesting = bInForceFailure;
}
#endif

void UPGXSpawnSubsystem::EnsureRuntimeObjects() const
{
	if (!SpawnConfig)
	{
		SpawnConfig = NewObject<UPGXSpawnConfig>(const_cast<UPGXSpawnSubsystem*>(this), UPGXSpawnConfig::StaticClass(), NAME_None, RF_Transient);
	}
}

bool UPGXSpawnSubsystem::IsRecordActive(const FPGXSpawnRecord& Record) const
{
	return Record.Status == EPGXSpawnRequestStatus::Queued || Record.Status == EPGXSpawnRequestStatus::Running;
}

int32 UPGXSpawnSubsystem::GetMaxConcurrentSpawnBudget() const
{
	EnsureRuntimeObjects();
	return SpawnConfig ? FMath::Max(0, SpawnConfig->MaxConcurrentActors) : 0;
}

FPGXSpawnResult UPGXSpawnSubsystem::ExecuteSpawnRequestInWorld(UWorld* World, const FPGXSpawnRequest& Request)
{
	const FPGXSpawnResult ValidationResult = ValidateSpawnRequest(Request);
	if (!ValidationResult.bSuccess)
	{
		return ValidationResult;
	}

	if (!IsValid(World))
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: request rejected because UWorld is invalid"));
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidWorld, EPGXSpawnRequestStatus::Failed, TEXT("Spawn world is invalid."));
	}

	const FPGXSpawnResult RegisterResult = RegisterSpawnRecord(Request, nullptr);
	if (!RegisterResult.bSuccess)
	{
		return RegisterResult;
	}

#if WITH_DEV_AUTOMATION_TESTS
	if (bForceNextSpawnActorFailureForTesting)
	{
		bForceNextSpawnActorFailureForTesting = false;
		return CompleteSpawnRecord(RegisterResult.Handle, EPGXSpawnResultCode::SpawnActorFailed, nullptr, TEXT("SpawnActor failure forced by automation seam."));
	}
#endif

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.Name = NAME_None;

	AActor* SpawnedActor = World->SpawnActor<AActor>(Request.SpawnClass.Get(), Request.Transform, SpawnParameters);
	FPGXSpawnRecord* Record = SpawnRecords.Find(RegisterResult.Handle.Id);
	if (!SpawnedActor)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: UWorld::SpawnActor returned null for class '%s'"), *GetNameSafe(Request.SpawnClass.Get()));
		return CompleteSpawnRecord(RegisterResult.Handle, EPGXSpawnResultCode::SpawnActorFailed, nullptr, TEXT("UWorld::SpawnActor returned null."));
	}

	if (Record)
	{
		Record->SpawnedActor = SpawnedActor;
		AppendLifecycleEvent(*Record, EPGXSpawnLifecycleEventType::Spawned, EPGXSpawnResultCode::Success, TEXT("UWorld::SpawnActor returned actor."));
	}

	return CompleteSpawnRecord(RegisterResult.Handle, EPGXSpawnResultCode::Success, SpawnedActor, TEXT("Actor spawned successfully."));
}

void UPGXSpawnSubsystem::AppendLifecycleEvent(FPGXSpawnRecord& Record, EPGXSpawnLifecycleEventType EventType, EPGXSpawnResultCode ResultCode, const FString& Message) const
{
	FPGXSpawnLifecycleEvent Event;
	Event.EventType = EventType;
	Event.ResultCode = ResultCode;
	Event.TimestampSeconds = FPlatformTime::Seconds();
	Event.Message = Message;
	Record.LifecycleEvents.Add(MoveTemp(Event));
}

// ========================================================================
// Wave scheduling and cancellation
// ========================================================================

FPGXSpawnResult UPGXSpawnSubsystem::StartWave(UPGXWaveDefinition* WaveDef)
{
	EnsureRuntimeObjects();

	if (!IsValid(WaveDef))
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidRequest, EPGXSpawnRequestStatus::Failed, TEXT("WaveDef is null."));
	}

	if (WaveDef->TotalSpawnCount <= 0)
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidRequest, EPGXSpawnRequestStatus::Failed, TEXT("WaveDef has TotalSpawnCount <= 0."));
	}

	if (GetActiveSpawnCount() >= GetMaxConcurrentSpawnBudget())
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: StartWave rejected for '%s' because budget is full (%d/%d)."),
			*WaveDef->WaveName.ToString(), GetActiveSpawnCount(), GetMaxConcurrentSpawnBudget());
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::BudgetExceeded, EPGXSpawnRequestStatus::Failed, TEXT("Active spawn budget is full."));
	}

	FPGXSpawnWaveRuntimeState NewWave;
	NewWave.WaveHandle = FPGXSpawnRequestHandle::NewHandle();
	NewWave.WaveDef = WaveDef;
	NewWave.WaveTag = WaveDef->WaveTag;
	NewWave.SpawnedSoFar = 0;
	NewWave.StartedTime = FPlatformTime::Seconds();
	NewWave.LastSpawnTime = 0.0;
	NewWave.Status = EPGXSpawnRequestStatus::Running;
	const int32 NewIndex = ActiveWaves.Add(MoveTemp(NewWave));

	// Schedule FTSTicker on first wave
	if (!WaveTickerHandle.IsValid())
	{
		WaveTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UPGXSpawnSubsystem::OnWaveTick),
			FMath::Max(WaveDef->SpawnInterval, 0.01f));
	}

	PGX_LOG_INFO(LogPGX, TEXT("PGXSpawn: Wave '%s' started (index=%d, handle=%s, total=%d, interval=%.2fs)"),
		*WaveDef->WaveName.ToString(), NewIndex, *NewWave.WaveHandle.Id.ToString(), WaveDef->TotalSpawnCount, WaveDef->SpawnInterval);

	return FPGXSpawnResult::Success(NewWave.WaveHandle, EPGXSpawnRequestStatus::Running, nullptr, TEXT("Wave started."));
}

FPGXSpawnResult UPGXSpawnSubsystem::CancelWave(FGameplayTag WaveTag)
{
	for (int32 Index = ActiveWaves.Num() - 1; Index >= 0; --Index)
	{
		if (ActiveWaves[Index].WaveTag == WaveTag)
		{
			PGX_LOG_INFO(LogPGX, TEXT("PGXSpawn: Wave with tag '%s' cancelled (handle=%s, spawned=%d)."),
				*WaveTag.ToString(), *ActiveWaves[Index].WaveHandle.Id.ToString(), ActiveWaves[Index].SpawnedSoFar);
			ActiveWaves[Index].Status = EPGXSpawnRequestStatus::Cancelled;
			ActiveWaves.RemoveAt(Index);
			return FPGXSpawnResult::Success(FPGXSpawnRequestHandle(), EPGXSpawnRequestStatus::Cancelled, nullptr, TEXT("Wave cancelled."));
		}
	}

	return FPGXSpawnResult::Failure(EPGXSpawnResultCode::RecordNotFound, EPGXSpawnRequestStatus::None, TEXT("No active wave with that tag."));
}

TArray<FPGXSpawnRecord> UPGXSpawnSubsystem::GetActiveWavesSnapshot() const
{
	TArray<FPGXSpawnRecord> Result;
	Result.Reserve(SpawnRecords.Num());

	for (const FPGXSpawnWaveRuntimeState& Wave : ActiveWaves)
	{
		for (const FPGXSpawnRequestHandle& Handle : Wave.SpawnedRecordHandles)
		{
			if (const FPGXSpawnRecord* Record = SpawnRecords.Find(Handle.Id))
			{
				Result.Add(*Record);
			}
		}
	}

	return Result;
}

bool UPGXSpawnSubsystem::OnWaveTick(float DeltaTime)
{
	if (ActiveWaves.Num() == 0)
	{
		// No more waves — remove ticker and stop
		WaveTickerHandle.Reset();
		return false;
	}

	const double Now = FPlatformTime::Seconds();

	// Iterate copy-by-value since we may modify ActiveWaves (completed waves removed)
	for (int32 Index = 0; Index < ActiveWaves.Num(); ++Index)
	{
		FPGXSpawnWaveRuntimeState& Wave = ActiveWaves[Index];
		UPGXWaveDefinition* WaveDef = Wave.WaveDef.Get();
		if (!WaveDef || WaveDef->TotalSpawnCount <= 0)
		{
			Wave.Status = EPGXSpawnRequestStatus::Failed;
			ActiveWaves.RemoveAt(Index--);
			continue;
		}

		// Re-check budget (other systems may have spawned since last tick)
		if (GetActiveSpawnCount() >= GetMaxConcurrentSpawnBudget())
		{
			PGX_LOG_VERBOSE(LogPGX, TEXT("PGXSpawn: Wave tick paused for '%s' because budget is full (%d/%d)."),
				*WaveDef->WaveName.ToString(), GetActiveSpawnCount(), GetMaxConcurrentSpawnBudget());
			continue;
		}

		// Check interval (skip first tick — fire immediately)
		if (Wave.LastSpawnTime > 0.0 && (Now - Wave.LastSpawnTime) < WaveDef->SpawnInterval)
		{
			continue;
		}

		// Build spawn request for this wave
		FPGXSpawnRequest Request;
		Request.SpawnClass = WaveDef->DefaultSpawnClass;
		Request.Transform = FTransform::Identity;  // Waves without an explicit spawn point use the identity transform
		Request.SourceTag = Wave.WaveTag;
		Request.Priority = 0;

		// Validate (will fail if DefaultSpawnClass is not set)
		const FPGXSpawnResult Validation = ValidateSpawnRequest(Request);
		if (!Validation.bSuccess)
		{
			PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: Wave tick for '%s' failed validation: %s"),
				*WaveDef->WaveName.ToString(), *Validation.Message);
			Wave.Status = EPGXSpawnRequestStatus::Failed;
			ActiveWaves.RemoveAt(Index--);
			continue;
		}

		// Execute (synchronous spawn)
		const FPGXSpawnResult Execution = ExecuteSpawnRequest(Request);
		Wave.SpawnedRecordHandles.Add(Execution.Handle);
		Wave.SpawnedSoFar++;
		Wave.LastSpawnTime = Now;

		PGX_LOG_VERBOSE(LogPGX, TEXT("PGXSpawn: Wave tick for '%s' spawned %d/%d (handle=%s)"),
			*WaveDef->WaveName.ToString(), Wave.SpawnedSoFar, WaveDef->TotalSpawnCount, *Execution.Handle.Id.ToString());

		// Check completion
		if (Wave.SpawnedSoFar >= WaveDef->TotalSpawnCount)
		{
			PGX_LOG_INFO(LogPGX, TEXT("PGXSpawn: Wave '%s' completed (spawned=%d)."),
				*WaveDef->WaveName.ToString(), Wave.SpawnedSoFar);
			Wave.Status = EPGXSpawnRequestStatus::Completed;
			ActiveWaves.RemoveAt(Index--);
		}
	}

	// Continue ticker if waves remain
	return ActiveWaves.Num() > 0;
}

int32 UPGXSpawnSubsystem::RegisterVolume(APGXSpawnVolume* Volume)
{
	// condition implementation: maintain volume registry, scan for SpawnPoints inside volume bounds
	return 0;
}

FPGXSpawnDebugSnapshot UPGXSpawnSubsystem::GetDebugSnapshot() const
{
	FPGXSpawnDebugSnapshot Snapshot;
	Snapshot.ActiveRecordCount = GetActiveSpawnCount();
	Snapshot.TotalRecordCount = GetTotalSpawnRecordCount();
	Snapshot.PeakConcurrentActors = PeakConcurrentActors;
	Snapshot.ActiveWaveCount = ActiveWaves.Num();
	Snapshot.PooledActorCount = 0;
	for (const TPair<TSubclassOf<AActor>, TArray<TWeakObjectPtr<AActor>>>& Pair : ObjectPool)
	{
		Snapshot.PooledActorCount += Pair.Value.Num();
	}
	Snapshot.LastCleanedRecords = GetLastCleanedSpawnRecordsSnapshot();
	Snapshot.DiagnosticsText = TEXT("Pool reuse and budget tracking active");
	return Snapshot;
}

void UPGXSpawnSubsystem::ReturnActorToPool(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}
	ReleaseToPool(Actor);
}

// ========================================================================
// EN: pool implementation — Object pool + budget tracking implementations
// ES: pool implementation — Implementaciones de object pool + budget tracking
// ========================================================================

AActor* UPGXSpawnSubsystem::AcquireFromPool(TSubclassOf<AActor> SpawnClass)
{
	if (!SpawnClass)
	{
		return nullptr;
	}

	EnsureRuntimeObjects();
	if (!SpawnConfig || !SpawnConfig->bUsePoolingForSpawns)
	{
		return nullptr;
	}

	TArray<TWeakObjectPtr<AActor>>* Pool = ObjectPool.Find(SpawnClass);
	if (!Pool || Pool->Num() == 0)
	{
		return nullptr;
	}

	// Pop the last entry (LIFO — better for cache locality)
	TWeakObjectPtr<AActor> WeakActor = Pool->Pop();
	AActor* Actor = WeakActor.Get();
	if (!Actor)
	{
		// Stale weak ref — recurse to try the next entry
		return AcquireFromPool(SpawnClass);
	}

	// Restore visibility + collision + tick
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

	return Actor;
}

void UPGXSpawnSubsystem::ReleaseToPool(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	EnsureRuntimeObjects();
	if (!SpawnConfig || !SpawnConfig->bUsePoolingForSpawns)
	{
		// Pool disabled — leave actor in world (or caller destroys)
		return;
	}

	UClass* ActorClass = Actor->GetClass();
	if (!ActorClass)
	{
		return;
	}

	// Hide + collision off + tick off
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);

	// Move to a safe location (e.g., far below the world) to avoid visual interference
	const FVector HiddenLocation(0.0, 0.0, -100000.0);
	Actor->SetActorLocation(HiddenLocation, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);

	// Add to pool
	TArray<TWeakObjectPtr<AActor>>& Pool = ObjectPool.FindOrAdd(ActorClass);
	Pool.Add(Actor);

	PGX_LOG_VERBOSE(LogPGX, TEXT("PGXSpawn: Actor '%s' released to pool (class='%s', pool size now %d)"),
		*Actor->GetName(), *ActorClass->GetName(), Pool.Num());
}

void UPGXSpawnSubsystem::UpdateBudgetTracking()
{
	const int32 CurrentCount = GetActiveSpawnCount();
	if (CurrentCount > PeakConcurrentActors)
	{
		PeakConcurrentActors = CurrentCount;
	}

	if (!SpawnConfig)
	{
		return;
	}

	const int32 MaxBudget = GetMaxConcurrentSpawnBudget();
	const int32 Threshold = FMath::FloorToInt(static_cast<float>(MaxBudget) * SpawnConfig->BudgetWarningThresholdPercent);
	if (CurrentCount >= Threshold && Threshold > 0)
	{
		PGX_LOG_VERBOSE(LogPGX, TEXT("PGXSpawn: Budget warning fired (count=%d, max=%d, threshold=%d)"),
			CurrentCount, MaxBudget, Threshold);
		OnBudgetWarning.Broadcast(CurrentCount, MaxBudget);
	}
}

// ========================================================================
// EN: condition implementation — Condition evaluator dispatcher
// ES: condition implementation — Dispatcher del evaluador de condiciones
// ========================================================================

bool UPGXSpawnSubsystem::EvaluateConditions(const FPGXSpawnRequest& Request, const TArray<FPGXSpawnConditionDefinition>& Conditions) const
{
	for (const FPGXSpawnConditionDefinition& Condition : Conditions)
	{
		if (!EvaluateCondition(Condition, Request))
		{
			PGX_LOG_VERBOSE(LogPGX, TEXT("PGXSpawn: condition '%s' failed for class '%s'"),
				*Condition.ConditionTag.ToString(),
				Request.SpawnClass ? *Request.SpawnClass->GetName() : TEXT("null"));
			return false;
		}
	}
	return true;
}

bool UPGXSpawnSubsystem::EvaluateCondition(const FPGXSpawnConditionDefinition& Condition, const FPGXSpawnRequest& Request) const
{
	// Cached tag resolution (no FName collisions since tags are FNames)
	const FGameplayTag PlayerDistanceTag = TAG_PGX_Spawn_Condition_PlayerDistance;
	const FGameplayTag MaxConcurrentTag = TAG_PGX_Spawn_Condition_MaxConcurrent;
	const FGameplayTag TimeOfDayTag = TAG_PGX_Spawn_Condition_TimeOfDay;
	const FGameplayTag GameplayTagCheckTag = TAG_PGX_Spawn_Condition_GameplayTag;

	// PlayerDistance — condition implementation STUB (Blueprint facade+ World integration)
	if (Condition.ConditionTag == PlayerDistanceTag)
	{
		const FPGXSpawnPlayerDistancePayload* Payload = Condition.Payload.GetPtr<FPGXSpawnPlayerDistancePayload>();
		if (!Payload)
		{
			return true;  // No payload = pass (extensible)
		}
		// Player-distance evaluation is not implemented; this condition currently passes.
		return true;
	}

	// MaxConcurrent — FULLY WORKING
	if (Condition.ConditionTag == MaxConcurrentTag)
	{
		const FPGXSpawnMaxConcurrentPayload* Payload = Condition.Payload.GetPtr<FPGXSpawnMaxConcurrentPayload>();
		if (!Payload)
		{
			return true;
		}
		int32 CurrentCount = 0;
		for (const TPair<FGuid, FPGXSpawnRecord>& Pair : SpawnRecords)
		{
			if (IsRecordActive(Pair.Value) && Pair.Value.Request.SourceTag == Request.SourceTag)
			{
				CurrentCount++;
			}
		}
		const bool bPass = CurrentCount <= Payload->Max;
		PGX_LOG_VERBOSE(LogPGX, TEXT("PGXSpawn: MaxConcurrent condition (SourceTag=%s, current=%d, max=%d) -> %s"),
			*Request.SourceTag.ToString(), CurrentCount, Payload->Max, bPass ? TEXT("PASS") : TEXT("FAIL"));
		return bPass;
	}

	// TimeOfDay — condition implementation STUB (Blueprint facade+ World integration)
	if (Condition.ConditionTag == TimeOfDayTag)
	{
		const FPGXSpawnTimeOfDayPayload* Payload = Condition.Payload.GetPtr<FPGXSpawnTimeOfDayPayload>();
		if (!Payload)
		{
			return true;
		}
		// Time-of-day evaluation is not implemented; this condition currently passes.
		return true;
	}

	// GameplayTag — FULLY WORKING
	if (Condition.ConditionTag == GameplayTagCheckTag)
	{
		const FPGXSpawnGameplayTagPayload* Payload = Condition.Payload.GetPtr<FPGXSpawnGameplayTagPayload>();
		if (!Payload)
		{
			return true;
		}
		const bool bPass = Request.SourceTag == Payload->RequiredTag;
		PGX_LOG_VERBOSE(LogPGX, TEXT("PGXSpawn: GameplayTag condition (request=%s, required=%s) -> %s"),
			*Request.SourceTag.ToString(), *Payload->RequiredTag.ToString(), bPass ? TEXT("PASS") : TEXT("FAIL"));
		return bPass;
	}

	// Unknown condition tag — pass by default (extensible framework)
	PGX_LOG_VERBOSE(LogPGX, TEXT("PGXSpawn: unknown condition tag '%s' (no evaluator registered) — passing by default"),
		*Condition.ConditionTag.ToString());
	return true;
}

// ========================================================================
// EN: console diagnostics — Console command helpers
// ES: console diagnostics — Helpers de comandos de consola
// ========================================================================

namespace
{
	FString RequestStatusToString(EPGXSpawnRequestStatus Status)
	{
		switch (Status)
		{
		case EPGXSpawnRequestStatus::None:		return TEXT("None");
		case EPGXSpawnRequestStatus::Queued:		return TEXT("Queued");
		case EPGXSpawnRequestStatus::Running:		return TEXT("Running");
		case EPGXSpawnRequestStatus::Completed:		return TEXT("Completed");
		case EPGXSpawnRequestStatus::Failed:		return TEXT("Failed");
		case EPGXSpawnRequestStatus::Cancelled:		return TEXT("Cancelled");
		case EPGXSpawnRequestStatus::Expired:		return TEXT("Expired");
		default: return TEXT("Unknown");
		}
	}

	FString ResultCodeToString(EPGXSpawnResultCode Code)
	{
		switch (Code)
		{
		case EPGXSpawnResultCode::Success:			return TEXT("Success");
		case EPGXSpawnResultCode::InvalidRequest:		return TEXT("InvalidRequest");
		case EPGXSpawnResultCode::InvalidSpawnClass:	return TEXT("InvalidSpawnClass");
		case EPGXSpawnResultCode::InvalidTransform:		return TEXT("InvalidTransform");
		case EPGXSpawnResultCode::BudgetExceeded:		return TEXT("BudgetExceeded");
		case EPGXSpawnResultCode::RecordNotFound:		return TEXT("RecordNotFound");
		case EPGXSpawnResultCode::AlreadyCompleted:		return TEXT("AlreadyCompleted");
		case EPGXSpawnResultCode::InternalError:		return TEXT("InternalError");
		case EPGXSpawnResultCode::InvalidWorld:			return TEXT("InvalidWorld");
		case EPGXSpawnResultCode::SpawnActorFailed:		return TEXT("SpawnActorFailed");
		default: return TEXT("Unknown");
		}
	}
}

// ========================================================================
// EN: console diagnostics — Console command handlers
// ES: console diagnostics — Handlers de comandos de consola
// ========================================================================

void UPGXSpawnSubsystem::HandleConsoleList(const TArray<FString>& Args)
{
	PGX_LOG_INFO(LogPGX, TEXT("PGXSpawn: === Active spawn records (%d) ==="), SpawnRecords.Num());
	for (const TPair<FGuid, FPGXSpawnRecord>& Pair : SpawnRecords)
	{
		const FPGXSpawnRecord& Record = Pair.Value;
		PGX_LOG_INFO(LogPGX, TEXT("  handle=%s status=%s class=%s src=%s msg=%s"),
			*Record.Handle.Id.ToString(),
			*RequestStatusToString(Record.Status),
			Record.Request.SpawnClass ? *Record.Request.SpawnClass->GetName() : TEXT("null"),
			*Record.Request.SourceTag.ToString(),
			*Record.Message);
	}
}

void UPGXSpawnSubsystem::HandleConsoleCleanup(const TArray<FString>& Args)
{
	const int32 CleanedCount = CleanupInactiveSpawnRecords();
	PGX_LOG_INFO(LogPGX, TEXT("PGXSpawn: Console cleanup removed %d inactive records."), CleanedCount);
}

void UPGXSpawnSubsystem::HandleConsoleBudget(const TArray<FString>& Args)
{
	const int32 Current = GetActiveSpawnCount();
	const int32 Max = GetMaxConcurrentSpawnBudget();
	PGX_LOG_INFO(LogPGX, TEXT("PGXSpawn: Budget state — current=%d / peak=%d / max=%d"), Current, PeakConcurrentActors, Max);
}

void UPGXSpawnSubsystem::HandleConsoleWaves(const TArray<FString>& Args)
{
	PGX_LOG_INFO(LogPGX, TEXT("PGXSpawn: === Active waves (%d) ==="), ActiveWaves.Num());
	for (int32 Index = 0; Index < ActiveWaves.Num(); ++Index)
	{
		const FPGXSpawnWaveRuntimeState& Wave = ActiveWaves[Index];
		PGX_LOG_INFO(LogPGX, TEXT("  wave[%d] handle=%s tag=%s spawned=%d status=%s"),
			Index,
			*Wave.WaveHandle.Id.ToString(),
			*Wave.WaveTag.ToString(),
			Wave.SpawnedSoFar,
			*RequestStatusToString(Wave.Status));
	}
}

void UPGXSpawnSubsystem::HandleConsoleTriggerPoint(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: pgx.spawn.triggerpoint requires a SpawnPoint name argument."));
		return;
	}
	const FString PointName = Args[0];

	UWorld* World = GetWorld();
	if (!World)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: pgx.spawn.triggerpoint failed — no World."));
		return;
	}

	for (TActorIterator<APGXSpawnPoint> It(World); It; ++It)
	{
		APGXSpawnPoint* Point = *It;
		if (Point && Point->GetName() == PointName)
		{
			// Spawn points currently expose validation and request construction; direct triggering is outside this subsystem path.
			PGX_LOG_INFO(LogPGX, TEXT("PGXSpawn: pgx.spawn.triggerpoint '%s' — APGXSpawnPoint exposes no TriggerSpawn operation."), *PointName);
			return;
		}
	}

	PGX_LOG_WARNING(LogPGX, TEXT("PGXSpawn: pgx.spawn.triggerpoint failed — no SpawnPoint named '%s' found in current World."),
		*PointName);
}

void UPGXSpawnSubsystem::HandleConsolePoolClear(const TArray<FString>& Args)
{
	int32 DestroyedCount = 0;
	for (TPair<TSubclassOf<AActor>, TArray<TWeakObjectPtr<AActor>>>& Pair : ObjectPool)
	{
		for (TWeakObjectPtr<AActor>& WeakActor : Pair.Value)
		{
			if (AActor* Actor = WeakActor.Get())
			{
				Actor->Destroy();
				DestroyedCount++;
			}
		}
		Pair.Value.Reset();
	}
	PGX_LOG_INFO(LogPGX, TEXT("PGXSpawn: Console pool.clear destroyed %d pooled actors."), DestroyedCount);
}
