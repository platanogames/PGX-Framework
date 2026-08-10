// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerBase.h"
#include "EventHandler/PGXEventHandlerConfig.h"
#include "EventHandler/PGXEventHandlerSettings.h"
#include "EventHandler/PGXEventHandlerLog.h"
#include "EventHandler/Tags/PGXEventHandlerTags.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"
#include "Utils/PGXConfigResolution.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "HAL/IConsoleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"

namespace
{
namespace PGXEventHandlerReason
{
const FName ValidationInvalidEventTag(TEXT("PGX.EventHandler.Validation.InvalidEventTag"));
const FName ValidationPayloadMismatch(TEXT("PGX.EventHandler.Validation.PayloadMismatch"));
const FName ValidationHandlerDisabled(TEXT("PGX.EventHandler.Validation.HandlerDisabled"));
const FName ValidationCanExecuteFalse(TEXT("PGX.EventHandler.Validation.CanExecuteFalse"));
const FName ResultHandlerNotFound(TEXT("PGX.EventHandler.Result.HandlerNotFound"));
const FName ResultAcquireHandlerFailed(TEXT("PGX.EventHandler.Result.AcquireHandlerFailed"));
const FName ResultChainBudgetExceeded(TEXT("PGX.EventHandler.Result.ChainBudgetExceeded"));
const FName ResultChainCycle(TEXT("PGX.EventHandler.Result.ChainCycle"));
}

FString PGXPayloadTypeName(const FInstancedStruct& Payload)
{
	const UScriptStruct* PayloadStruct = Payload.GetScriptStruct();
	return PayloadStruct ? PayloadStruct->GetName() : FString(TEXT("None"));
}

bool PGXPayloadTypeMatchesExpected(const UScriptStruct* PayloadStruct, const FString& ExpectedPayloadType)
{
	if (ExpectedPayloadType.IsEmpty())
	{
		return true;
	}

	if (!PayloadStruct)
	{
		return false;
	}

	const FString ActualName = PayloadStruct->GetName();
	const FString ActualPath = PayloadStruct->GetPathName();
	return ActualName.Equals(ExpectedPayloadType, ESearchCase::IgnoreCase)
		|| ActualPath.Equals(ExpectedPayloadType, ESearchCase::IgnoreCase);
}

FGameplayTag PGXReasonTag(FName ReasonName)
{
	if (ReasonName == PGXEventHandlerReason::ValidationInvalidEventTag)
	{
		return TAG_PGX_EventHandler_Validation_InvalidEventTag.GetTag();
	}
	if (ReasonName == PGXEventHandlerReason::ValidationPayloadMismatch)
	{
		return TAG_PGX_EventHandler_Validation_PayloadMismatch.GetTag();
	}
	if (ReasonName == PGXEventHandlerReason::ValidationHandlerDisabled)
	{
		return TAG_PGX_EventHandler_Validation_HandlerDisabled.GetTag();
	}
	if (ReasonName == PGXEventHandlerReason::ValidationCanExecuteFalse)
	{
		return TAG_PGX_EventHandler_Validation_CanExecuteFalse.GetTag();
	}
	if (ReasonName == PGXEventHandlerReason::ResultHandlerNotFound)
	{
		return TAG_PGX_EventHandler_Result_HandlerNotFound.GetTag();
	}
	if (ReasonName == PGXEventHandlerReason::ResultAcquireHandlerFailed)
	{
		return TAG_PGX_EventHandler_Result_AcquireHandlerFailed.GetTag();
	}
	if (ReasonName == PGXEventHandlerReason::ResultChainBudgetExceeded)
	{
		return TAG_PGX_EventHandler_Result_ChainBudgetExceeded.GetTag();
	}
	if (ReasonName == PGXEventHandlerReason::ResultChainCycle)
	{
		return TAG_PGX_EventHandler_Result_ChainCycle.GetTag();
	}
	return FGameplayTag();
}

FString PGXReasonString(FName ReasonName)
{
	return ReasonName.IsNone() ? FString() : ReasonName.ToString();
}

FString PGXBlackboxObjectName(const TWeakObjectPtr<UObject>& Object, const UPGXEventHandlerConfig* Config)
{
	if (!Object.IsValid())
	{
		return FString(TEXT("None"));
	}

	if (Config && Config->bRedactBlackboxObjectNames)
	{
		return FString(TEXT("REDACTED"));
	}

	return Config && Config->bUseObjectPathInBlackbox
		? Object->GetPathName()
		: Object->GetName();
}

struct FPGXExecutionStackGuard
{
	FPGXExecutionStackGuard(TArray<FGameplayTag>& InExecutionStack, int32& InCurrentDepth, FGameplayTag InEventTag)
		: ExecutionStack(InExecutionStack)
		, CurrentDepth(InCurrentDepth)
		, EventTag(InEventTag)
	{
		ExecutionStack.Add(EventTag);
		++CurrentDepth;
	}

	~FPGXExecutionStackGuard()
	{
		--CurrentDepth;
		if (ExecutionStack.Num() > 0 && ExecutionStack.Last() == EventTag)
		{
			ExecutionStack.Pop(EAllowShrinking::No);
		}
		else
		{
			ExecutionStack.RemoveSingle(EventTag);
		}
	}

	TArray<FGameplayTag>& ExecutionStack;
	int32& CurrentDepth;
	FGameplayTag EventTag;
};
}

// ============================================================
// Static Accessor
// ============================================================

UPGXEventHandlerSubsystem* UPGXEventHandlerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject)) return nullptr;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World) || !IsValid(World->GetGameInstance())) return nullptr;
	return UGameInstance::GetSubsystem<UPGXEventHandlerSubsystem>(World->GetGameInstance());
}

// ============================================================
// Lifecycle
// ============================================================

void UPGXEventHandlerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// EN: NOTE: This subsystem runs on game thread only. No synchronization required.
	// ES: NOTA: Este subsistema corre solo en game thread. No requiere sincronizacion.
	Super::Initialize(Collection);
	Collection.InitializeDependency<UPGXMessageSubsystem>();

	DiscoverAndLoadConfig();
	LoadHandlerTables();
	RegisterConsoleCommands();

	// ── Profile Integration ──
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			if (ProfileSS->IsProfileResolved())
			{
				ApplyProfileConstraints(ProfileSS->GetResolvedProfile());
			}
			ProfileSS->OnProfileChangedNative.AddUObject(this, &ThisClass::HandleProfileChanged);
		}
	}

	UE_LOG(LogPGXEventHandler, Log, TEXT("PGX EventHandlerSubsystem initialized. Registered handlers: %d, Config: %s"),
		HandlerRegistry.Num(), IsValid(CachedConfig) ? *CachedConfig->GetName() : TEXT("Default"));
}

void UPGXEventHandlerSubsystem::Deinitialize()
{
	// ── Profile Unbind ──
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			ProfileSS->OnProfileChangedNative.RemoveAll(this);
		}
	}

	// EN: Teardown order matters:
	//   1. Unregister console commands (prevents console access to partially-torn-down state)
	//   2. Deactivate cached handlers (OnDeactivated gives handlers cleanup chance)
	//   3. Clear all data structures
	// ES: El orden de teardown importa:
	//   1. Desregistrar comandos de consola (previene acceso a estado parcialmente destruido)
	//   2. Desactivar handlers cacheados (OnDeactivated da oportunidad de limpieza)
	//   3. Limpiar todas las estructuras de datos
	UnregisterConsoleCommands();

	// EN: Deactivate all cached handlers / ES: Desactivar todos los handlers cacheados
	for (auto& Pair : HandlerCache)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->OnDeactivated();
		}
	}

	HandlerCache.Empty();
	HandlerRegistry.Empty();
	CategoryIndex.Empty();
	CacheLRUOrder.Empty();
	TelemetryMap.Empty();
	BlackboxBuffer.Empty();
	RegisteredTables.Empty();
	CachedConfig = nullptr;
	CacheStats = FPGXHandlerCacheStats();
	CurrentExecutionDepth = 0;
	ExecutionStack.Empty();

	UE_LOG(LogPGXEventHandler, Log, TEXT("PGX EventHandlerSubsystem deinitialized."));
	Super::Deinitialize();
}

// ============================================================
// Config Discovery
// ============================================================

void UPGXEventHandlerSubsystem::DiscoverAndLoadConfig()
{
	// EN: Settings-first resolution with AssetRegistry fallback (deprecated)
	// ES: Resolucion Settings-first con fallback a AssetRegistry (deprecated)
	const UPGXEventHandlerSettings* Settings = GetDefault<UPGXEventHandlerSettings>();
	CachedConfig = PGX::ResolveSingleConfig<UPGXEventHandlerConfig>(Settings->ActiveConfig, TEXT("EventHandler"));

	if (IsValid(CachedConfig))
	{
		CacheStats.MaxCachedHandlers = CachedConfig->MaxCachedHandlers;
		UE_LOG(LogPGXEventHandler, Log, TEXT("PGXEventHandlerConfig loaded: %s (MaxCache=%d, MaxDepth=%d, Tables=%d)"),
			*CachedConfig->GetName(), CachedConfig->MaxCachedHandlers, CachedConfig->MaxExecutionDepth, CachedConfig->HandlerTables.Num());
	}
}

void UPGXEventHandlerSubsystem::LoadHandlerTables()
{
	if (!IsValid(CachedConfig)) return;

	for (const TSoftObjectPtr<UDataTable>& TableRef : CachedConfig->HandlerTables)
	{
		UDataTable* Table = TableRef.LoadSynchronous();
		if (IsValid(Table))
		{
			RegisterHandlerTable(Table);
		}
		else
		{
			UE_LOG(LogPGXEventHandler, Warning, TEXT("Failed to load handler DataTable: %s"), *TableRef.ToString());
		}
	}
}

// ============================================================
// Core: ResolveAndExecute (public overloads + internal)
// ============================================================

EPGXEventResult UPGXEventHandlerSubsystem::ResolveAndExecute(FGameplayTag EventTag, UObject* Instigator, const FInstancedStruct& Payload)
{
	FPGXEventContext Context;
	Context.Instigator = Instigator;
	return ResolveAndExecuteInternal(EventTag, Context, Payload);
}

EPGXEventResult UPGXEventHandlerSubsystem::ResolveAndExecuteWithContext(FGameplayTag EventTag, const FPGXEventContext& Context, const FInstancedStruct& Payload)
{
	return ResolveAndExecuteInternal(EventTag, Context, Payload);
}

