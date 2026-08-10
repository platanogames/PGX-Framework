// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLevelFlowSubsystem.h"
#include "Misc/Char.h"

namespace
{
	bool NormalizeOpenLevelOptions(const FString& RawOptions, FString& OutOptions)
	{
		OutOptions = RawOptions;
		OutOptions.TrimStartAndEndInline();
		while (OutOptions.StartsWith(TEXT("?")))
		{
			OutOptions.RightChopInline(1, EAllowShrinking::No);
		}
		for (const TCHAR Character : OutOptions)
		{
			if (FChar::IsControl(Character) || Character == TEXT('#') || Character == TEXT('|'))
			{
				OutOptions.Reset();
				return false;
			}
		}
		return true;
	}
}
#include "Logging/PGXLogMacros.h"
#include "PGXLevelFlowConfig.h"
#include "PGXLevelProfile.h"
#include "PGXLevelFlowSettings.h"
#include "Utils/PGXConfigResolution.h"
#include "PGXLoadingRuntime.h"
#include "Tags/PGXLevelFlowTags.h"

// Core traceability
#include "Trace/PGXTraceHelper.h"
#include "Trace/PGXTraceTags.h"

// AssetRegistry for auto-discovery
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

// Level loading
#include "Engine/AssetManager.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "ShaderPipelineCache.h"

// LevelFlow actor
#include "PGXLevelFlowActor.h"

// Profile integration
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"

// Message bridge integration
#include "Messages/PGXMessage.h"
#include "Messages/PGXMessageSubsystem.h"

// EN: LevelFlow subsystem — complete implementation
//     (discovery, state machine, transition pipeline, timing, GameFlow, console)
// ES: Subsistema LevelFlow — implementacion completa
//     (descubrimiento, maquina de estados, pipeline de transicion, timing, GameFlow, consola)

// ============================================================================
// Static
// ============================================================================

TWeakObjectPtr<UPGXLevelFlowSubsystem> UPGXLevelFlowSubsystem::CachedInstance = nullptr;

// ============================================================================
// Lifecycle
// ============================================================================

void UPGXLevelFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UPGXMessageSubsystem>();

	Super::Initialize(Collection);

	CachedInstance = this;

	DiscoverConfigs();
	MergeLevelCatalogs();
	// EN: Bind to PostLoadMapWithWorld to detect when OpenLevel completes
	// ES: Enlazar a PostLoadMapWithWorld para detectar cuando OpenLevel completa
	PostLoadMapDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UPGXLevelFlowSubsystem::OnPostLoadMap);

	bIsInitialized = true;

	// EN: Register trace config for Loading system
	// ES: Registrar config de traza para sistema Loading
	FPGXTraceConfig TraceConfig;
	if (ActiveConfig)
	{
		TraceConfig = ActiveConfig->TraceConfig;
	}
	FPGXTraceHelper::RegisterSystemTraceConfig(TAG_PGX_System_Loading, TraceConfig);

	// EN: Apply project profile constraints if available
	// ES: Aplicar restricciones del profile de proyecto si esta disponible
	if (auto* ProfileSS = GetGameInstance()->GetSubsystem<UPGXProfileSubsystem>())
	{
		if (ProfileSS->IsProfileResolved())
		{
			ApplyProfileConstraints(ProfileSS->GetResolvedProfile());
		}
		ProfileSS->OnProfileChangedNative.AddUObject(this, &ThisClass::HandleProfileChanged);
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlowSubsystem] Initialized: %d profiles, %d levels registered"),
		DiscoveredProfiles.Num(),
		MergedLevelCatalog.Num());
}

void UPGXLevelFlowSubsystem::Deinitialize()
{
	// EN: Cleanup Profile delegate subscription / ES: Limpiar suscripcion a delegate de Profile
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* Profile = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			Profile->OnProfileChangedNative.RemoveAll(this);
		}
	}

	// EN: Cancel any active transition
	// ES: Cancelar cualquier transicion activa
	if (CurrentState != EPGXLevelFlowState::Idle)
	{
		// Cleanup without broadcasting (shutting down)
		if (ActiveStreamableHandle.IsValid())
		{
			ActiveStreamableHandle->CancelHandle();
			ActiveStreamableHandle.Reset();
		}
		if (TimingTickerHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(TimingTickerHandle);
			TimingTickerHandle.Reset();
		}
		CurrentState = EPGXLevelFlowState::Idle;
	}
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapDelegateHandle);
	FPGXTraceHelper::UnregisterSystemTraceConfig(TAG_PGX_System_Loading);

	CachedInstance = nullptr;
	bIsInitialized = false;

	MergedLevelCatalog.Empty();
	DiscoveredProfiles.Empty();
	ActiveConfig = nullptr;
	TransitionHistory.Empty();
	LoadedSubLevels.Empty();
	CurrentLevelFlowActorRef = nullptr;

	Super::Deinitialize();
}

// ============================================================================
// Discovery & Catalog
// ============================================================================

