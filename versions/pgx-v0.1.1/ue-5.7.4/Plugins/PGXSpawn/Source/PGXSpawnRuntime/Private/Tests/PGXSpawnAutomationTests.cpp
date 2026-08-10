// Copyright PGX Framework. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXSpawnConfig.h"
#include "PGXSpawnPoint.h"
#include "PGXSpawnRuntime.h"
#include "PGXSpawnSubsystem.h"
#include "PGXWaveDefinition.h"
#include "UPGXSpawnBlueprintLibrary.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/OutputDeviceNull.h"
#include "Modules/ModuleManager.h"
#include "NativeGameplayTags.h"
#include "Observability/PGXObservable.h"

namespace PGXSpawnAutomation
{
#define PGX_SPAWN_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	UPGXSpawnSubsystem* MakeSpawnSubsystem(int32 MaxConcurrentActors = 8)
	{
		UPGXSpawnSubsystem* Spawn = NewObject<UPGXSpawnSubsystem>(GetTransientPackage(), UPGXSpawnSubsystem::StaticClass(), NAME_None, RF_Transient);
		UPGXSpawnConfig* Config = NewObject<UPGXSpawnConfig>(GetTransientPackage(), UPGXSpawnConfig::StaticClass(), NAME_None, RF_Transient);
		Config->MaxConcurrentActors = MaxConcurrentActors;
		Spawn->InjectTestSpawnConfig(Config);
		Spawn->ClearSpawnRecordsForTesting();
		return Spawn;
	}