EPGXEventResult UPGXEventHandlerSubsystem::ResolveAndExecuteInternal(FGameplayTag EventTag, const FPGXEventContext& Context, const FInstancedStruct& Payload)
{
	if (!EventTag.IsValid())
	{
		UE_LOG(LogPGXEventHandler, Warning, TEXT("ResolveAndExecute: Invalid event tag"));
		UpdateTelemetry(EventTag, EPGXEventResult::Failed, 0.0);
		RecordBlackboxEntry(EventTag, EPGXEventResult::Failed, TEXT("None"), 0.0, Context, PGXEventHandlerReason::ValidationInvalidEventTag, &Payload);
		return EPGXEventResult::Failed;
	}

	// EN: Recursion depth check / ES: Verificacion de profundidad de recursion
	const int32 MaxDepth = IsValid(CachedConfig) ? CachedConfig->MaxExecutionDepth : 8;
	if (CurrentExecutionDepth >= MaxDepth)
	{
		UE_LOG(LogPGXEventHandler, Error, TEXT("ResolveAndExecute: Max recursion depth (%d) reached for %s"), MaxDepth, *EventTag.ToString());
		UpdateTelemetry(EventTag, EPGXEventResult::Blocked, 0.0);
		RecordBlackboxEntry(EventTag, EPGXEventResult::Blocked, TEXT("DepthExceeded"), 0.0, Context, PGXEventHandlerReason::ResultChainBudgetExceeded, &Payload);
		return EPGXEventResult::Blocked;
	}

	if (ExecutionStack.Contains(EventTag))
	{
		UE_LOG(LogPGXEventHandler, Error, TEXT("ResolveAndExecute: Cycle detected for %s at depth %d"), *EventTag.ToString(), CurrentExecutionDepth);
		UpdateTelemetry(EventTag, EPGXEventResult::Blocked, 0.0);
		RecordBlackboxEntry(EventTag, EPGXEventResult::Blocked, TEXT("CycleDetected"), 0.0, Context, PGXEventHandlerReason::ResultChainCycle, &Payload);
		return EPGXEventResult::Blocked;
	}

	// EN: Find handler registration / ES: Buscar registro de handler
	const FPGXEventHandlerRow* Row = HandlerRegistry.Find(EventTag);
	if (!Row)
	{
		if (IsValid(CachedConfig) && CachedConfig->bLogExecutions)
		{
			UE_LOG(LogPGXEventHandler, Log, TEXT("No handler registered for: %s"), *EventTag.ToString());
		}
		OnHandlerNotFound.Broadcast(EventTag);
		UpdateTelemetry(EventTag, EPGXEventResult::NotFound, 0.0);
		RecordBlackboxEntry(EventTag, EPGXEventResult::NotFound, TEXT("None"), 0.0, Context, PGXEventHandlerReason::ResultHandlerNotFound, &Payload);
		return EPGXEventResult::NotFound;
	}

	if (!Row->bEnabled)
	{
		UpdateTelemetry(EventTag, EPGXEventResult::Skipped, 0.0);
		RecordBlackboxEntry(EventTag, EPGXEventResult::Skipped, Row->HandlerClass.ToString(), 0.0, Context, PGXEventHandlerReason::ValidationHandlerDisabled, &Payload);
		return EPGXEventResult::Skipped;
	}

	FString PayloadIssue;
	if (!ValidatePayloadForRow(*Row, Payload, PayloadIssue))
	{
		UE_LOG(LogPGXEventHandler, Error, TEXT("ResolveAndExecute: %s"), *PayloadIssue);
		UpdateTelemetry(EventTag, EPGXEventResult::Failed, 0.0);
		RecordBlackboxEntry(EventTag, EPGXEventResult::Failed, Row->HandlerClass.ToString(), 0.0, Context, PGXEventHandlerReason::ValidationPayloadMismatch, &Payload);
		return EPGXEventResult::Failed;
	}

	// EN: Acquire handler instance / ES: Adquirir instancia del handler
	UPGXEventHandlerBase* Handler = AcquireHandler(*Row, Context);
	if (!IsValid(Handler))
	{
		UE_LOG(LogPGXEventHandler, Error, TEXT("Failed to acquire handler for: %s (class: %s)"), *EventTag.ToString(), *Row->HandlerClass.ToString());
		UpdateTelemetry(EventTag, EPGXEventResult::Failed, 0.0);
		RecordBlackboxEntry(EventTag, EPGXEventResult::Failed, Row->HandlerClass.ToString(), 0.0, Context, PGXEventHandlerReason::ResultAcquireHandlerFailed, &Payload);
		return EPGXEventResult::Failed;
	}

	FPGXExecutionStackGuard ExecutionGuard(ExecutionStack, CurrentExecutionDepth, EventTag);

	// EN: Check conditions / ES: Verificar condiciones
	if (!Handler->CanExecute(Context))
	{
		ReleaseHandler(EventTag, Handler, Row->Lifecycle);
		UpdateTelemetry(EventTag, EPGXEventResult::Skipped, 0.0);
		RecordBlackboxEntry(EventTag, EPGXEventResult::Skipped, Handler->GetClass()->GetName(), 0.0, Context, PGXEventHandlerReason::ValidationCanExecuteFalse, &Payload);
		return EPGXEventResult::Skipped;
	}

	// EN: Execute / ES: Ejecutar
	const double StartTime = FPlatformTime::Seconds();

	const EPGXEventResult Result = Handler->Execute(Context, Payload);

	const double ExecutionTimeMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;

	// EN: Post-execution / ES: Post-ejecucion
	ReleaseHandler(EventTag, Handler, Row->Lifecycle);
	UpdateTelemetry(EventTag, Result, ExecutionTimeMs);
	RecordBlackboxEntry(EventTag, Result, Handler->GetClass()->GetName(), ExecutionTimeMs, Context, NAME_None, &Payload);

	if (IsValid(CachedConfig) && CachedConfig->bLogExecutions)
	{
		UE_LOG(LogPGXEventHandler, Log, TEXT("Executed: %s -> %s (%.2fms) Result=%d"),
			*EventTag.ToString(), *Handler->GetClass()->GetName(), ExecutionTimeMs, static_cast<int32>(Result));
	}

	// EN: Fire delegates / ES: Disparar delegates
	OnHandlerExecuted.Broadcast(EventTag, Result, Handler->GetClass()->GetName());
	OnHandlerExecutedNative.Broadcast(EventTag, Result, ExecutionTimeMs);

	return Result;
}

EPGXEventResult UPGXEventHandlerSubsystem::ExecuteSequence(const TArray<FGameplayTag>& EventTags, const FPGXEventContext& Context, const TArray<FInstancedStruct>& Payloads, bool bStopOnFailure)
{
	const bool bHasPayloads = Payloads.Num() > 0;
	if (bHasPayloads && Payloads.Num() != EventTags.Num())
	{
		UE_LOG(LogPGXEventHandler, Error,
			TEXT("ExecuteSequence: Payloads(%d) != EventTags(%d). Must match or be empty."),
			Payloads.Num(), EventTags.Num());
		return EPGXEventResult::Failed;
	}

	const FInstancedStruct EmptyPayload;
	EPGXEventResult LastResult = EPGXEventResult::Success;
	for (int32 i = 0; i < EventTags.Num(); ++i)
	{
		const FInstancedStruct& Payload = bHasPayloads ? Payloads[i] : EmptyPayload;
		LastResult = ResolveAndExecuteInternal(EventTags[i], Context, Payload);
		if (bStopOnFailure && (LastResult == EPGXEventResult::Failed || LastResult == EPGXEventResult::Blocked))
		{
			return LastResult;
		}
	}
	return LastResult;
}