void UPGXLevelFlowSubsystem::DiscoverConfigs()
{
	const UPGXLevelFlowSettings* Settings = GetDefault<UPGXLevelFlowSettings>();

	// --- Phase 1: LevelFlowConfig (single config — Settings first) ---
	// EN: Settings-first resolution with AssetRegistry fallback (deprecated)
	// ES: Resolucion Settings-first con fallback a AssetRegistry (deprecated)
	ActiveConfig = PGX::ResolveSingleConfig<UPGXLevelFlowConfig>(Settings->ActiveConfig, TEXT("LevelFlow"));

	// --- Phase 2: Level Profiles (all discovered — Settings DataTable or AssetRegistry) ---
	if (!Settings->LevelCatalogTable.IsNull())
	{
		// EN: Load profiles from DataTable (deterministic)
		// ES: Cargar profiles desde DataTable (deterministico)
		UDataTable* Table = Settings->LevelCatalogTable.LoadSynchronous();
		if (IsValid(Table))
		{
			TArray<FPGXLevelCatalogRow*> Rows;
			Table->GetAllRows<FPGXLevelCatalogRow>(TEXT("LevelFlowDiscovery"), Rows);

			for (const FPGXLevelCatalogRow* Row : Rows)
			{
				if (!Row || Row->ProfileRef.IsNull()) { continue; }
				UPGXLevelProfile* Profile = Row->ProfileRef.LoadSynchronous();
				if (IsValid(Profile))
				{
					DiscoveredProfiles.Add(Profile);
					PGX_LOG_INFO(LogPGXLoading, TEXT("[Discovery] Profile from DataTable: '%s' — %d levels"),
						*Profile->GetName(), Profile->LevelCatalog.Num());
				}
			}
			PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] %d profiles resolved from DataTable."), DiscoveredProfiles.Num());
		}
	}
	else
	{
		// EN: AssetRegistry fallback (deprecated)
		// ES: Fallback AssetRegistry (deprecated)
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		FARFilter Filter;
		Filter.ClassPaths.Add(UPGXLevelProfile::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;

		TArray<FAssetData> AssetDataList;
		AssetRegistry.GetAssets(Filter, AssetDataList);

		for (const FAssetData& AssetData : AssetDataList)
		{
			UPGXLevelProfile* Profile = Cast<UPGXLevelProfile>(AssetData.GetAsset());
			if (!Profile)
			{
				PGX_LOG_WARNING(LogPGXLoading, TEXT("[Discovery] Failed to load LevelProfile: %s"), *AssetData.GetObjectPathString());
				continue;
			}

			DiscoveredProfiles.Add(Profile);
			PGX_LOG_INFO(LogPGXLoading, TEXT("[Discovery] Profile: '%s' — %d levels"),
				*Profile->GetName(), Profile->LevelCatalog.Num());
		}

		if (DiscoveredProfiles.Num() > 0)
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow] %d profiles auto-discovered from AssetRegistry. "
				"Configure a DataTable in Project Settings > PGX > Level Flow to remove this warning."),
				DiscoveredProfiles.Num());
		}
	}
}

void UPGXLevelFlowSubsystem::MergeLevelCatalogs()
{
	MergedLevelCatalog.Empty();

	for (const UPGXLevelProfile* Profile : DiscoveredProfiles)
	{
		if (!Profile) continue;

		for (const auto& Pair : Profile->LevelCatalog)
		{
			const FGameplayTag& LevelTag = Pair.Key;
			const FPGXLevelEntry& Entry = Pair.Value;

			if (!LevelTag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXLoading, TEXT("[MergeCatalogs] Invalid tag in profile '%s' — skipped"),
					*Profile->GetName());
				continue;
			}

			if (MergedLevelCatalog.Contains(LevelTag))
			{
				PGX_LOG_WARNING(LogPGXLoading, TEXT("[MergeCatalogs] Duplicate tag '%s' in profile '%s' — skipped (already registered)"),
					*LevelTag.ToString(), *Profile->GetName());
				continue;
			}

			MergedLevelCatalog.Add(LevelTag, Entry);
		}
	}
}

// ============================================================================
// State Machine
// ============================================================================

void UPGXLevelFlowSubsystem::SetTransitionState(EPGXLevelFlowState NewState)
{
	if (CurrentState == NewState) return;

	const EPGXLevelFlowState OldState = CurrentState;
	CurrentState = NewState;

	const FGameplayTag& RelevantTag = ActiveTransitionTargetTag.IsValid()
		? ActiveTransitionTargetTag
		: CurrentLevelTag;

	OnLevelFlowStateChangedNative.Broadcast(NewState, RelevantTag);

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] State: %d → %d (Level: %s)"),
		static_cast<int32>(OldState),
		static_cast<int32>(NewState),
		RelevantTag.IsValid() ? *RelevantTag.ToString() : TEXT("(none)"));
}

// ============================================================================
// Transition Pipeline — Request
// ============================================================================

FPGXLevelFlowResult UPGXLevelFlowSubsystem::RequestLevelTransition(FGameplayTag LevelTag, UObject* /*Source*/)
{
	// Guard: initialized
	if (!bIsInitialized)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed,
			TEXT("LevelFlow subsystem not initialized"));
	}

	// Guard: not already transitioning
	if (CurrentState != EPGXLevelFlowState::Idle)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::TransitionInProgress,
			FString::Printf(TEXT("Transition already in progress (state: %d)"), static_cast<int32>(CurrentState)));
	}

	// Guard: valid tag
	if (!LevelTag.IsValid())
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::TagNotFound, TEXT("Invalid level tag"));
	}

	// Resolve tag
	const FPGXLevelEntry* FoundEntry = MergedLevelCatalog.Find(LevelTag);
	if (!FoundEntry)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::TagNotFound,
			FString::Printf(TEXT("Level tag '%s' not found in catalog"), *LevelTag.ToString()));
	}

	// Guard: not same level
	if (CurrentLevelTag == LevelTag)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::AlreadyInLevel,
			FString::Printf(TEXT("Already in level '%s'"), *LevelTag.ToString()));
	}

	// ── PREPARING ──
	ActiveTransitionEntry = *FoundEntry;
	ActiveTransitionTargetTag = LevelTag;
	ActiveTransitionFromTag = CurrentLevelTag;
	TransitionStartTime = FPlatformTime::Seconds();
	TransitionProgressValue = 0.0f;

	SetTransitionState(EPGXLevelFlowState::Preparing);

	// Notify current actor that we're leaving
	if (APGXLevelFlowActor* Actor = CurrentLevelFlowActorRef.Get())
	{
		Actor->OnLevelExiting();
	}

	// Broadcast transition start
	OnTransitionStarted.Broadcast(LevelTag);
	OnTransitionStartedNative.Broadcast(LevelTag);

	// GameFlow integration: set Loading state
	SetGameFlowLoading();

	// ── Start loading based on strategy ──
	switch (ActiveTransitionEntry.LoadStrategy)
	{
	case EPGXLoadStrategy::AsyncLoad:
		SetTransitionState(EPGXLevelFlowState::Loading);
		StartAsyncLoad();
		break;

	case EPGXLoadStrategy::SyncLoad:
		SetTransitionState(EPGXLevelFlowState::Loading);
		// Synchronous: skip straight to OpenLevel
		SetTransitionState(EPGXLevelFlowState::Transitioning);
		ExecuteOpenLevel();
		break;

	case EPGXLoadStrategy::StreamSubLevel:
		// EN: Sub-level streaming doesn't change persistent level
		// ES: Streaming de sub-nivel no cambia nivel persistente
		PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow] StreamSubLevel strategy used via RequestLevelTransition — use RequestSubLevelLoad instead"));
		FailTransition(EPGXLevelFlowResultCode::LoadFailed, TEXT("StreamSubLevel should use RequestSubLevelLoad"));
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed,
			TEXT("Use RequestSubLevelLoad for sub-level streaming"));
	}

	return FPGXLevelFlowResult::MakeSuccess(
		FString::Printf(TEXT("Transition to '%s' started"), *LevelTag.ToString()));
}