	FPGXSpawnRequest MakeRequest()
	{
		FPGXSpawnRequest Request;
		Request.SpawnClass = AActor::StaticClass();
		Request.Transform = FTransform::Identity;
		return Request;
	}

	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGX_Test_Spawn_SourceA, "PGX.Test.Spawn.SourceA");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGX_Test_Spawn_SourceB, "PGX.Test.Spawn.SourceB");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGX_Test_Spawn_UnknownCondition, "PGX.Test.Spawn.UnknownCondition");


	struct FScopedTestWorld
	{
		FScopedTestWorld()
			: World(UWorld::CreateWorld(EWorldType::Game, false))
		{
		}

		~FScopedTestWorld()
		{
			if (World)
			{
				World->DestroyWorld(false);
				World = nullptr;
			}
		}

		UWorld* Get() const { return World; }

	private:
		UWorld* World = nullptr;
	};

	struct FScopedGameInstance
	{
		FScopedGameInstance(FAutomationTestBase& Test, const TCHAR* WorldName)
		{
			if (!GEngine)
			{
				Test.AddError(TEXT("PGXSpawn automation setup failed: engine unavailable."));
				return;
			}

			GameInstance = NewObject<UGameInstance>(GEngine, UGameInstance::StaticClass(), NAME_None, RF_Transient);
			if (!GameInstance)
			{
				Test.AddError(TEXT("PGXSpawn automation setup failed: transient GameInstance creation failed."));
				return;
			}
			GameInstance->AddToRoot();
			GameInstance->InitializeStandalone(WorldName);
		}

		~FScopedGameInstance()
		{
			if (GameInstance)
			{
				GameInstance->Shutdown();
				GameInstance->RemoveFromRoot();
			}
		}

		FScopedGameInstance(const FScopedGameInstance&) = delete;
		FScopedGameInstance& operator=(const FScopedGameInstance&) = delete;

		UWorld* GetWorld() const { return GameInstance ? GameInstance->GetWorld() : nullptr; }
		UPGXSpawnSubsystem* GetSpawnSubsystem() const
		{
			UWorld* World = GetWorld();
			return World ? World->GetSubsystem<UPGXSpawnSubsystem>() : nullptr;
		}

	private:
		UGameInstance* GameInstance = nullptr;
	};

	const TCHAR* const ConsoleCommandNames[] =
	{
		TEXT("pgx.spawn.list"),
		TEXT("pgx.spawn.cleanup"),
		TEXT("pgx.spawn.budget"),
		TEXT("pgx.spawn.waves"),
		TEXT("pgx.spawn.triggerpoint"),
		TEXT("pgx.spawn.pool.clear")
	};

	bool HasLifecycleEvent(const FPGXSpawnRecord& Record, EPGXSpawnLifecycleEventType EventType)
	{
		return Record.LifecycleEvents.ContainsByPredicate([EventType](const FPGXSpawnLifecycleEvent& Event)
		{
			return Event.EventType == EventType;
		});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_ValidRequestRecordAutomationTest,
	"PGX.Spawn.Runtime.ValidRequestRecord", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_ValidRequestRecordAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	const FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();

	const FPGXSpawnResult Result = Spawn->RegisterSpawnRecord(Request);
	TestTrue(TEXT("ValidRequestRecord result succeeds"), Result.bSuccess);
	TestTrue(TEXT("ValidRequestRecord handle valid"), Result.Handle.IsValid());
	TestTrue(TEXT("ValidRequestRecord stored"), Spawn->HasSpawnRecord(Result.Handle));
	TestEqual(TEXT("ValidRequestRecord active count"), Spawn->GetActiveSpawnCount(), 1);
	TestEqual(TEXT("ValidRequestRecord total count"), Spawn->GetTotalSpawnRecordCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_InvalidClassTypedFailureAutomationTest,
	"PGX.Spawn.Runtime.InvalidClassTypedFailure", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_InvalidClassTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();
	Request.SpawnClass = nullptr;

	const FPGXSpawnResult Result = Spawn->ValidateSpawnRequest(Request);
	TestFalse(TEXT("InvalidClassTypedFailure result fails"), Result.bSuccess);
	TestTrue(TEXT("InvalidClassTypedFailure code"), Result.Code == EPGXSpawnResultCode::InvalidSpawnClass);
	TestFalse(TEXT("InvalidClassTypedFailure visible message"), Result.Message.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_InvalidTransformTypedFailureAutomationTest,
	"PGX.Spawn.Runtime.InvalidTransformTypedFailure", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_InvalidTransformTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();
	Request.Transform.SetScale3D(FVector::ZeroVector);

	const FPGXSpawnResult Result = Spawn->ValidateSpawnRequest(Request);
	TestFalse(TEXT("InvalidTransformTypedFailure result fails"), Result.bSuccess);
	TestTrue(TEXT("InvalidTransformTypedFailure code"), Result.Code == EPGXSpawnResultCode::InvalidTransform);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_BudgetTypedFailureAutomationTest,
	"PGX.Spawn.Runtime.BudgetTypedFailure", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_BudgetTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem(1);
	const FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();
	const FPGXSpawnResult FirstResult = Spawn->RegisterSpawnRecord(Request);
	const FPGXSpawnResult SecondResult = Spawn->ValidateSpawnRequest(Request);

	TestTrue(TEXT("BudgetTypedFailure setup succeeds"), FirstResult.bSuccess);
	TestFalse(TEXT("BudgetTypedFailure second request fails"), SecondResult.bSuccess);
	TestTrue(TEXT("BudgetTypedFailure code"), SecondResult.Code == EPGXSpawnResultCode::BudgetExceeded);
	TestEqual(TEXT("BudgetTypedFailure active count unchanged"), Spawn->GetActiveSpawnCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_ActiveRegistryCleanupAutomationTest,
	"PGX.Spawn.Runtime.ActiveRegistryCleanup", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_ActiveRegistryCleanupAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	const FPGXSpawnResult RegisterResult = Spawn->RegisterSpawnRecord(PGXSpawnAutomation::MakeRequest());
	const FPGXSpawnResult CompleteResult = Spawn->CompleteSpawnRecord(RegisterResult.Handle, EPGXSpawnResultCode::Success, nullptr, TEXT("Automation complete"));

	TestTrue(TEXT("ActiveRegistryCleanup complete succeeds"), CompleteResult.bSuccess);
	TestEqual(TEXT("ActiveRegistryCleanup active count after complete"), Spawn->GetActiveSpawnCount(), 0);
	TestEqual(TEXT("ActiveRegistryCleanup total before cleanup"), Spawn->GetTotalSpawnRecordCount(), 1);
	TestEqual(TEXT("ActiveRegistryCleanup removed count"), Spawn->CleanupInactiveSpawnRecords(), 1);
	TestEqual(TEXT("ActiveRegistryCleanup total after cleanup"), Spawn->GetTotalSpawnRecordCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_PointRequestPolicyAutomationTest,
	"PGX.Spawn.Runtime.PointRequestPolicy", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_PointRequestPolicyAutomationTest::RunTest(const FString& Parameters)
{
	APGXSpawnPoint* Point = NewObject<APGXSpawnPoint>(GetTransientPackage(), APGXSpawnPoint::StaticClass(), NAME_None, RF_Transient);
	const FPGXSpawnResult MissingClassResult = Point->ValidateSpawnPointPolicy();
	Point->SpawnClass = AActor::StaticClass();
	const FPGXSpawnResult ValidPolicyResult = Point->ValidateSpawnPointPolicy();
	const FPGXSpawnRequest Request = Point->BuildSpawnRequest();

	TestFalse(TEXT("PointRequestPolicy rejects missing class"), MissingClassResult.bSuccess);
	TestTrue(TEXT("PointRequestPolicy missing class code"), MissingClassResult.Code == EPGXSpawnResultCode::InvalidSpawnClass);
	TestTrue(TEXT("PointRequestPolicy valid policy succeeds"), ValidPolicyResult.bSuccess);
	TestTrue(TEXT("PointRequestPolicy request carries class"), Request.SpawnClass.Get() == AActor::StaticClass());
	TestFalse(TEXT("PointRequestPolicy request transform valid"), Request.Transform.ContainsNaN());
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_InvalidWorldTypedFailureAutomationTest,
	"PGX.Spawn.Integration.InvalidWorldTypedFailure", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_InvalidWorldTypedFailureAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	const FPGXSpawnResult Result = Spawn->ExecuteSpawnRequestForTesting(nullptr, PGXSpawnAutomation::MakeRequest());

	TestFalse(TEXT("InvalidWorldTypedFailure result fails"), Result.bSuccess);
	TestTrue(TEXT("InvalidWorldTypedFailure code"), Result.Code == EPGXSpawnResultCode::InvalidWorld);
	TestFalse(TEXT("InvalidWorldTypedFailure visible message"), Result.Message.IsEmpty());
	TestEqual(TEXT("InvalidWorldTypedFailure no records"), Spawn->GetTotalSpawnRecordCount(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_ExecuteSpawnActorLifecycleAutomationTest,
	"PGX.Spawn.Integration.ExecuteSpawnActorLifecycle", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_ExecuteSpawnActorLifecycleAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	PGXSpawnAutomation::FScopedGameInstance Fixture(*this, TEXT("PGXSpawnLifecycleAutomationWorld"));
	UWorld* World = Fixture.GetWorld();
	TestNotNull(TEXT("ExecuteSpawnActorLifecycle test world"), World);
	if (!World)
	{
		return false;
	}

	const FPGXSpawnResult Result = Spawn->ExecuteSpawnRequestForTesting(World, PGXSpawnAutomation::MakeRequest());
	TestTrue(TEXT("ExecuteSpawnActorLifecycle result succeeds"), Result.bSuccess);
	TestTrue(TEXT("ExecuteSpawnActorLifecycle handle valid"), Result.Handle.IsValid());
	TestNotNull(TEXT("ExecuteSpawnActorLifecycle spawned actor"), Result.SpawnedActor.Get());
	TestEqual(TEXT("ExecuteSpawnActorLifecycle active count completed"), Spawn->GetActiveSpawnCount(), 0);
	TestEqual(TEXT("ExecuteSpawnActorLifecycle total records retained"), Spawn->GetTotalSpawnRecordCount(), 1);

	const TArray<FPGXSpawnRecord> Snapshot = Spawn->GetSpawnRecordsSnapshot();
	TestEqual(TEXT("ExecuteSpawnActorLifecycle snapshot count"), Snapshot.Num(), 1);
	if (Snapshot.Num() == 1)
	{
		TestTrue(TEXT("ExecuteSpawnActorLifecycle requested event"), PGXSpawnAutomation::HasLifecycleEvent(Snapshot[0], EPGXSpawnLifecycleEventType::Requested));
		TestTrue(TEXT("ExecuteSpawnActorLifecycle spawned event"), PGXSpawnAutomation::HasLifecycleEvent(Snapshot[0], EPGXSpawnLifecycleEventType::Spawned));
		TestTrue(TEXT("ExecuteSpawnActorLifecycle completed event"), PGXSpawnAutomation::HasLifecycleEvent(Snapshot[0], EPGXSpawnLifecycleEventType::Completed));
		TestTrue(TEXT("ExecuteSpawnActorLifecycle weak actor valid"), Snapshot[0].SpawnedActor.IsValid());
	}

	if (Result.SpawnedActor)
	{
		World->DestroyActor(Result.SpawnedActor.Get());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_ForcedSpawnActorFailureLifecycleAutomationTest,
	"PGX.Spawn.Integration.ForcedSpawnActorFailureLifecycle", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_ForcedSpawnActorFailureLifecycleAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	PGXSpawnAutomation::FScopedTestWorld World;
	TestNotNull(TEXT("ForcedSpawnActorFailureLifecycle test world"), World.Get());
	if (!World.Get())
	{
		return false;
	}

	Spawn->SetForceNextSpawnActorFailureForTesting(true);
	const FPGXSpawnResult Result = Spawn->ExecuteSpawnRequestForTesting(World.Get(), PGXSpawnAutomation::MakeRequest());
	TestFalse(TEXT("ForcedSpawnActorFailureLifecycle result fails"), Result.bSuccess);
	TestTrue(TEXT("ForcedSpawnActorFailureLifecycle code"), Result.Code == EPGXSpawnResultCode::SpawnActorFailed);
	TestEqual(TEXT("ForcedSpawnActorFailureLifecycle active count"), Spawn->GetActiveSpawnCount(), 0);
	TestEqual(TEXT("ForcedSpawnActorFailureLifecycle record retained"), Spawn->GetTotalSpawnRecordCount(), 1);

	const TArray<FPGXSpawnRecord> Snapshot = Spawn->GetSpawnRecordsSnapshot();
	if (Snapshot.Num() == 1)
	{
		TestTrue(TEXT("ForcedSpawnActorFailureLifecycle requested event"), PGXSpawnAutomation::HasLifecycleEvent(Snapshot[0], EPGXSpawnLifecycleEventType::Requested));
		TestTrue(TEXT("ForcedSpawnActorFailureLifecycle failed event"), PGXSpawnAutomation::HasLifecycleEvent(Snapshot[0], EPGXSpawnLifecycleEventType::Failed));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_CancelCleanupLifecycleVisibilityAutomationTest,
	"PGX.Spawn.Integration.CancelCleanupLifecycleVisibility", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_CancelCleanupLifecycleVisibilityAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	const FPGXSpawnResult RegisterResult = Spawn->RegisterSpawnRecord(PGXSpawnAutomation::MakeRequest());
	const FPGXSpawnResult CancelResult = Spawn->CancelSpawnRecord(RegisterResult.Handle, TEXT("Automation cancel"));
	const int32 RemovedCount = Spawn->CleanupInactiveSpawnRecords();
	const TArray<FPGXSpawnRecord> Cleaned = Spawn->GetLastCleanedSpawnRecordsSnapshot();

	TestTrue(TEXT("CancelCleanupLifecycleVisibility setup succeeds"), RegisterResult.bSuccess);
	TestTrue(TEXT("CancelCleanupLifecycleVisibility cancel succeeds"), CancelResult.bSuccess);
	TestEqual(TEXT("CancelCleanupLifecycleVisibility removed count"), RemovedCount, 1);
	TestEqual(TEXT("CancelCleanupLifecycleVisibility cleaned snapshot count"), Cleaned.Num(), 1);
	if (Cleaned.Num() == 1)
	{
		TestTrue(TEXT("CancelCleanupLifecycleVisibility requested event"), PGXSpawnAutomation::HasLifecycleEvent(Cleaned[0], EPGXSpawnLifecycleEventType::Requested));
		TestTrue(TEXT("CancelCleanupLifecycleVisibility cancelled event"), PGXSpawnAutomation::HasLifecycleEvent(Cleaned[0], EPGXSpawnLifecycleEventType::Cancelled));
		TestTrue(TEXT("CancelCleanupLifecycleVisibility cleanup event"), PGXSpawnAutomation::HasLifecycleEvent(Cleaned[0], EPGXSpawnLifecycleEventType::Cleanup));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_ObservableDataAssetContractsAutomationTest,
	"PGX.Spawn.ObservableDataAssetContracts", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_ObservableDataAssetContractsAutomationTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("UPGXSpawnConfig implements IPGXObservable"),
		UPGXSpawnConfig::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));
	TestTrue(TEXT("UPGXWaveDefinition implements IPGXObservable"),
		UPGXWaveDefinition::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));

	UPGXSpawnConfig* Config = NewObject<UPGXSpawnConfig>(GetTransientPackage(), UPGXSpawnConfig::StaticClass(), NAME_None, RF_Transient);
	UPGXWaveDefinition* Wave = NewObject<UPGXWaveDefinition>(GetTransientPackage(), UPGXWaveDefinition::StaticClass(), NAME_None, RF_Transient);
	TestNotNull(TEXT("Observable spawn config instance"), Config);
	TestNotNull(TEXT("Observable wave definition instance"), Wave);
	if (!Config || !Wave)
	{
		return false;
	}

	Wave->WaveName = TEXT("AutomationWave");

	const FPGXJsonValue ConfigJson = Config->ToJson();
	const FPGXJsonValue WaveJson = Wave->ToJson();
	TestFalse(TEXT("Config ToJson returns non-empty envelope"), ConfigJson.IsEmpty());
	TestFalse(TEXT("Wave ToJson returns non-empty envelope"), WaveJson.IsEmpty());
	TestTrue(TEXT("Config data includes MaxConcurrentActors"), ConfigJson.JsonString.Contains(TEXT("\"MaxConcurrentActors\"")));
	TestTrue(TEXT("Wave data includes WaveName"), WaveJson.JsonString.Contains(TEXT("\"WaveName\":\"AutomationWave\"")));

	const FPGXSchemaDescriptor ConfigDescriptor = Config->GetSchemaDescriptor();
	const FPGXSchemaDescriptor WaveDescriptor = Wave->GetSchemaDescriptor();
	TestEqual(TEXT("Config descriptor type"), ConfigDescriptor.TypeName, UPGXSpawnConfig::StaticClass()->GetFName());
	TestEqual(TEXT("Wave descriptor type"), WaveDescriptor.TypeName, UPGXWaveDefinition::StaticClass()->GetFName());
	TestTrue(TEXT("Config descriptor exposes fields"), ConfigDescriptor.Fields.Num() >= 6);
	TestTrue(TEXT("Wave descriptor exposes fields"), WaveDescriptor.Fields.Num() >= 3);

	TestFalse(TEXT("Config FromJson empty payload visibly fails"), Config->FromJson(FPGXJsonValue()).bValid);
	TestFalse(TEXT("Wave FromJson empty payload visibly fails"), Wave->FromJson(FPGXJsonValue()).bValid);
	return true;
}

// ========================================================================
// EN: runtime behavior tests (Wave scheduler, Budget/pool, Conditions, BlueprintLibrary)
// ES: Tests runtime behavior (Wave scheduler, Budget/pool, Conditions, BlueprintLibrary)
// ========================================================================

// ---- Wave scheduler (wave implementation) ----

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_WaveStart_ValidWaveAddsActiveCountAutomationTest,
	"PGX.Spawn.Runtime.WaveStart.ValidWaveAddsActiveCount", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_WaveStart_ValidWaveAddsActiveCountAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	UPGXWaveDefinition* Wave = NewObject<UPGXWaveDefinition>(GetTransientPackage(), UPGXWaveDefinition::StaticClass(), NAME_None, RF_Transient);
	Wave->WaveName = FName(TEXT("PGXTest_WaveStart"));
	Wave->TotalSpawnCount = 5;
	Wave->SpawnInterval = 1.0f;

	const FPGXSpawnResult Result = Spawn->StartWave(Wave);
	TestTrue(TEXT("WaveStart_ValidWaveAddsActiveCount result"), Result.bSuccess);
	TestEqual(TEXT("WaveStart_ValidWaveAddsActiveCount DebugSnapshot count"), Spawn->GetDebugSnapshot().ActiveWaveCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_WaveStart_NullWaveDefRejectsAutomationTest,
	"PGX.Spawn.Runtime.WaveStart.NullWaveDefRejects", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_WaveStart_NullWaveDefRejectsAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	const FPGXSpawnResult Result = Spawn->StartWave(nullptr);
	TestFalse(TEXT("WaveStart_NullWaveDefRejects fails"), Result.bSuccess);
	TestEqual(TEXT("WaveStart_NullWaveDefRejects no active wave"), Spawn->GetDebugSnapshot().ActiveWaveCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_WaveStart_ZeroTotalRejectsAutomationTest,
	"PGX.Spawn.Runtime.WaveStart.ZeroTotalRejects", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_WaveStart_ZeroTotalRejectsAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	UPGXWaveDefinition* Wave = NewObject<UPGXWaveDefinition>(GetTransientPackage(), UPGXWaveDefinition::StaticClass(), NAME_None, RF_Transient);
	Wave->TotalSpawnCount = 0;
	Wave->SpawnInterval = 1.0f;

	const FPGXSpawnResult Result = Spawn->StartWave(Wave);
	TestFalse(TEXT("WaveStart_ZeroTotalRejects fails"), Result.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_CancelWave_AfterStartRemovesActiveCountAutomationTest,
	"PGX.Spawn.Runtime.CancelWave.AfterStartRemovesActiveCount", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_CancelWave_AfterStartRemovesActiveCountAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	UPGXWaveDefinition* Wave = NewObject<UPGXWaveDefinition>(GetTransientPackage(), UPGXWaveDefinition::StaticClass(), NAME_None, RF_Transient);
	Wave->WaveName = FName(TEXT("PGXTest_CancelWave"));
	Wave->WaveTag = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.Spawn.CancelWave")), false);
	Wave->TotalSpawnCount = 5;
	Wave->SpawnInterval = 1.0f;

	Spawn->StartWave(Wave);
	TestEqual(TEXT("CancelWave_AfterStartRemovesActiveCount before cancel"), Spawn->GetDebugSnapshot().ActiveWaveCount, 1);

	const FPGXSpawnResult CancelResult = Spawn->CancelWave(Wave->WaveTag);
	TestTrue(TEXT("CancelWave_AfterStartRemovesActiveCount result"), CancelResult.bSuccess);
	TestEqual(TEXT("CancelWave_AfterStartRemovesActiveCount after cancel"), Spawn->GetDebugSnapshot().ActiveWaveCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_CancelWave_NoActiveWaveReturnsNotFoundAutomationTest,
	"PGX.Spawn.Runtime.CancelWave.NoActiveWaveReturnsNotFound", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_CancelWave_NoActiveWaveReturnsNotFoundAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	const FPGXSpawnResult Result = Spawn->CancelWave(FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.NonExistent")), false));
	TestFalse(TEXT("CancelWave_NoActiveWaveReturnsNotFound result"), Result.bSuccess);
	TestEqual(TEXT("CancelWave_NoActiveWaveReturnsNotFound code"), static_cast<int32>(Result.Code), static_cast<int32>(EPGXSpawnResultCode::RecordNotFound));
	return true;
}

// ---- Condition evaluator (condition implementation) ----

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_EvaluateCondition_MaxConcurrentUnderLimitPassesAutomationTest,
	"PGX.Spawn.Runtime.Condition.MaxConcurrentUnderLimitPasses", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_EvaluateCondition_MaxConcurrentUnderLimitPassesAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();
	Request.SourceTag = PGXSpawnAutomation::TAG_PGX_Test_Spawn_SourceA;

	FPGXSpawnConditionDefinition Cond;
	Cond.ConditionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Spawn.Condition.MaxConcurrent")), false);
	FPGXSpawnMaxConcurrentPayload Payload;
	Payload.Max = 100;  // 100 max, 0 active = pass
	Cond.Payload = FInstancedStruct(FInstancedStruct::Make(Payload));

	TestTrue(TEXT("Condition.MaxConcurrentUnderLimitPasses"), Spawn->EvaluateCondition(Cond, Request));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_EvaluateCondition_MaxConcurrentOverLimitFailsAutomationTest,
	"PGX.Spawn.Runtime.Condition.MaxConcurrentOverLimitFails", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_EvaluateCondition_MaxConcurrentOverLimitFailsAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem(2);
	FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();
	Request.SourceTag = PGXSpawnAutomation::TAG_PGX_Test_Spawn_SourceA;

	// Register 2 records (fills budget)
	for (int32 Index = 0; Index < 2; ++Index)
	{
		Spawn->RegisterSpawnRecord(Request);
	}

	FPGXSpawnConditionDefinition Cond;
	Cond.ConditionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Spawn.Condition.MaxConcurrent")), false);
	FPGXSpawnMaxConcurrentPayload Payload;
	Payload.Max = 1;  // 1 max, 2 active = fail
	Cond.Payload = FInstancedStruct(FInstancedStruct::Make(Payload));

	TestFalse(TEXT("Condition.MaxConcurrentOverLimitFails"), Spawn->EvaluateCondition(Cond, Request));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_EvaluateCondition_GameplayTagMatchPassesAutomationTest,
	"PGX.Spawn.Runtime.Condition.GameplayTagMatchPasses", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_EvaluateCondition_GameplayTagMatchPassesAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();
	const FGameplayTag MatchTag = PGXSpawnAutomation::TAG_PGX_Test_Spawn_SourceA;
	Request.SourceTag = MatchTag;

	FPGXSpawnConditionDefinition Cond;
	Cond.ConditionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Spawn.Condition.GameplayTag")), false);
	FPGXSpawnGameplayTagPayload Payload;
	Payload.RequiredTag = MatchTag;
	Cond.Payload = FInstancedStruct(FInstancedStruct::Make(Payload));

	TestTrue(TEXT("Condition.GameplayTagMatchPasses"), Spawn->EvaluateCondition(Cond, Request));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_EvaluateCondition_GameplayTagMismatchFailsAutomationTest,
	"PGX.Spawn.Runtime.Condition.GameplayTagMismatchFails", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_EvaluateCondition_GameplayTagMismatchFailsAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();
	Request.SourceTag = PGXSpawnAutomation::TAG_PGX_Test_Spawn_SourceA;

	FPGXSpawnConditionDefinition Cond;
	Cond.ConditionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Spawn.Condition.GameplayTag")), false);
	FPGXSpawnGameplayTagPayload Payload;
	Payload.RequiredTag = PGXSpawnAutomation::TAG_PGX_Test_Spawn_SourceB;
	Cond.Payload = FInstancedStruct(FInstancedStruct::Make(Payload));

	TestFalse(TEXT("Condition.GameplayTagMismatchFails"), Spawn->EvaluateCondition(Cond, Request));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_EvaluateCondition_UnknownTagPassesAutomationTest,
	"PGX.Spawn.Runtime.Condition.UnknownTagPasses", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_EvaluateCondition_UnknownTagPassesAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();

	FPGXSpawnConditionDefinition Cond;
	Cond.ConditionTag = PGXSpawnAutomation::TAG_PGX_Test_Spawn_UnknownCondition;
	// Empty payload

	TestTrue(TEXT("Condition.UnknownTagPasses extensible default"), Spawn->EvaluateCondition(Cond, Request));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_EvaluateConditions_EmptyArrayAlwaysPassesAutomationTest,
	"PGX.Spawn.Runtime.Conditions.EmptyArrayAlwaysPasses", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_EvaluateConditions_EmptyArrayAlwaysPassesAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();
	const TArray<FPGXSpawnConditionDefinition> Empty;
	TestTrue(TEXT("Conditions.EmptyArrayAlwaysPasses"), Spawn->EvaluateConditions(Request, Empty));
	return true;
}

// ---- Debug snapshot (pool implementation) ----

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_GetDebugSnapshot_ActiveWaveCountReflectsStateAutomationTest,
	"PGX.Spawn.Runtime.DebugSnapshot.ActiveWaveCountReflectsState", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_GetDebugSnapshot_ActiveWaveCountReflectsStateAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	const FPGXSpawnDebugSnapshot Empty = Spawn->GetDebugSnapshot();
	TestEqual(TEXT("DebugSnapshot.ActiveWaveCountReflectsState initial count"), Empty.ActiveWaveCount, 0);

	UPGXWaveDefinition* Foundation = NewObject<UPGXWaveDefinition>(GetTransientPackage(), UPGXWaveDefinition::StaticClass(), NAME_None, RF_Transient);
	Foundation->TotalSpawnCount = 10;
	Foundation->WaveTag = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.Spawn.DbgWave1")), false);
	Foundation->SpawnInterval = 1.0f;
	Spawn->StartWave(Foundation);

	UPGXWaveDefinition* Configuration = NewObject<UPGXWaveDefinition>(GetTransientPackage(), UPGXWaveDefinition::StaticClass(), NAME_None, RF_Transient);
	Configuration->TotalSpawnCount = 10;
	Configuration->WaveTag = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.Spawn.DbgWave2")), false);
	Configuration->SpawnInterval = 1.0f;
	Spawn->StartWave(Configuration);

	const FPGXSpawnDebugSnapshot After = Spawn->GetDebugSnapshot();
	TestEqual(TEXT("DebugSnapshot.ActiveWaveCountReflectsState after 2 starts"), After.ActiveWaveCount, 2);
	TestEqual(TEXT("DebugSnapshot.ActiveRecordCountReflectsState after 2 starts"), After.ActiveRecordCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_GetDebugSnapshot_PeakConcurrentTracksAutomationTest,
	"PGX.Spawn.Runtime.DebugSnapshot.PeakConcurrentTracks", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_GetDebugSnapshot_PeakConcurrentTracksAutomationTest::RunTest(const FString& Parameters)
{
	UPGXSpawnSubsystem* Spawn = PGXSpawnAutomation::MakeSpawnSubsystem();
	FPGXSpawnRequest Request = PGXSpawnAutomation::MakeRequest();
	Request.SourceTag = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.Spawn.PeakTrack")), false);

	for (int32 Index = 0; Index < 3; ++Index)
	{
		Spawn->RegisterSpawnRecord(Request);
	}

	const FPGXSpawnDebugSnapshot Snap = Spawn->GetDebugSnapshot();
	TestEqual(TEXT("DebugSnapshot.PeakConcurrentTracks after 3 records"), Snap.PeakConcurrentActors, 3);
	TestEqual(TEXT("DebugSnapshot.PeakConcurrentTracks active"), Snap.ActiveRecordCount, 3);
	return true;
}

// ---- BlueprintLibrary (Blueprint facade) ----

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_BlueprintLibrary_IsValidSpawnRequestBasicChecksAutomationTest,
	"PGX.Spawn.Runtime.BlueprintLibrary.IsValidSpawnRequestBasicChecks", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_BlueprintLibrary_IsValidSpawnRequestBasicChecksAutomationTest::RunTest(const FString& Parameters)
{
	FPGXSpawnRequest Valid = PGXSpawnAutomation::MakeRequest();
	TestTrue(TEXT("BlueprintLibrary.IsValidSpawnRequest valid request"), UPGXSpawnBlueprintLibrary::IsValidSpawnRequest(Valid));

	FPGXSpawnRequest NullClass = Valid;
	NullClass.SpawnClass = nullptr;
	TestFalse(TEXT("BlueprintLibrary.IsValidSpawnRequest null class"), UPGXSpawnBlueprintLibrary::IsValidSpawnRequest(NullClass));

	FPGXSpawnRequest NaNTransform = Valid;
	NaNTransform.Transform = FTransform(FQuat(0, 0, 0, NAN), FVector::ZeroVector, FVector::OneVector);
	TestFalse(TEXT("BlueprintLibrary.IsValidSpawnRequest NaN transform"), UPGXSpawnBlueprintLibrary::IsValidSpawnRequest(NaNTransform));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_BlueprintLibrary_MakeSpawnRequestBuildsCorrectlyAutomationTest,
	"PGX.Spawn.Runtime.BlueprintLibrary.MakeSpawnRequestBuildsCorrectly", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_BlueprintLibrary_MakeSpawnRequestBuildsCorrectlyAutomationTest::RunTest(const FString& Parameters)
{
	const FTransform T(FQuat::Identity, FVector(100, 200, 300), FVector::OneVector);
	const FGameplayTag Src = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.Test.Spawn.BPLMake")), false);
	const FPGXSpawnRequest Req = UPGXSpawnBlueprintLibrary::MakeSpawnRequest(AActor::StaticClass(), T, Src, 42);
	TestEqual(TEXT("BlueprintLibrary.MakeSpawnRequest Class"), Req.SpawnClass.Get(), AActor::StaticClass());
	TestEqual(TEXT("BlueprintLibrary.MakeSpawnRequest Priority"), Req.Priority, 42);
	TestEqual(TEXT("BlueprintLibrary.MakeSpawnRequest SourceTag"), Req.SourceTag, Src);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_BlueprintLibrary_ResultCodeToStringCoversAllAutomationTest,
	"PGX.Spawn.Runtime.BlueprintLibrary.ResultCodeToStringCoversAll", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_BlueprintLibrary_ResultCodeToStringCoversAllAutomationTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("BlueprintLibrary.ResultCodeToString Success"),
		UPGXSpawnBlueprintLibrary::ResultCodeToString(EPGXSpawnResultCode::Success), FString(TEXT("Success")));
	TestEqual(TEXT("BlueprintLibrary.ResultCodeToString BudgetExceeded"),
		UPGXSpawnBlueprintLibrary::ResultCodeToString(EPGXSpawnResultCode::BudgetExceeded), FString(TEXT("Budget Exceeded")));
	TestEqual(TEXT("BlueprintLibrary.ResultCodeToString SpawnActorFailed"),
		UPGXSpawnBlueprintLibrary::ResultCodeToString(EPGXSpawnResultCode::SpawnActorFailed), FString(TEXT("Spawn Actor Failed")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_ConsoleWorldRoutingAutomationTest,
	"PGX.Spawn.Console.WorldRouting", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_ConsoleWorldRoutingAutomationTest::RunTest(const FString& Parameters)
{
	IConsoleManager& Console = IConsoleManager::Get();
	TArray<IConsoleObject*> OriginalCommands;
	for (const TCHAR* CommandName : PGXSpawnAutomation::ConsoleCommandNames)
	{
		IConsoleObject* Command = Console.FindConsoleObject(CommandName);
		TestNotNull(FString::Printf(TEXT("%s is registered before world creation"), CommandName), Command);
		OriginalCommands.Add(Command);
	}

	FOutputDeviceNull Output;
	for (int32 FixtureIndex = 0; FixtureIndex < 2; ++FixtureIndex)
	{
		PGXSpawnAutomation::FScopedGameInstance Fixture(
			*this,
			FixtureIndex == 0 ? TEXT("PGXSpawnConsoleWorldA") : TEXT("PGXSpawnConsoleWorldB"));
		UPGXSpawnSubsystem* Spawn = Fixture.GetSpawnSubsystem();
		TestNotNull(FString::Printf(TEXT("World %d owns a Spawn subsystem"), FixtureIndex), Spawn);
		if (!Spawn)
		{
			return false;
		}

		for (int32 CommandIndex = 0; CommandIndex < UE_ARRAY_COUNT(PGXSpawnAutomation::ConsoleCommandNames); ++CommandIndex)
		{
			TestTrue(FString::Printf(TEXT("World %d retains module-owned %s"), FixtureIndex, PGXSpawnAutomation::ConsoleCommandNames[CommandIndex]),
				Console.FindConsoleObject(PGXSpawnAutomation::ConsoleCommandNames[CommandIndex]) == OriginalCommands[CommandIndex]);
		}

		const int32 RecordCount = FixtureIndex + 1;
		for (int32 RecordIndex = 0; RecordIndex < RecordCount; ++RecordIndex)
		{
			const FPGXSpawnResult Record = Spawn->RegisterSpawnRecord(PGXSpawnAutomation::MakeRequest());
			TestTrue(FString::Printf(TEXT("World %d record %d registered"), FixtureIndex, RecordIndex), Record.bSuccess);
			Spawn->CompleteSpawnRecord(Record.Handle);
		}
		TestEqual(FString::Printf(TEXT("World %d starts with its own records"), FixtureIndex), Spawn->GetTotalSpawnRecordCount(), RecordCount);
		TestTrue(FString::Printf(TEXT("cleanup dispatch accepted for world %d"), FixtureIndex),
			Console.ProcessUserConsoleInput(TEXT("pgx.spawn.cleanup"), Output, Fixture.GetWorld()));
		TestEqual(FString::Printf(TEXT("world %d command cleans its supplied subsystem"), FixtureIndex), Spawn->GetTotalSpawnRecordCount(), 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSpawn_ConsoleModuleOwnershipAutomationTest,
	"PGX.Spawn.Console.ModuleOwnership", PGX_SPAWN_AUTOMATION_FLAGS)
bool FPGXSpawn_ConsoleModuleOwnershipAutomationTest::RunTest(const FString& Parameters)
{
	IConsoleManager& Console = IConsoleManager::Get();
	IConsoleCommand* Sentinel = Console.RegisterConsoleCommand(
		TEXT("pgx.spawn.ownership.test.sentinel"),
		TEXT("PGXSpawn module ownership automation sentinel."),
		FConsoleCommandDelegate::CreateLambda([]() {}),
		ECVF_Default);
	TestNotNull(TEXT("foreign sentinel registered"), Sentinel);

	FPGXSpawnRuntimeModule& Module = FModuleManager::GetModuleChecked<FPGXSpawnRuntimeModule>(TEXT("PGXSpawnRuntime"));
	Module.ShutdownModule();
	for (const TCHAR* CommandName : PGXSpawnAutomation::ConsoleCommandNames)
	{
		TestNull(FString::Printf(TEXT("Shutdown removes module-owned %s"), CommandName), Console.FindConsoleObject(CommandName));
	}
	TestTrue(TEXT("Shutdown preserves a foreign console object"),
		Console.FindConsoleObject(TEXT("pgx.spawn.ownership.test.sentinel")) == Sentinel);

	Module.StartupModule();
	for (const TCHAR* CommandName : PGXSpawnAutomation::ConsoleCommandNames)
	{
		TestNotNull(FString::Printf(TEXT("Startup restores %s exactly once"), CommandName), Console.FindConsoleObject(CommandName));
	}
	if (Sentinel)
	{
		Console.UnregisterConsoleObject(Sentinel);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