bool UPGXEventHandlerSubsystem::ValidateConditions(const TArray<FGameplayTag>& EventTags, const FPGXEventContext& Context)
{
	for (const FGameplayTag& EvtTag : EventTags)
	{
		const FPGXEventHandlerRow* Row = HandlerRegistry.Find(EvtTag);
		if (!Row || !Row->bEnabled) return false;

		UPGXEventHandlerBase* Handler = AcquireHandler(*Row, Context);
		if (!IsValid(Handler)) return false;

		const bool bCanExec = Handler->CanExecute(Context);
		ReleaseHandler(EvtTag, Handler, Row->Lifecycle);
		if (!bCanExec) return false;
	}
	return true;
}

// ============================================================
// Registration
// ============================================================

void UPGXEventHandlerSubsystem::RegisterHandlerTable(UDataTable* InTable)
{
	if (!IsValid(InTable)) return;

	if (!InTable->GetRowStruct() || !InTable->GetRowStruct()->IsChildOf(FPGXEventHandlerRow::StaticStruct()))
	{
		UE_LOG(LogPGXEventHandler, Error,
			TEXT("RegisterHandlerTable: Table [%s] has incompatible row struct [%s]. Expected FPGXEventHandlerRow."),
			*InTable->GetName(),
			InTable->GetRowStruct() ? *InTable->GetRowStruct()->GetName() : TEXT("null"));
		return;
	}

	RegisteredTables.AddUnique(InTable);

	TArray<FPGXEventHandlerRow*> Rows;
	InTable->GetAllRows<FPGXEventHandlerRow>(TEXT("RegisterHandlerTable"), Rows);

	for (const FPGXEventHandlerRow* Row : Rows)
	{
		if (Row)
		{
			FString Issue;
			if (!ValidateHandlerRow(*Row, InTable->GetName(), Issue))
			{
				UE_LOG(LogPGXEventHandler, Error, TEXT("RegisterHandlerTable: %s"), *Issue);
				continue;
			}

			if (const FPGXEventHandlerRow* Existing = HandlerRegistry.Find(Row->EventTag))
			{
				UE_LOG(LogPGXEventHandler, Warning,
					TEXT("RegisterHandlerTable: Duplicate EventTag [%s] in [%s]; replacing [%s] with [%s] (LastWins)."),
					*Row->EventTag.ToString(), *InTable->GetName(), *Existing->HandlerClass.ToString(), *Row->HandlerClass.ToString());
				RemoveCategoryIndexEntry(Existing->CategoryTag, Row->EventTag);
				EvictHandler(Row->EventTag);
			}

			HandlerRegistry.Add(Row->EventTag, *Row);

			// EN: Update category index / ES: Actualizar indice de categoria
			if (Row->CategoryTag.IsValid())
			{
				CategoryIndex.FindOrAdd(Row->CategoryTag).AddUnique(Row->EventTag);
			}

			if (IsValid(CachedConfig) && CachedConfig->bLogRegistration)
			{
				UE_LOG(LogPGXEventHandler, Log, TEXT("Registered handler: %s -> %s (%s)"),
					*Row->EventTag.ToString(), *Row->HandlerClass.ToString(),
					*UEnum::GetValueAsString(Row->Lifecycle));
			}
		}
	}

	UE_LOG(LogPGXEventHandler, Log, TEXT("Loaded DataTable: %s (%d rows)"), *InTable->GetName(), Rows.Num());
}

void UPGXEventHandlerSubsystem::UnregisterHandlerTable(UDataTable* InTable)
{
	if (!IsValid(InTable)) return;

	TArray<FPGXEventHandlerRow*> Rows;
	InTable->GetAllRows<FPGXEventHandlerRow>(TEXT("UnregisterHandlerTable"), Rows);

	for (const FPGXEventHandlerRow* Row : Rows)
	{
		if (Row && Row->EventTag.IsValid())
		{
			EvictHandler(Row->EventTag);
			HandlerRegistry.Remove(Row->EventTag);

			if (Row->CategoryTag.IsValid())
			{
				if (TArray<FGameplayTag>* CatEntries = CategoryIndex.Find(Row->CategoryTag))
				{
					CatEntries->Remove(Row->EventTag);
					if (CatEntries->Num() == 0) CategoryIndex.Remove(Row->CategoryTag);
				}
			}
		}
	}

	RegisteredTables.Remove(InTable);
}

void UPGXEventHandlerSubsystem::RegisterHandler(FGameplayTag EventTag, TSubclassOf<UPGXEventHandlerBase> HandlerClass,
	EPGXHandlerLifecycle Lifecycle, FGameplayTag CategoryTag)
{
	if (!EventTag.IsValid() || !HandlerClass)
	{
		UE_LOG(LogPGXEventHandler, Warning, TEXT("RegisterHandler: invalid EventTag or HandlerClass"));
		return;
	}

	FPGXEventHandlerRow Row;
	Row.EventTag = EventTag;
	Row.HandlerClass = HandlerClass;
	Row.Lifecycle = Lifecycle;
	Row.CategoryTag = CategoryTag;
	Row.bEnabled = true;

	if (const FPGXEventHandlerRow* Existing = HandlerRegistry.Find(EventTag))
	{
		RemoveCategoryIndexEntry(Existing->CategoryTag, EventTag);
		EvictHandler(EventTag);
	}

	HandlerRegistry.Add(EventTag, Row);
	if (CategoryTag.IsValid())
	{
		CategoryIndex.FindOrAdd(CategoryTag).AddUnique(EventTag);
	}
}

void UPGXEventHandlerSubsystem::UnregisterHandler(FGameplayTag EventTag)
{
	EvictHandler(EventTag);
	const FPGXEventHandlerRow* Row = HandlerRegistry.Find(EventTag);
	if (Row && Row->CategoryTag.IsValid())
	{
		if (TArray<FGameplayTag>* CatEntries = CategoryIndex.Find(Row->CategoryTag))
		{
			CatEntries->Remove(EventTag);
			if (CatEntries->Num() == 0) CategoryIndex.Remove(Row->CategoryTag);
		}
	}
	HandlerRegistry.Remove(EventTag);
}