// ============================================================================
// Transition Pipeline — Async Load
// ============================================================================

void UPGXLevelFlowSubsystem::StartAsyncLoad()
{
	const FSoftObjectPath AssetPath = ActiveTransitionEntry.LevelReference.ToSoftObjectPath();

	if (!AssetPath.IsValid())
	{
		FailTransition(EPGXLevelFlowResultCode::LoadFailed,
			FString::Printf(TEXT("Invalid LevelReference for '%s'"), *ActiveTransitionTargetTag.ToString()));
		return;
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] Starting async load: %s"), *AssetPath.ToString());

	FStreamableManager& Manager = UAssetManager::GetStreamableManager();
	ActiveStreamableHandle = Manager.RequestAsyncLoad(
		AssetPath,
		FStreamableDelegate::CreateUObject(this, &UPGXLevelFlowSubsystem::OnAsyncLoadComplete)
	);

	if (!ActiveStreamableHandle.IsValid())
	{
		FailTransition(EPGXLevelFlowResultCode::LoadFailed,
			TEXT("Failed to create async load handle"));
	}
}

void UPGXLevelFlowSubsystem::OnAsyncLoadComplete()
{
	// Guard: still in Loading state
	if (CurrentState != EPGXLevelFlowState::Loading)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow] Async load completed but state is %d (expected Loading) — ignoring"),
			static_cast<int32>(CurrentState));
		return;
	}

	// Check if load was successful
	if (!ActiveStreamableHandle.IsValid() || !ActiveStreamableHandle->HasLoadCompleted())
	{
		FailTransition(EPGXLevelFlowResultCode::LoadFailed,
			TEXT("Async load failed or was cancelled"));
		return;
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] Async load complete for '%s'"),
		*ActiveTransitionTargetTag.ToString());

	TransitionProgressValue = 0.5f;
	BroadcastProgressUpdate();

	// Proceed to OpenLevel
	SetTransitionState(EPGXLevelFlowState::Transitioning);
	ExecuteOpenLevel();
}

// ============================================================================
// Transition Pipeline — OpenLevel
// ============================================================================

void UPGXLevelFlowSubsystem::ExecuteOpenLevel()
{
	const FString MapPath = ResolveLevelPath(ActiveTransitionEntry);

	if (MapPath.IsEmpty())
	{
		FailTransition(EPGXLevelFlowResultCode::LoadFailed,
			FString::Printf(TEXT("Cannot resolve level path for '%s'"), *ActiveTransitionTargetTag.ToString()));
		return;
	}

	FString OpenLevelOptions;
	if (const FString* RawOptions = ActiveTransitionEntry.CustomParams.Find(FName(TEXT("OpenLevelOptions"))))
	{
		if (!NormalizeOpenLevelOptions(*RawOptions, OpenLevelOptions))
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow] OpenLevelOptions contains an unsafe separator or control character — transition rejected"));
			FailTransition(EPGXLevelFlowResultCode::LoadFailed, TEXT("Invalid OpenLevelOptions"));
			return;
		}
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] OpenLevel: %s (options=%s)"), *MapPath, *OpenLevelOptions);

	// EN: Clear loaded sub-levels — new level starts fresh
	// ES: Limpiar sub-niveles cargados — nuevo nivel empieza limpio
	LoadedSubLevels.Empty();

	// EN: Release streamable handle before world transition
	// ES: Liberar handle de streaming antes de la transicion de mundo
	ActiveStreamableHandle.Reset();

	// EN: OpenLevel triggers world teardown → new world load → PostLoadMapWithWorld fires
	// ES: OpenLevel dispara teardown del mundo → carga nuevo mundo → PostLoadMapWithWorld se dispara
	UGameplayStatics::OpenLevel(this, FName(*MapPath), true, OpenLevelOptions);
}

#if WITH_DEV_AUTOMATION_TESTS
bool UPGXLevelFlowSubsystem::NormalizeOpenLevelOptionsForTesting(const FString& RawOptions, FString& OutOptions)
{
	return NormalizeOpenLevelOptions(RawOptions, OutOptions);
}
#endif

// ============================================================================
// Transition Pipeline — Post-Load Map Detection
// ============================================================================

void UPGXLevelFlowSubsystem::OnPostLoadMap(UWorld* NewWorld)
{
	// EN: Only process if we're expecting a new level (Transitioning state)
	// ES: Solo procesar si esperamos un nuevo nivel (estado Transitioning)
	if (CurrentState != EPGXLevelFlowState::Transitioning)
	{
		return;
	}

	if (!NewWorld)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow] PostLoadMap fired with null world"));
		return;
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] New world loaded — entering PostLoad"));

	SetTransitionState(EPGXLevelFlowState::PostLoad);
	StartPostLoadTiming();
}

// ============================================================================
// Transition Pipeline — PostLoad Timing Algorithm
// ============================================================================

void UPGXLevelFlowSubsystem::StartPostLoadTiming()
{
	PostLoadElapsed = 0.0f;
	ShadersOnEntry = static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
	TransitionProgressValue = 0.6f;

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] PostLoad timing started — Shaders pending: %d, MinTime: %.2f, MaxTime: %.2f"),
		ShadersOnEntry,
		GetActiveTiming().MinTime,
		GetActiveTiming().MaxTime);

	TimingTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UPGXLevelFlowSubsystem::OnTimingTick),
		0.0f
	);
}