// ============================================================
// Query API
// ============================================================

bool UPGXEventHandlerSubsystem::IsHandlerRegistered(FGameplayTag EventTag) const
{
	return HandlerRegistry.Contains(EventTag);
}

FPGXEventHandlerInfo UPGXEventHandlerSubsystem::GetHandlerInfo(FGameplayTag EventTag) const
{
	FPGXEventHandlerInfo Info;
	if (const FPGXEventHandlerRow* Row = HandlerRegistry.Find(EventTag))
	{
		Info.EventTag = Row->EventTag;
		Info.HandlerClassName = Row->HandlerClass.ToString();
		Info.Lifecycle = Row->Lifecycle;
		Info.CategoryTag = Row->CategoryTag;
		Info.Description = Row->Description;
		Info.bEnabled = Row->bEnabled;
		Info.bCached = HandlerCache.Contains(EventTag);
	}
	return Info;
}

TArray<FPGXEventHandlerInfo> UPGXEventHandlerSubsystem::GetHandlersByCategory(FGameplayTag CategoryTag) const
{
	TArray<FPGXEventHandlerInfo> Result;
	if (const TArray<FGameplayTag>* Tags = CategoryIndex.Find(CategoryTag))
	{
		for (const FGameplayTag& EvtTag : *Tags)
		{
			Result.Add(GetHandlerInfo(EvtTag));
		}
	}
	return Result;
}

TArray<FGameplayTag> UPGXEventHandlerSubsystem::GetAllRegisteredTags() const
{
	TArray<FGameplayTag> Tags;
	HandlerRegistry.GetKeys(Tags);
	return Tags;
}

bool UPGXEventHandlerSubsystem::HasEntryByTag(FGameplayTag Tag) const
{
	return IsHandlerRegistered(Tag);
}

int32 UPGXEventHandlerSubsystem::GetCount() const
{
	return HandlerRegistry.Num();
}

void UPGXEventHandlerSubsystem::GetSnapshot(TArray<FGameplayTag>& OutTags) const
{
	HandlerRegistry.GetKeys(OutTags);
}

TArray<FGameplayTag> UPGXEventHandlerSubsystem::GetAllCategories() const
{
	TArray<FGameplayTag> Categories;
	CategoryIndex.GetKeys(Categories);
	return Categories;
}

FPGXHandlerCacheStats UPGXEventHandlerSubsystem::GetCacheStats() const
{
	FPGXHandlerCacheStats Stats = CacheStats;
	Stats.CachedHandlers = HandlerCache.Num();
	return Stats;
}

// ============================================================
// Cache Management
// ============================================================

UPGXEventHandlerBase* UPGXEventHandlerSubsystem::AcquireHandler(const FPGXEventHandlerRow& Row, const FPGXEventContext& Context)
{
	const FGameplayTag& EventTag = Row.EventTag;

	// EN: Check cache first / ES: Verificar cache primero
	if (TObjectPtr<UPGXEventHandlerBase>* CachedPtr = HandlerCache.Find(EventTag))
	{
		if (IsValid(*CachedPtr))
		{
			CacheStats.CacheHits++;
			// EN: Move to front of LRU / ES: Mover al frente del LRU
			CacheLRUOrder.Remove(EventTag);
			CacheLRUOrder.Insert(EventTag, 0);

			if (IsValid(CachedConfig) && CachedConfig->bLogCacheOperations)
			{
				UE_LOG(LogPGXEventHandler, Log, TEXT("Cache HIT: %s"), *EventTag.ToString());
			}
			return *CachedPtr;
		}
		// EN: Invalid cached handler, remove it / ES: Handler cacheado invalido, removemos
		HandlerCache.Remove(EventTag);
		CacheLRUOrder.Remove(EventTag);
	}

	CacheStats.CacheMisses++;

	// EN: Create new instance / ES: Crear nueva instancia
	UClass* HandlerClass = Row.HandlerClass.LoadSynchronous();
	if (!HandlerClass)
	{
		UE_LOG(LogPGXEventHandler, Error, TEXT("Failed to load handler class: %s"), *Row.HandlerClass.ToString());
		return nullptr;
	}
	if (!HandlerClass->IsChildOf(UPGXEventHandlerBase::StaticClass()))
	{
		UE_LOG(LogPGXEventHandler, Error, TEXT("Handler class is not a UPGXEventHandlerBase: %s"), *HandlerClass->GetName());
		return nullptr;
	}

	UPGXEventHandlerBase* Handler = NewObject<UPGXEventHandlerBase>(this, HandlerClass);
	if (!IsValid(Handler)) return nullptr;

	// EN: Cache based on lifecycle / ES: Cachear basado en ciclo de vida
	if (Row.Lifecycle == EPGXHandlerLifecycle::Singleton || Row.Lifecycle == EPGXHandlerLifecycle::Cached)
	{
		// EN: Evict if at capacity / ES: Evacuar si al maximo de capacidad
		const int32 MaxCache = IsValid(CachedConfig) ? CachedConfig->MaxCachedHandlers : 128;
		while (HandlerCache.Num() >= MaxCache)
		{
			const int32 BeforeEvictCount = HandlerCache.Num();
			EvictLRU();
			if (HandlerCache.Num() >= BeforeEvictCount)
			{
				UE_LOG(LogPGXEventHandler, Error,
					TEXT("AcquireHandler: cache full (%d) and no evictable cached handlers for %s"),
					MaxCache, *EventTag.ToString());
				return nullptr;
			}
		}

		HandlerCache.Add(EventTag, Handler);
		CacheLRUOrder.Insert(EventTag, 0);
		Handler->OnActivated(Context);

		OnHandlerCacheChanged.Broadcast(EventTag, true);

		if (IsValid(CachedConfig) && CachedConfig->bLogCacheOperations)
		{
			UE_LOG(LogPGXEventHandler, Log, TEXT("Cache MISS + STORE: %s (%s)"), *EventTag.ToString(), *UEnum::GetValueAsString(Row.Lifecycle));
		}
	}

	return Handler;
}

void UPGXEventHandlerSubsystem::ReleaseHandler(FGameplayTag /*EventTag*/, UPGXEventHandlerBase* Handler, EPGXHandlerLifecycle Lifecycle)
{
	if (Lifecycle == EPGXHandlerLifecycle::Ephemeral && IsValid(Handler))
	{
		Handler->OnDeactivated();
		// EN: Ephemeral handlers are not cached — GC will collect them
		// ES: Handlers efimeros no se cachean — GC los recolectara
	}
}

void UPGXEventHandlerSubsystem::EvictLRU()
{
	if (CacheLRUOrder.Num() == 0) return;

	// EN: Evict the least recently used (last in the list) / ES: Evacuar el menos recientemente usado
	// Skip singletons
	for (int32 i = CacheLRUOrder.Num() - 1; i >= 0; --i)
	{
		const FGameplayTag& EvictTag = CacheLRUOrder[i];
		const FPGXEventHandlerRow* Row = HandlerRegistry.Find(EvictTag);
		if (Row && Row->Lifecycle == EPGXHandlerLifecycle::Singleton) continue;

		EvictHandler(EvictTag);
		return;
	}
}

void UPGXEventHandlerSubsystem::InvalidateCache()
{
	for (auto& Pair : HandlerCache)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->OnDeactivated();
		}
	}
	HandlerCache.Empty();
	CacheLRUOrder.Empty();
	CacheStats.CachedHandlers = 0;

	UE_LOG(LogPGXEventHandler, Log, TEXT("Handler cache invalidated."));
}

void UPGXEventHandlerSubsystem::EvictHandler(FGameplayTag EventTag)
{
	if (TObjectPtr<UPGXEventHandlerBase>* CachedPtr = HandlerCache.Find(EventTag))
	{
		if (IsValid(*CachedPtr))
		{
			(*CachedPtr)->OnDeactivated();
		}
		HandlerCache.Remove(EventTag);
		CacheLRUOrder.Remove(EventTag);
		CacheStats.Evictions++;
		OnHandlerCacheChanged.Broadcast(EventTag, false);
	}
}

bool UPGXEventHandlerSubsystem::ValidateHandlerRow(const FPGXEventHandlerRow& Row, const FString& SourceName, FString& OutIssue) const
{
	if (!Row.EventTag.IsValid())
	{
		OutIssue = FString::Printf(TEXT("[%s] invalid EventTag"), *SourceName);
		return false;
	}

	if (Row.HandlerClass.IsNull())
	{
		OutIssue = FString::Printf(TEXT("[%s] %s has null HandlerClass"), *SourceName, *Row.EventTag.ToString());
		return false;
	}

	UClass* LoadedClass = Row.HandlerClass.LoadSynchronous();
	if (!LoadedClass || !LoadedClass->IsChildOf(UPGXEventHandlerBase::StaticClass()))
	{
		OutIssue = FString::Printf(TEXT("[%s] %s HandlerClass [%s] is not a UPGXEventHandlerBase"),
			*SourceName, *Row.EventTag.ToString(), *Row.HandlerClass.ToString());
		return false;
	}

	return true;
}

bool UPGXEventHandlerSubsystem::ValidatePayloadForRow(const FPGXEventHandlerRow& Row, const FInstancedStruct& Payload, FString& OutIssue) const
{
	if (Row.ExpectedPayloadType.IsEmpty())
	{
		return true;
	}

	const UScriptStruct* PayloadStruct = Payload.GetScriptStruct();
	if (PGXPayloadTypeMatchesExpected(PayloadStruct, Row.ExpectedPayloadType))
	{
		return true;
	}

	OutIssue = FString::Printf(TEXT("PayloadMismatch for %s: expected [%s], got [%s]"),
		*Row.EventTag.ToString(), *Row.ExpectedPayloadType, *PGXPayloadTypeName(Payload));
	return false;
}

void UPGXEventHandlerSubsystem::RemoveCategoryIndexEntry(FGameplayTag CategoryTag, FGameplayTag EventTag)
{
	if (!CategoryTag.IsValid())
	{
		return;
	}

	if (TArray<FGameplayTag>* CatEntries = CategoryIndex.Find(CategoryTag))
	{
		CatEntries->Remove(EventTag);
		if (CatEntries->Num() == 0)
		{
			CategoryIndex.Remove(CategoryTag);
		}
	}
}

// ============================================================
// Telemetry
// ============================================================

void UPGXEventHandlerSubsystem::UpdateTelemetry(FGameplayTag EventTag, EPGXEventResult Result, double ExecutionTimeMs)
{
	FPGXHandlerTelemetry& Telem = TelemetryMap.FindOrAdd(EventTag);
	Telem.EventTag = EventTag;
	Telem.ExecutionCount++;
	Telem.TotalExecutionTimeMs += ExecutionTimeMs;
	Telem.AvgExecutionTimeMs = Telem.TotalExecutionTimeMs / Telem.ExecutionCount;
	Telem.MaxExecutionTimeMs = FMath::Max(Telem.MaxExecutionTimeMs, ExecutionTimeMs);
	Telem.LastExecutionTimestamp = FPlatformTime::Seconds();

	switch (Result)
	{
	case EPGXEventResult::Success:
	case EPGXEventResult::Partial:
		Telem.SuccessCount++; break;
	case EPGXEventResult::Failed:
	case EPGXEventResult::Blocked:
		Telem.FailCount++; break;
	case EPGXEventResult::Skipped:
	case EPGXEventResult::NotFound:
		Telem.SkipCount++; break;
	}
}

FPGXHandlerTelemetry UPGXEventHandlerSubsystem::GetHandlerTelemetry(FGameplayTag EventTag) const
{
	if (const FPGXHandlerTelemetry* Telem = TelemetryMap.Find(EventTag))
	{
		return *Telem;
	}
	return FPGXHandlerTelemetry();
}