bool UPGXLevelFlowSubsystem::OnTimingTick(float DeltaTime)
{
	// Guard: still in PostLoad
	if (CurrentState != EPGXLevelFlowState::PostLoad)
	{
		return false; // Stop ticking
	}

	PostLoadElapsed += DeltaTime;
	const FPGXTransitionTiming& Timing = GetActiveTiming();

	// ── Phase 1: Wait MinTime (visual smoothness) ──
	if (PostLoadElapsed < Timing.MinTime)
	{
		TransitionProgressValue = 0.6f + (PostLoadElapsed / Timing.MinTime) * 0.2f;
		BroadcastProgressUpdate();
		return true; // Continue ticking
	}

	// ── Phase 2: Check readiness conditions ──
	bool bAllConditionsMet = true;

	if (Timing.bWaitForShaderCompilation)
	{
		const int32 Remaining = static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
		if (Remaining > 0)
		{
			bAllConditionsMet = false;
		}
	}

	// EN: MaxTime is the absolute safety net — always proceed
	// ES: MaxTime es la red de seguridad absoluta — siempre proceder
	if (bAllConditionsMet || PostLoadElapsed >= Timing.MaxTime)
	{
		const bool bTimedOut = !bAllConditionsMet && (PostLoadElapsed >= Timing.MaxTime);
		CompleteTransition(bTimedOut);
		return false; // Stop ticking
	}

	// EN: Still waiting for conditions — update progress
	// ES: Aun esperando condiciones — actualizar progreso
	const float ConditionProgress = FMath::Clamp(
		(PostLoadElapsed - Timing.MinTime) / (Timing.MaxTime - Timing.MinTime),
		0.0f, 1.0f);
	TransitionProgressValue = 0.8f + ConditionProgress * 0.15f;
	BroadcastProgressUpdate();

	return true; // Continue ticking
}

// ============================================================================
// Transition Pipeline — Complete
// ============================================================================

void UPGXLevelFlowSubsystem::CompleteTransition(bool bTimedOut)
{
	// Remove timing ticker
	if (TimingTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TimingTickerHandle);
		TimingTickerHandle.Reset();
	}

	SetTransitionState(EPGXLevelFlowState::Complete);

	TransitionProgressValue = 1.0f;

	if (bTimedOut)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow] Transition completed with TIMEOUT (%.2f s) — some conditions were not met"),
			PostLoadElapsed);
	}
	else
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] Transition completed in %.2f s"),
			PostLoadElapsed);
	}

	// Update level tracking
	PreviousLevelTag = ActiveTransitionFromTag;
	CurrentLevelTag = ActiveTransitionTargetTag;

	// Notify actor: level is ready
	if (APGXLevelFlowActor* Actor = CurrentLevelFlowActorRef.Get())
	{
		Actor->OnLevelReady();
	}

	// GameFlow: set the level's on-enter state
	SetGameFlowOnEnter();

	// Broadcast completion
	OnTransitionCompleted.Broadcast(CurrentLevelTag);
	OnTransitionCompletedNative.Broadcast(CurrentLevelTag);
	BroadcastProgressUpdate();

	// Record in history
	RecordTransition(bTimedOut, EPGXLevelFlowResultCode::Success);

	// Reset to idle
	ActiveTransitionTargetTag = FGameplayTag();
	ActiveTransitionFromTag = FGameplayTag();
	SetTransitionState(EPGXLevelFlowState::Idle);
}

// ============================================================================
// Transition Pipeline — Failure
// ============================================================================

void UPGXLevelFlowSubsystem::FailTransition(EPGXLevelFlowResultCode Code, const FString& Reason)
{
	PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow] Transition FAILED: %s (code: %d)"),
		*Reason, static_cast<int32>(Code));

	// Cleanup async work
	if (ActiveStreamableHandle.IsValid())
	{
		ActiveStreamableHandle->CancelHandle();
		ActiveStreamableHandle.Reset();
	}
	if (TimingTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TimingTickerHandle);
		TimingTickerHandle.Reset();
	}

	SetTransitionState(EPGXLevelFlowState::Failed);

	// Revert GameFlow
	RevertGameFlow();

	// Broadcast failure
	OnTransitionFailed.Broadcast(Reason, ActiveTransitionTargetTag);

	// Record in history
	RecordTransition(false, Code);

	// Reset to idle
	ActiveTransitionTargetTag = FGameplayTag();
	ActiveTransitionFromTag = FGameplayTag();
	TransitionProgressValue = 0.0f;
	SetTransitionState(EPGXLevelFlowState::Idle);
}

// ============================================================================
// Cancel Transition
// ============================================================================

FPGXLevelFlowResult UPGXLevelFlowSubsystem::CancelTransition()
{
	if (CurrentState == EPGXLevelFlowState::Idle)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::Cancelled,
			TEXT("No active transition to cancel"));
	}

	// EN: Cannot cancel during Transitioning — OpenLevel already called, world is changing
	// ES: No se puede cancelar durante Transitioning — OpenLevel ya llamado, mundo esta cambiando
	if (CurrentState == EPGXLevelFlowState::Transitioning)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::TransitionInProgress,
			TEXT("Cannot cancel: OpenLevel already called"));
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] Transition cancelled (was in state %d)"),
		static_cast<int32>(CurrentState));

	FailTransition(EPGXLevelFlowResultCode::Cancelled, TEXT("Transition cancelled by user"));

	return FPGXLevelFlowResult::MakeSuccess(TEXT("Transition cancelled"));
}

// ============================================================================
// Actor Registration
// ============================================================================

void UPGXLevelFlowSubsystem::RegisterLevelFlowActor(APGXLevelFlowActor* Actor)
{
	if (!Actor)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow] Attempted to register null actor"));
		return;
	}

	CurrentLevelFlowActorRef = Actor;

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] Actor registered: %s (LevelTag: %s)"),
		*Actor->GetName(),
		*Actor->LevelTag.ToString());

	// EN: If we're not transitioning and no current level tag set,
	//     adopt the actor's tag as the current level (editor startup / PIE)
	// ES: Si no estamos transicionando y no hay tag de nivel actual,
	//     adoptar el tag del actor como nivel actual (inicio de editor / PIE)
	if (CurrentState == EPGXLevelFlowState::Idle && !CurrentLevelTag.IsValid())
	{
		CurrentLevelTag = Actor->LevelTag;
		PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] Adopted current level from actor: %s"),
			*CurrentLevelTag.ToString());
	}
}