TArray<FPGXHandlerTelemetry> UPGXEventHandlerSubsystem::GetAllTelemetry() const
{
	TArray<FPGXHandlerTelemetry> Result;
	TelemetryMap.GenerateValueArray(Result);
	return Result;
}

void UPGXEventHandlerSubsystem::ResetTelemetry()
{
	TelemetryMap.Empty();
}

// ============================================================
// Blackbox
// ============================================================

void UPGXEventHandlerSubsystem::RecordBlackboxEntry(FGameplayTag EventTag, EPGXEventResult Result,
	const FString& HandlerClassName, double ExecutionTimeMs, const FPGXEventContext& Context, FName FailureReasonName, const FInstancedStruct* Payload)
{
	const int32 MaxSize = IsValid(CachedConfig) ? CachedConfig->BlackboxBufferSize : 256;

	FPGXBlackboxEntry Entry;
	Entry.Timestamp = FPlatformTime::Seconds();
	Entry.EventTag = EventTag;
	Entry.Result = Result;
	Entry.HandlerClassName = HandlerClassName;
	Entry.ExecutionTimeMs = ExecutionTimeMs;
	Entry.InstigatorName = PGXBlackboxObjectName(Context.Instigator, CachedConfig);
	Entry.TargetName = PGXBlackboxObjectName(Context.Target, CachedConfig);
	Entry.PayloadTypeName = Payload ? PGXPayloadTypeName(*Payload) : TEXT("None");
	Entry.FailureReasonTag = PGXReasonTag(FailureReasonName);
	Entry.FailureReason = PGXReasonString(FailureReasonName);
	Entry.ChainDepth = CurrentExecutionDepth;

	BlackboxBuffer.Add(MoveTemp(Entry));
	while (BlackboxBuffer.Num() > MaxSize)
	{
		BlackboxBuffer.RemoveAt(0);
	}
}

FString UPGXEventHandlerSubsystem::DumpBlackboxToString() const
{
	FString Result;
	Result += FString::Printf(TEXT("=== PGX EventHandler Blackbox (%d entries) ===\n"), BlackboxBuffer.Num());
	for (const FPGXBlackboxEntry& Entry : BlackboxBuffer)
	{
		Result += FString::Printf(TEXT("[%.2f] depth=%d %s -> %s (%s) %.2fms Instigator=%s Target=%s Payload=%s Reason=%s\n"),
			Entry.Timestamp, Entry.ChainDepth, *Entry.EventTag.ToString(), *Entry.HandlerClassName,
			*UEnum::GetValueAsString(Entry.Result), Entry.ExecutionTimeMs,
			*Entry.InstigatorName, *Entry.TargetName, *Entry.PayloadTypeName, *Entry.FailureReason);
	}
	return Result;
}

const TArray<FPGXBlackboxEntry>& UPGXEventHandlerSubsystem::GetBlackboxEntries() const
{
	return BlackboxBuffer;
}

FString UPGXEventHandlerSubsystem::ExportReport() const
{
	FString Report;
	Report += TEXT("# PGX Event Handler Report\n\n");

	// EN: Summary / ES: Resumen
	Report += TEXT("## Summary\n");
	Report += FString::Printf(TEXT("- Registered Handlers: %d\n"), HandlerRegistry.Num());
	Report += FString::Printf(TEXT("- Cached Handlers: %d / %d\n"), HandlerCache.Num(), CacheStats.MaxCachedHandlers);
	Report += FString::Printf(TEXT("- Cache Hits: %d | Misses: %d | Evictions: %d\n"), CacheStats.CacheHits, CacheStats.CacheMisses, CacheStats.Evictions);
	Report += FString::Printf(TEXT("- Categories: %d\n\n"), CategoryIndex.Num());

	// EN: Telemetry / ES: Telemetria
	Report += TEXT("## Telemetry\n");
	Report += TEXT("| Event Tag | Executions | Success | Fail | Skip | Avg(ms) | Max(ms) |\n");
	Report += TEXT("|-----------|-----------|---------|------|------|---------|---------|\n");
	for (const auto& Pair : TelemetryMap)
	{
		const FPGXHandlerTelemetry& T = Pair.Value;
		Report += FString::Printf(TEXT("| %s | %d | %d | %d | %d | %.2f | %.2f |\n"),
			*T.EventTag.ToString(), T.ExecutionCount, T.SuccessCount, T.FailCount, T.SkipCount, T.AvgExecutionTimeMs, T.MaxExecutionTimeMs);
	}

	// EN: Recent blackbox / ES: Blackbox reciente
	const int32 MaxEntries = IsValid(CachedConfig) ? CachedConfig->BlackboxEntriesInReport : 50;
	Report += TEXT("\n## Recent Blackbox\n");
	const int32 Start = FMath::Max(0, BlackboxBuffer.Num() - MaxEntries);
	for (int32 i = Start; i < BlackboxBuffer.Num(); ++i)
	{
		const FPGXBlackboxEntry& E = BlackboxBuffer[i];
		Report += FString::Printf(TEXT("- [%.2f] %s -> %s (%s) %.2fms\n"),
			E.Timestamp, *E.EventTag.ToString(), *E.HandlerClassName,
			*UEnum::GetValueAsString(E.Result), E.ExecutionTimeMs);
	}

	return Report;
}

// ============================================================
// Console Commands
// ============================================================