void UPGXLevelFlowSubsystem::UnregisterLevelFlowActor(APGXLevelFlowActor* Actor)
{
	if (!Actor) return;

	if (CurrentLevelFlowActorRef.Get() == Actor)
	{
		CurrentLevelFlowActorRef = nullptr;
		PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] Actor unregistered: %s"), *Actor->GetName());
	}
}

APGXLevelFlowActor* UPGXLevelFlowSubsystem::GetCurrentLevelFlowActor() const
{
	return CurrentLevelFlowActorRef.Get();
}

// ============================================================================
// Query API
// ============================================================================

bool UPGXLevelFlowSubsystem::ResolveLevelByTag(FGameplayTag LevelTag, FPGXLevelEntry& OutEntry) const
{
	const FPGXLevelEntry* Found = MergedLevelCatalog.Find(LevelTag);
	if (Found)
	{
		OutEntry = *Found;
		return true;
	}
	return false;
}

TArray<FGameplayTag> UPGXLevelFlowSubsystem::GetRegisteredLevelTags() const
{
	TArray<FGameplayTag> Tags;
	MergedLevelCatalog.GetKeys(Tags);
	return Tags;
}

// EN: IPGXTaggedRegistry facade — exposes MergedLevelCatalog via the canonical
//     tag-keyed read contract. Behavior-preserving: same data, unified interface
//     so cross-plugin consumers can treat LevelFlow like any tagged registry.
// ES: Fachada IPGXTaggedRegistry — expone MergedLevelCatalog via el contrato
//     canonico de lectura keyed-por-tag. Behavior-preserving: mismos datos,
//     interfaz unificada para que consumers cross-plugin traten LevelFlow
//     como cualquier registry tagged.
// Keep validation output aligned with the shared PGX result contract.
bool UPGXLevelFlowSubsystem::HasEntryByTag(FGameplayTag Tag) const
{
	return MergedLevelCatalog.Contains(Tag);
}

int32 UPGXLevelFlowSubsystem::GetCount() const
{
	return MergedLevelCatalog.Num();
}

void UPGXLevelFlowSubsystem::GetSnapshot(TArray<FGameplayTag>& OutTags) const
{
	MergedLevelCatalog.GetKeys(OutTags);
}

// ============================================================================
// Sub-Level API (Basic Implementation)
// ============================================================================

FPGXLevelFlowResult UPGXLevelFlowSubsystem::RequestSubLevelLoad(FGameplayTag SubLevelTag)
{
	if (!SubLevelTag.IsValid())
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::TagNotFound,
			TEXT("Invalid sub-level tag"));
	}

	if (LoadedSubLevels.Contains(SubLevelTag))
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::AlreadyInLevel,
			FString::Printf(TEXT("Sub-level '%s' already loaded"), *SubLevelTag.ToString()));
	}

	// EN: Apply the active platform-profile level-flow budget.
	//     EnforcedMaxLoadedSubLevels = 0 means no platform constraint; > 0
	//     enforces the cap before the underlying ULevelStreamingDynamic
	//     load is attempted, so the platform budget is honored deterministically
	//     instead of silently overshot.
	// ES: Aplicar el budget de level-flow del perfil de plataforma activo.
	//     EnforcedMaxLoadedSubLevels = 0 significa sin restriccion de plataforma;
	//     > 0 enforce el cap antes de intentar el load del ULevelStreamingDynamic
	//     subyacente, para que el budget de plataforma se honre deterministicamente
	//     en lugar de sobrepasarse silenciosamente.
	if (EnforcedMaxLoadedSubLevels > 0 && LoadedSubLevels.Num() >= EnforcedMaxLoadedSubLevels)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed,
			FString::Printf(
				TEXT("Sub-level '%s' rejected — platform budget cap reached (%d/%d loaded). "
				     "Increase platform LevelFlowBudgets.MaxLoadedSubLevels or unload before requesting more."),
				*SubLevelTag.ToString(),
				LoadedSubLevels.Num(),
				EnforcedMaxLoadedSubLevels));
	}

	// EN: Find sub-level in current level entry
	// ES: Buscar sub-nivel en la entrada de nivel actual
	const FPGXLevelEntry* CurrentEntry = MergedLevelCatalog.Find(CurrentLevelTag);
	if (!CurrentEntry)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::TagNotFound,
			TEXT("No current level entry — cannot resolve sub-level"));
	}

	const FPGXSubLevelEntry* SubEntry = nullptr;
	for (const FPGXSubLevelEntry& Entry : CurrentEntry->SubLevels)
	{
		if (Entry.SubLevelTag == SubLevelTag)
		{
			SubEntry = &Entry;
			break;
		}
	}

	if (!SubEntry)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::TagNotFound,
			FString::Printf(TEXT("Sub-level '%s' not found in current level"), *SubLevelTag.ToString()));
	}

	// EN: Use UE native streaming to load the sub-level
	// ES: Usar streaming nativo de UE para cargar el sub-nivel
	const FString SubLevelPath = SubEntry->SubLevelReference.ToSoftObjectPath().GetLongPackageName();
	if (SubLevelPath.IsEmpty())
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed,
			TEXT("Sub-level has empty reference"));
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed,
			TEXT("No valid world"));
	}

	// EN: Use LoadStreamLevel via latent action (BP pattern exposed to C++)
	// ES: Usar LoadStreamLevel via accion latente (patron BP expuesto a C++)
	bool bSuccess = false;
	ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstance(
		World,
		SubLevelPath,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		bSuccess
	);

	if (!bSuccess || !StreamingLevel)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed,
			FString::Printf(TEXT("Failed to stream sub-level '%s'"), *SubLevelTag.ToString()));
	}

	LoadedSubLevels.Add(SubLevelTag, StreamingLevel);
	OnSubLevelLoadedDelegate.Broadcast(SubLevelTag);

	if (APGXLevelFlowActor* Actor = CurrentLevelFlowActorRef.Get())
	{
		Actor->OnSubLevelLoaded(SubLevelTag);
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] Sub-level loaded: %s"), *SubLevelTag.ToString());

	return FPGXLevelFlowResult::MakeSuccess(
		FString::Printf(TEXT("Sub-level '%s' loaded"), *SubLevelTag.ToString()));
}

FPGXLevelFlowResult UPGXLevelFlowSubsystem::RequestSubLevelUnload(FGameplayTag SubLevelTag)
{
	if (!SubLevelTag.IsValid())
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::TagNotFound,
			TEXT("Invalid sub-level tag"));
	}

	if (!LoadedSubLevels.Contains(SubLevelTag))
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::TagNotFound,
			FString::Printf(TEXT("Sub-level '%s' is not loaded"), *SubLevelTag.ToString()));
	}

	// EN: Execute real unload via UE streaming, then remove from tracking
	// ES: Ejecutar descarga real via streaming de UE, luego remover del tracking
	if (TWeakObjectPtr<ULevelStreamingDynamic>* Found = LoadedSubLevels.Find(SubLevelTag))
	{
		if (ULevelStreamingDynamic* StreamingLevel = Found->Get())
		{
			StreamingLevel->SetShouldBeLoaded(false);
			StreamingLevel->SetShouldBeVisible(false);
			StreamingLevel->SetIsRequestingUnloadAndRemoval(true);
		}
	}
	LoadedSubLevels.Remove(SubLevelTag);
	OnSubLevelUnloadedDelegate.Broadcast(SubLevelTag);

	if (APGXLevelFlowActor* Actor = CurrentLevelFlowActorRef.Get())
	{
		Actor->OnSubLevelUnloaded(SubLevelTag);
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow] Sub-level unloaded: %s"), *SubLevelTag.ToString());

	return FPGXLevelFlowResult::MakeSuccess(
		FString::Printf(TEXT("Sub-level '%s' unloaded"), *SubLevelTag.ToString()));
}

bool UPGXLevelFlowSubsystem::IsSubLevelLoaded(FGameplayTag SubLevelTag) const
{
	return LoadedSubLevels.Contains(SubLevelTag);
}

TArray<FGameplayTag> UPGXLevelFlowSubsystem::GetLoadedSubLevels() const
{
	TArray<FGameplayTag> Tags;
	LoadedSubLevels.GetKeys(Tags);
	return Tags;
}

// ============================================================================
// GameFlow Integration
// ============================================================================

namespace
{
	constexpr const TCHAR* PGXLoadingGameFlowSetStateChannelName = TEXT("PGX.Loading.GameFlow.SetState");
	constexpr const TCHAR* PGXLoadingGameFlowRevertChannelName = TEXT("PGX.Loading.GameFlow.Revert");

	FGameplayTag ResolveLoadingGameFlowChannel(const TCHAR* ChannelName)
	{
		return FGameplayTag::RequestGameplayTag(FName(ChannelName), false);
	}
}

void UPGXLevelFlowSubsystem::SetGameFlowLoading()
{
	if (!ActiveConfig || !ActiveConfig->bAutoIntegrateGameFlow) return;
	if (!ActiveConfig->GameFlowLoadingTag.IsValid()) return;

	PublishGameFlowSetState(ActiveConfig->GameFlowLoadingTag);
}

void UPGXLevelFlowSubsystem::SetGameFlowOnEnter()
{
	if (!ActiveConfig || !ActiveConfig->bAutoIntegrateGameFlow) return;
	if (!ActiveTransitionEntry.GameFlowTagOnEnter.IsValid()) return;

	PublishGameFlowSetState(ActiveTransitionEntry.GameFlowTagOnEnter);
}

void UPGXLevelFlowSubsystem::RevertGameFlow()
{
	if (!ActiveConfig || !ActiveConfig->bAutoIntegrateGameFlow) return;

	const FGameplayTag RevertChannel = ResolveLoadingGameFlowChannel(PGXLoadingGameFlowRevertChannelName);
	if (!RevertChannel.IsValid())
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("LevelFlow GameFlow revert skipped: channel tag '%s' is unavailable."), PGXLoadingGameFlowRevertChannelName);
		return;
	}

	UPGXMessageSubsystem* MessageSubsystem = UPGXMessageSubsystem::Get(this);
	if (!MessageSubsystem)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("LevelFlow GameFlow revert skipped: PGXMessageSubsystem unavailable."));
		return;
	}

	FPGXMessage Message;
	Message.MessageTag = RevertChannel;
	Message.Owner = this;
	Message.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	MessageSubsystem->BroadcastMessage<FPGXMessage>(RevertChannel, Message);
}

void UPGXLevelFlowSubsystem::PublishGameFlowSetState(FGameplayTag TargetStateTag)
{
	const FGameplayTag SetStateChannel = ResolveLoadingGameFlowChannel(PGXLoadingGameFlowSetStateChannelName);
	if (!SetStateChannel.IsValid())
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("LevelFlow GameFlow set-state skipped: channel tag '%s' is unavailable."), PGXLoadingGameFlowSetStateChannelName);
		return;
	}

	UPGXMessageSubsystem* MessageSubsystem = UPGXMessageSubsystem::Get(this);
	if (!MessageSubsystem)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("LevelFlow GameFlow set-state skipped: PGXMessageSubsystem unavailable."));
		return;
	}

	FPGXMessage Message;
	Message.MessageTag = TargetStateTag;
	Message.Owner = this;
	Message.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	MessageSubsystem->BroadcastMessage<FPGXMessage>(SetStateChannel, Message);
}

// ============================================================================
// Helpers
// ============================================================================

FString UPGXLevelFlowSubsystem::ResolveLevelPath(const FPGXLevelEntry& Entry) const
{
	// EN: Use explicit LevelPath if set, otherwise derive from LevelReference
	// ES: Usar LevelPath explicito si esta seteado, sino derivar de LevelReference
	if (!Entry.LevelPath.IsEmpty())
	{
		return Entry.LevelPath;
	}

	const FSoftObjectPath AssetPath = Entry.LevelReference.ToSoftObjectPath();
	if (AssetPath.IsValid())
	{
		return AssetPath.GetLongPackageName();
	}

	return FString();
}

const FPGXTransitionTiming& UPGXLevelFlowSubsystem::GetActiveTiming() const
{
	return ActiveTransitionEntry.Timing;
}