void UPGXEventHandlerSubsystem::RegisterConsoleCommands()
{
	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.event.status"),
		TEXT("Display PGX Event Handler status"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGXEventHandler, Log, TEXT("=== PGX EventHandler Status ==="));
			UE_LOG(LogPGXEventHandler, Log, TEXT("  Registered: %d handlers"), HandlerRegistry.Num());
			UE_LOG(LogPGXEventHandler, Log, TEXT("  Cached: %d / %d"), HandlerCache.Num(), CacheStats.MaxCachedHandlers);
			UE_LOG(LogPGXEventHandler, Log, TEXT("  Cache Hits: %d | Misses: %d | Evictions: %d"), CacheStats.CacheHits, CacheStats.CacheMisses, CacheStats.Evictions);
			UE_LOG(LogPGXEventHandler, Log, TEXT("  Categories: %d"), CategoryIndex.Num());
			UE_LOG(LogPGXEventHandler, Log, TEXT("  Blackbox entries: %d"), BlackboxBuffer.Num());
		}),
		ECVF_Default));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.event.list"),
		TEXT("List all registered event handlers"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGXEventHandler, Log, TEXT("=== Registered Event Handlers (%d) ==="), HandlerRegistry.Num());
			for (const auto& Pair : HandlerRegistry)
			{
				const FPGXEventHandlerRow& Row = Pair.Value;
				UE_LOG(LogPGXEventHandler, Log, TEXT("  %s -> %s [%s] %s"),
					*Row.EventTag.ToString(), *Row.HandlerClass.ToString(),
					*UEnum::GetValueAsString(Row.Lifecycle),
					Row.bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
			}
		}),
		ECVF_Default));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.event.cache"),
		TEXT("Show handler cache state"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			const FPGXHandlerCacheStats S = GetCacheStats();
			UE_LOG(LogPGXEventHandler, Log, TEXT("=== Handler Cache ==="));
			UE_LOG(LogPGXEventHandler, Log, TEXT("  Cached: %d / %d"), S.CachedHandlers, S.MaxCachedHandlers);
			UE_LOG(LogPGXEventHandler, Log, TEXT("  Hits: %d | Misses: %d | Evictions: %d"), S.CacheHits, S.CacheMisses, S.Evictions);
			for (const FGameplayTag& CtxTag : CacheLRUOrder)
			{
				UE_LOG(LogPGXEventHandler, Log, TEXT("  [LRU] %s"), *CtxTag.ToString());
			}
		}),
		ECVF_Default));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.event.cache.clear"),
		TEXT("Clear the handler cache"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			InvalidateCache();
			UE_LOG(LogPGXEventHandler, Log, TEXT("Handler cache cleared."));
		}),
		ECVF_Default));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.event.telemetry"),
		TEXT("Show handler execution telemetry"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGXEventHandler, Log, TEXT("=== Handler Telemetry ==="));
			for (const auto& Pair : TelemetryMap)
			{
				const FPGXHandlerTelemetry& T = Pair.Value;
				UE_LOG(LogPGXEventHandler, Log, TEXT("  %s: %d exec, %d ok, %d fail, avg %.2fms, max %.2fms"),
					*T.EventTag.ToString(), T.ExecutionCount, T.SuccessCount, T.FailCount, T.AvgExecutionTimeMs, T.MaxExecutionTimeMs);
			}
		}),
		ECVF_Default));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.event.blackbox"),
		TEXT("Dump blackbox execution history"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGXEventHandler, Log, TEXT("%s"), *DumpBlackboxToString());
		}),
		ECVF_Default));

	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.event.report"),
		TEXT("Export a full event handler report"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGXEventHandler, Log, TEXT("%s"), *ExportReport());
		}),
		ECVF_Default));
}

void UPGXEventHandlerSubsystem::UnregisterConsoleCommands()
{
	for (IConsoleCommand* Cmd : ConsoleCommands)
	{
		if (Cmd)
		{
			IConsoleManager::Get().UnregisterConsoleObject(Cmd);
		}
	}
	ConsoleCommands.Empty();
}

// ============================================================
// Profile Integration
// ============================================================

void UPGXEventHandlerSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& /*Profile*/)
{
	int32 EnforcedMaxCachedHandlers = 0;
	int32 EnforcedBlackboxBufferSize = 0;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
			{
				const auto& B = PlatformCfg->EventHandlerBudgets;
				EnforcedMaxCachedHandlers = B.MaxCachedHandlers;
				EnforcedBlackboxBufferSize = B.BlackboxBufferSize;
			}
		}
	}

	// EN: Apply cache limit — use profile budget if set, otherwise keep config/default
	// ES: Aplicar limite de cache — usar budget de profile si existe, sino mantener config/default
	if (EnforcedMaxCachedHandlers > 0)
	{
		CacheStats.MaxCachedHandlers = EnforcedMaxCachedHandlers;

		// EN: Trim cache if it exceeds the new limit / ES: Recortar cache si excede el nuevo limite
		while (HandlerCache.Num() > EnforcedMaxCachedHandlers)
		{
			EvictLRU();
		}
	}

	// EN: Apply blackbox buffer limit — trim oldest entries if needed
	// ES: Aplicar limite de blackbox — recortar entradas mas antiguas si es necesario
	if (EnforcedBlackboxBufferSize > 0)
	{
		while (BlackboxBuffer.Num() > EnforcedBlackboxBufferSize)
		{
			BlackboxBuffer.RemoveAt(0);
		}
	}

	UE_LOG(LogPGXEventHandler, Log, TEXT("[EventHandlerSubsystem] Profile constraints applied — MaxCachedHandlers=%d, BlackboxBuffer=%d"),
		EnforcedMaxCachedHandlers, EnforcedBlackboxBufferSize);
}

void UPGXEventHandlerSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}

// ============================================================
// Test Harness Support
// ============================================================

#if WITH_EDITOR
void UPGXEventHandlerSubsystem::InjectTestConfig(UPGXEventHandlerConfig* TestConfig)
{
	CachedConfig = TestConfig;
	UE_LOG(LogPGXEventHandler, Log, TEXT("Test config injected (MaxCached=%d, MaxDepth=%d)"),
		IsValid(CachedConfig) ? CachedConfig->MaxCachedHandlers : 0,
		IsValid(CachedConfig) ? CachedConfig->MaxExecutionDepth : 0);
}

void UPGXEventHandlerSubsystem::ClearTestConfigs()
{
	CachedConfig = nullptr;
	DiscoverAndLoadConfig();
	UE_LOG(LogPGXEventHandler, Log, TEXT("Test config cleared, re-discovered from AssetRegistry"));
}
#endif