void UPGXLevelFlowSubsystem::BroadcastProgressUpdate()
{
	const FGameplayTag& RelevantTag = ActiveTransitionTargetTag.IsValid()
		? ActiveTransitionTargetTag
		: CurrentLevelTag;

	OnTransitionProgress.Broadcast(TransitionProgressValue, RelevantTag);
}

void UPGXLevelFlowSubsystem::RecordTransition(bool bTimedOut, EPGXLevelFlowResultCode Code)
{
	FPGXLevelTransitionRecord Record;
	Record.FromLevelTag = ActiveTransitionFromTag;
	Record.ToLevelTag = ActiveTransitionTargetTag;
	Record.Timestamp = FDateTime::Now();
	Record.LoadDurationSeconds = static_cast<float>(FPlatformTime::Seconds() - TransitionStartTime) - PostLoadElapsed;
	Record.WaitDurationSeconds = PostLoadElapsed;
	Record.ShaderCompilationsOnEntry = ShadersOnEntry;
	Record.bTimedOut = bTimedOut;
	Record.ResultCode = Code;

	TransitionHistory.Add(Record);

	// EN: Trim history to limit
	// ES: Recortar historial al limite
	const int32 MaxDepth = ActiveConfig ? ActiveConfig->MaxHistoryDepth : 50;
	while (TransitionHistory.Num() > MaxDepth)
	{
		TransitionHistory.RemoveAt(0);
	}
}

// ============================================================================
// Console Commands
// ============================================================================

void UPGXLevelFlowSubsystem::ExecuteConsoleCommand(const FString& CommandName, const TArray<FString>& Args, UWorld* World)
{
	if (CommandName == TEXT("pgx.level.cancel"))
	{
		const FPGXLevelFlowResult Result = CancelTransition();
		PGX_LOG_INFO(LogPGXLoading, TEXT("CancelTransition: %s (%s)"),
			Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"),
			*Result.Description);
		return;
	}
	if (CommandName == TEXT("pgx.level.entrypoints"))
	{
		APGXLevelFlowActor* Actor = CurrentLevelFlowActorRef.Get();
		if (!Actor)
		{
			PGX_LOG_INFO(LogPGXLoading, TEXT("No LevelFlowActor in current level"));
			return;
		}
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== Entry Points (%s) ==="), *Actor->LevelTag.ToString());
		for (const auto& Pair : Actor->EntryPoints)
		{
			const FVector Loc = Pair.Value.GetLocation();
			PGX_LOG_INFO(LogPGXLoading, TEXT("  '%s' → (%.1f, %.1f, %.1f)"),
				*Pair.Key.ToString(), Loc.X, Loc.Y, Loc.Z);
		}
		return;
	}
	if (CommandName == TEXT("pgx.level.history"))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== Transition History (%d entries) ==="), TransitionHistory.Num());
		for (int32 i = 0; i < TransitionHistory.Num(); ++i)
		{
			const FPGXLevelTransitionRecord& R = TransitionHistory[i];
			PGX_LOG_INFO(LogPGXLoading, TEXT("  [%d] %s → %s | Load: %.2fs | Wait: %.2fs | Shaders: %d | Timeout: %s | Result: %d"),
				i,
				R.FromLevelTag.IsValid() ? *R.FromLevelTag.ToString() : TEXT("(none)"),
				R.ToLevelTag.IsValid() ? *R.ToLevelTag.ToString() : TEXT("(none)"),
				R.LoadDurationSeconds,
				R.WaitDurationSeconds,
				R.ShaderCompilationsOnEntry,
				R.bTimedOut ? TEXT("YES") : TEXT("NO"),
				static_cast<int32>(R.ResultCode));
		}
		return;
	}
	if (CommandName == TEXT("pgx.level.load"))
	{
		if (Args.Num() < 1)
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("Usage: pgx.level.load <GameplayTag>"));
			return;
		}
		const FGameplayTag LevelTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
		if (!LevelTag.IsValid())
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("Invalid tag: %s"), *Args[0]);
			return;
		}
		const FPGXLevelFlowResult Result = RequestLevelTransition(LevelTag, nullptr);
		PGX_LOG_INFO(LogPGXLoading, TEXT("LoadLevel '%s': %s (%s)"),
			*LevelTag.ToString(),
			Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"),
			*Result.Description);
		return;
	}
	if (CommandName == TEXT("pgx.level.profiles"))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== Level Profiles (%d) ==="), DiscoveredProfiles.Num());
		for (const UPGXLevelProfile* Profile : DiscoveredProfiles)
		{
			if (!Profile) continue;
			PGX_LOG_INFO(LogPGXLoading, TEXT("  '%s' — %d levels"),
				*Profile->GetName(), Profile->LevelCatalog.Num());
		}
		return;
	}
	if (CommandName == TEXT("pgx.level.resolve"))
	{
		if (Args.Num() < 1)
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("Usage: pgx.level.resolve <GameplayTag>"));
			return;
		}
		const FGameplayTag LevelTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
		if (!LevelTag.IsValid())
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("Invalid tag: %s"), *Args[0]);
			return;
		}
		FPGXLevelEntry Entry;
		if (ResolveLevelByTag(LevelTag, Entry))
		{
			PGX_LOG_INFO(LogPGXLoading, TEXT("=== Level: %s ==="), *LevelTag.ToString());
			PGX_LOG_INFO(LogPGXLoading, TEXT("  DisplayName: %s"), *Entry.DisplayName.ToString());
			PGX_LOG_INFO(LogPGXLoading, TEXT("  LevelPath: %s"), *ResolveLevelPath(Entry));
			PGX_LOG_INFO(LogPGXLoading, TEXT("  LoadStrategy: %d"), static_cast<int32>(Entry.LoadStrategy));
			PGX_LOG_INFO(LogPGXLoading, TEXT("  TransitionMode: %d"), static_cast<int32>(Entry.TransitionMode));
			PGX_LOG_INFO(LogPGXLoading, TEXT("  Timing: Min=%.2f, Max=%.2f, WaitShader=%s"),
				Entry.Timing.MinTime, Entry.Timing.MaxTime,
				Entry.Timing.bWaitForShaderCompilation ? TEXT("YES") : TEXT("NO"));
			PGX_LOG_INFO(LogPGXLoading, TEXT("  SubLevels: %d"), Entry.SubLevels.Num());
			PGX_LOG_INFO(LogPGXLoading, TEXT("  GameFlowOnEnter: %s"),
				Entry.GameFlowTagOnEnter.IsValid() ? *Entry.GameFlowTagOnEnter.ToString() : TEXT("(none)"));
		}
		else
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("Tag '%s' not found in catalog"), *LevelTag.ToString());
		}
		return;
	}
	if (CommandName == TEXT("pgx.level.status"))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== LevelFlow Status ==="));
		PGX_LOG_INFO(LogPGXLoading, TEXT("  State: %d"), static_cast<int32>(CurrentState));
		PGX_LOG_INFO(LogPGXLoading, TEXT("  Current Level: %s"),
			CurrentLevelTag.IsValid() ? *CurrentLevelTag.ToString() : TEXT("(none)"));
		PGX_LOG_INFO(LogPGXLoading, TEXT("  Previous Level: %s"),
			PreviousLevelTag.IsValid() ? *PreviousLevelTag.ToString() : TEXT("(none)"));
		PGX_LOG_INFO(LogPGXLoading, TEXT("  Registered Levels: %d"), MergedLevelCatalog.Num());
		PGX_LOG_INFO(LogPGXLoading, TEXT("  Loaded Sub-Levels: %d"), LoadedSubLevels.Num());
		PGX_LOG_INFO(LogPGXLoading, TEXT("  History Entries: %d"), TransitionHistory.Num());
		PGX_LOG_INFO(LogPGXLoading, TEXT("  Actor: %s"),
			CurrentLevelFlowActorRef.IsValid() ? *CurrentLevelFlowActorRef->GetName() : TEXT("(none)"));
		PGX_LOG_INFO(LogPGXLoading, TEXT("  Shaders Pending: %d"), FShaderPipelineCache::NumPrecompilesRemaining());
		return;
	}
	if (CommandName == TEXT("pgx.level.sublevels"))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== Sub-Levels ==="));
		PGX_LOG_INFO(LogPGXLoading, TEXT("  Loaded: %d"), LoadedSubLevels.Num());
		for (const auto& Pair : LoadedSubLevels)
		{
			PGX_LOG_INFO(LogPGXLoading, TEXT("    %s [Streaming=%s]"),
				*Pair.Key.ToString(),
				Pair.Value.IsValid() ? TEXT("Valid") : TEXT("Stale"));
		}

		// EN: Show available sub-levels from current level entry
		// ES: Mostrar sub-niveles disponibles de la entrada de nivel actual
		const FPGXLevelEntry* CurrentEntry = MergedLevelCatalog.Find(CurrentLevelTag);
		if (CurrentEntry && CurrentEntry->SubLevels.Num() > 0)
		{
			PGX_LOG_INFO(LogPGXLoading, TEXT("  Available in '%s':"),
				CurrentLevelTag.IsValid() ? *CurrentLevelTag.ToString() : TEXT("(none)"));
			for (const FPGXSubLevelEntry& Sub : CurrentEntry->SubLevels)
			{
				const bool bLoaded = LoadedSubLevels.Contains(Sub.SubLevelTag);
				PGX_LOG_INFO(LogPGXLoading, TEXT("    %s [%s] Priority:%d AutoLoad:%s"),
					*Sub.SubLevelTag.ToString(),
					bLoaded ? TEXT("LOADED") : TEXT("UNLOADED"),
					Sub.LoadPriority,
					Sub.bAutoLoadOnEntry ? TEXT("YES") : TEXT("NO"));
			}
		}
		return;
	}
}


// ============================================================================
// Profile Integration
// ============================================================================

void UPGXLevelFlowSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& Profile)
{
	// EN: Read the platform-profile level-flow budget
	//     LevelFlowBudgets and ASSIGN them to subsystem members so the runtime
	//     gate (RequestSubLevel for MaxLoadedSubLevels; CVar r.Streaming.PoolSize
	//     for StreamingPool_MB) enforces them through the active runtime gates.
	// ES: Leer LevelFlowBudgets del perfil de plataforma
	//     de plataforma y ASIGNARLOS a miembros del subsistema para que el gate
	//     runtime (RequestSubLevel para MaxLoadedSubLevels; CVar r.Streaming.
	//     PoolSize para StreamingPool_MB) pueda enforcearlos. La impl previa
	//     leia los budgets a vars locales y solo logueaba, dejando el gate sin
	//     cerrar.
	EnforcedStreamingPool_MB = 0;
	EnforcedMaxLoadedSubLevels = 0;

	if (auto* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
	{
		if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
		{
			const auto& B = PlatformCfg->LevelFlowBudgets;
			EnforcedStreamingPool_MB = B.StreamingPool_MB;
			EnforcedMaxLoadedSubLevels = B.MaxLoadedSubLevels;
		}
	}

	// EN: Apply r.Streaming.PoolSize CVar when the platform budget specifies one
	//     (> 0). Skipped when 0 so projects without platform-budget constraints
	//     keep the engine default. Logged for diagnostics.
	// ES: Aplicar el CVar r.Streaming.PoolSize cuando el budget de plataforma
	//     especifica uno (> 0). Skipped cuando 0 para que proyectos sin
	//     restricciones platform-budget mantengan el default del motor. Logged
	//     para diagnostica.
	if (EnforcedStreamingPool_MB > 0)
	{
		if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize")))
		{
			CVar->Set(EnforcedStreamingPool_MB, ECVF_SetByGameSetting);
			PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlowSubsystem] r.Streaming.PoolSize set to %d MB by platform LevelFlowBudgets"),
				EnforcedStreamingPool_MB);
		}
		else
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlowSubsystem] r.Streaming.PoolSize CVar unavailable — platform budget %d MB not applied"),
				EnforcedStreamingPool_MB);
		}
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlowSubsystem] Profile constraints applied — StreamingPool=%dMB, MaxSubLevels=%d, GlobalStreamPool=%lld, VTex=%d"),
		EnforcedStreamingPool_MB, EnforcedMaxLoadedSubLevels,
		Profile.Budgets.StreamingPool_MB,
		static_cast<int32>(Profile.Features.VirtualTextures.Policy));
}

void UPGXLevelFlowSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}
