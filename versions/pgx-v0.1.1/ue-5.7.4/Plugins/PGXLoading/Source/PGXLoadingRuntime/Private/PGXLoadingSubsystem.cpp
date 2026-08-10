// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingRuntime.h"
#include "PGXLoadingConfig.h"
#include "PGXLoadingSettings.h"
#include "Utils/PGXConfigResolution.h"
#include "PGXLoadingProfile.h"
#include "PGXViewportOverlayManager.h"
#include "PGXLoadingWidget.h"
#include "PGXLoadingStrategyBase.h"
#include "PGXLoadingStrategy_Minimal.h"
#include "PGXLoadingStrategy_Image.h"
#include "PGXLoadingStrategy_Slideshow.h"
#include "PGXLoadingStrategy_Material.h"
#include "PGXLoadingStrategy_Video.h"
#include "Base/PGXBaseMessaging.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/PGXMessage.h"
#include "Tags/PGXLoadingTags.h"
#include "Trace/PGXTraceHelper.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformTime.h"
#include "Curves/CurveFloat.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"
#include "PGXLevelFlowSubsystem.h"
#include "PGXLevelFlowTypes.h"

TWeakObjectPtr<UPGXLoadingSubsystem> UPGXLoadingSubsystem::CachedInstance;

// ============================================================================
// Lifecycle
// ============================================================================

void UPGXLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// EN: Cache instance for static access / ES: Cachear instancia para acceso estatico
	CachedInstance = this;

	// EN: Discover config DA / ES: Descubrir config DA
	DiscoverConfigs();

	// EN: Discover loading profiles and build context map / ES: Descubrir perfiles y construir mapa de contextos
	DiscoverProfiles();
	MergeContextMappings();

	// EN: Create overlay manager / ES: Crear gestor de overlay
	OverlayManager = NewObject<UPGXViewportOverlayManager>(this);

	// EN: Bind to PostLoadMapWithWorld (fires after any level is loaded)
	// ES: Enlazar a PostLoadMapWithWorld (se dispara despues de cargar cualquier nivel)
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &ThisClass::OnPostLoadMap);

	// EN: Bind to PSO bridge through PGX messages (no L2 runtime dependency)
	// ES: Enlazar al puente PSO via mensajes PGX (sin dependencia runtime L2)
	BindPSOMessageBridge();

	// EN: Bind to LevelFlow subsystem (auto-activation of loading screen on transitions)
	// ES: Enlazar al subsistema LevelFlow (auto-activacion de pantalla de carga en transiciones)
	BindLevelFlowSubsystem();

	// EN: Bind to network failure delegates (prevent stuck loading screens)
	// ES: Enlazar a delegados de fallo de red (prevenir pantallas de carga atascadas)
	BindNetworkFailureHandlers();

	// EN: Register console commands / ES: Registrar comandos de consola
	// EN: Register trace config (Infrastructure v0.4.0)
	// ES: Registrar config de trazabilidad
	if (LoadingConfig)
	{
		FPGXTraceHelper::RegisterSystemTraceConfig(
			FGameplayTag::RequestGameplayTag(FName("PGX.System.Loading")),
			LoadingConfig->TraceConfig);
	}

	bInitialized = true;

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

	int32 ProfileCount = DiscoveredProfiles.Num();
	int32 ContextCount = ContextMap.Num();
	PGX_LOG_INFO(LogPGXLoading, TEXT("PGX Loading Screen initialized: %d profiles, %d contexts, Config=%s"),
		ProfileCount, ContextCount,
		LoadingConfig ? *LoadingConfig->GetName() : TEXT("DEFAULT"));
}

void UPGXLoadingSubsystem::Deinitialize()
{
	// EN: Cleanup Profile delegate subscription / ES: Limpiar suscripcion a delegate de Profile
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* Profile = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			Profile->OnProfileChangedNative.RemoveAll(this);
		}
	}

	// EN: Stop all tickers / ES: Detener todos los tickers
	StopWatchdog();
	StopEvaluationTicker();
	StopFadeAnimation();
	StopPSOTimeout();

	// EN: Cancel deferred input restore / ES: Cancelar restauracion diferida de input
	if (InputFlushHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(InputFlushHandle);
		InputFlushHandle.Reset();
	}

	// EN: Hide overlay if still active / ES: Ocultar overlay si sigue activo
	if (OverlayManager && OverlayManager->IsOverlayActive())
	{
		OverlayManager->HideOverlay();
	}

	// EN: Unbind integration delegates / ES: Desenlazar delegados de integracion
	UnbindPSOMessageBridge();
	UnbindLevelFlowSubsystem();
	UnbindNetworkFailureHandlers();

	// EN: Unbind PostLoadMap / ES: Desenlazar PostLoadMap
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	// EN: Unregister console commands / ES: Desregistrar comandos de consola
	// EN: Unregister trace config / ES: Desregistrar config de trazabilidad
	FPGXTraceHelper::UnregisterSystemTraceConfig(
		FGameplayTag::RequestGameplayTag(FName("PGX.System.Loading")));

	// EN: Release handles / ES: Liberar handles
	if (ActiveLoadHandle.IsValid())
	{
		ActiveLoadHandle->CancelHandle();
		ActiveLoadHandle.Reset();
	}
	if (FadeCurveLoadHandle.IsValid())
	{
		FadeCurveLoadHandle->CancelHandle();
		FadeCurveLoadHandle.Reset();
	}

	ActiveStrategy = nullptr;
	ResolvedProfile = nullptr;
	LoadedFadeCurve = nullptr;
	OverlayManager = nullptr;
	LoadingConfig = nullptr;

	// EN: Clear cached instance / ES: Limpiar instancia cacheada
	if (CachedInstance.Get() == this)
	{
		CachedInstance.Reset();
	}

	bInitialized = false;

	PGX_LOG_INFO(LogPGXLoading, TEXT("PGX Loading Screen deinitialized."));

	Super::Deinitialize();
}

// ============================================================================
// Discovery
// ============================================================================

void UPGXLoadingSubsystem::DiscoverConfigs()
{
	// EN: Settings-first resolution with AssetRegistry fallback (deprecated)
	// ES: Resolucion Settings-first con fallback a AssetRegistry (deprecated)
	const UPGXLoadingSettings* Settings = GetDefault<UPGXLoadingSettings>();
	LoadingConfig = PGX::ResolveSingleConfig<UPGXLoadingConfig>(Settings->ActiveConfig, TEXT("Loading"));

	if (IsValid(LoadingConfig))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("Discovered LoadingConfig: %s"), *LoadingConfig->GetName());
	}

	if (!LoadingConfig)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("No UPGXLoadingConfig found — using defaults."));
	}
}

void UPGXLoadingSubsystem::DiscoverProfiles()
{
	DiscoveredProfiles.Empty();
	const UPGXLoadingSettings* Settings = GetDefault<UPGXLoadingSettings>();
	const bool bConfiguredTableAttempted = Settings && !Settings->LoadingProfileTable.IsNull();
#if WITH_DEV_AUTOMATION_TESTS
	bProfileTableFallbackUsedForTesting = false;
#endif
	if (bConfiguredTableAttempted)
	{
		if (UDataTable* Table = Settings->LoadingProfileTable.LoadSynchronous())
		{
			TArray<FName> RowNames = Table->GetRowNames();
			RowNames.Sort(FNameLexicalLess());
			TSet<UPGXLoadingProfile*> UniqueProfiles;
			for (const FName RowName : RowNames)
			{
				const FPGXLoadingProfileRow* Row = Table->FindRow<FPGXLoadingProfileRow>(RowName, TEXT("LoadingProfileDiscovery"), false);
				UPGXLoadingProfile* Profile = Row && !Row->ProfileRef.IsNull() ? Row->ProfileRef.LoadSynchronous() : nullptr;
				if (!IsValid(Profile))
				{
					PGX_LOG_WARNING(LogPGXLoading, TEXT("LoadingProfileTable row '%s' is null or invalid — skipped."), *RowName.ToString());
					continue;
				}
				if (UniqueProfiles.Contains(Profile))
				{
					PGX_LOG_WARNING(LogPGXLoading, TEXT("LoadingProfileTable row '%s' duplicates profile '%s' — skipped."), *RowName.ToString(), *Profile->GetName());
					continue;
				}
				UniqueProfiles.Add(Profile);
				DiscoveredProfiles.Add(Profile);
			}
			if (!DiscoveredProfiles.IsEmpty())
			{
				PGX_LOG_INFO(LogPGXLoading, TEXT("Resolved %d unique loading profiles from LoadingProfileTable."), DiscoveredProfiles.Num());
				return;
			}
			PGX_LOG_WARNING(LogPGXLoading, TEXT("LoadingProfileTable is empty or has no valid rows — falling back to AssetRegistry."));
		}
		else
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("LoadingProfileTable failed to load — falling back to AssetRegistry."));
		}
	}

#if WITH_DEV_AUTOMATION_TESTS
	bProfileTableFallbackUsedForTesting = bConfiguredTableAttempted;
#endif

	// EN: Scan AssetRegistry for all UPGXLoadingProfile DAs
	// ES: Escanear AssetRegistry buscando todos los DAs UPGXLoadingProfile
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	TArray<FAssetData> ProfileAssets;
	Registry.GetAssetsByClass(UPGXLoadingProfile::StaticClass()->GetClassPathName(), ProfileAssets, true);

	for (const FAssetData& AssetData : ProfileAssets)
	{
		if (UPGXLoadingProfile* Profile = Cast<UPGXLoadingProfile>(AssetData.GetAsset()))
		{
			DiscoveredProfiles.Add(Profile);
			PGX_LOG_INFO(LogPGXLoading, TEXT("Discovered LoadingProfile: %s (%d context tags)"),
				*Profile->GetName(), Profile->ContextTags.Num());
		}
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("Profile discovery complete: %d profiles found."), DiscoveredProfiles.Num());
}

void UPGXLoadingSubsystem::MergeContextMappings()
{
	// EN: Merge all profile ContextTags into a flat ContextMap (TMap<FGameplayTag, Profile>)
	//     If duplicate context tags found across profiles, last-discovered wins with a warning.
	// ES: Fusionar todos los ContextTags de perfiles en un ContextMap plano.
	//     Si hay tags duplicados entre perfiles, el ultimo descubierto gana con warning.
	ContextMap.Empty();

	for (UPGXLoadingProfile* Profile : DiscoveredProfiles)
	{
		if (!Profile) continue;

		for (const FGameplayTag& CtxTag : Profile->ContextTags)
		{
			if (!CtxTag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXLoading, TEXT("Profile '%s' has an invalid context tag — skipped."),
					*Profile->GetName());
				continue;
			}

			if (ContextMap.Contains(CtxTag))
			{
				PGX_LOG_WARNING(LogPGXLoading,
					TEXT("Duplicate context tag '%s' — profile '%s' overrides previous mapping."),
					*CtxTag.ToString(), *Profile->GetName());
			}

			ContextMap.Add(CtxTag, TSoftObjectPtr<UPGXLoadingProfile>(Profile));
		}
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("Context map built: %d unique context tags from %d profiles."),
		ContextMap.Num(), DiscoveredProfiles.Num());
}

// ============================================================================
// State Machine
// ============================================================================

bool UPGXLoadingSubsystem::CanTransitionTo(EPGXLoadingScreenState NewState) const
{
	switch (CurrentState)
	{
	case EPGXLoadingScreenState::Idle:
		return NewState == EPGXLoadingScreenState::Preparing;

	case EPGXLoadingScreenState::Preparing:
		return NewState == EPGXLoadingScreenState::FadingIn
			|| NewState == EPGXLoadingScreenState::Idle; // timeout/fail

	case EPGXLoadingScreenState::FadingIn:
		return NewState == EPGXLoadingScreenState::Active
			|| NewState == EPGXLoadingScreenState::FadingOut; // ForceClose

	case EPGXLoadingScreenState::Active:
		return NewState == EPGXLoadingScreenState::WaitingClose
			|| NewState == EPGXLoadingScreenState::FadingOut; // ForceClose

	case EPGXLoadingScreenState::WaitingClose:
		return NewState == EPGXLoadingScreenState::FadingOut;

	case EPGXLoadingScreenState::FadingOut:
		return NewState == EPGXLoadingScreenState::Idle;

	default:
		return false;
	}
}

void UPGXLoadingSubsystem::SetState(EPGXLoadingScreenState NewState)
{
	if (CurrentState == NewState) return;

	if (!CanTransitionTo(NewState))
	{
		PGX_LOG_ERROR(LogPGXLoading, TEXT("Invalid state transition: %s → %s"),
			*GetStateName(CurrentState), *GetStateName(NewState));
		ensureMsgf(false, TEXT("PGXLoading: Invalid state transition %s → %s"),
			*GetStateName(CurrentState), *GetStateName(NewState));
		return;
	}

	EPGXLoadingScreenState OldState = CurrentState;
	CurrentState = NewState;
	StateEntryTime = FPlatformTime::Seconds();

	PGX_LOG_INFO(LogPGXLoading, TEXT("State: %s → %s (context=%s)"),
		*GetStateName(OldState), *GetStateName(NewState),
		*CurrentContextTag.ToString());

	// EN: Fire delegates / ES: Disparar delegados
	OnLoadingStateChanged.Broadcast(NewState, OldState);
	OnLoadingStateChangedNative.Broadcast(NewState, OldState);
}

FString UPGXLoadingSubsystem::GetStateName(EPGXLoadingScreenState State) const
{
	switch (State)
	{
	case EPGXLoadingScreenState::Idle:         return TEXT("Idle");
	case EPGXLoadingScreenState::Preparing:    return TEXT("Preparing");
	case EPGXLoadingScreenState::FadingIn:     return TEXT("FadingIn");
	case EPGXLoadingScreenState::Active:       return TEXT("Active");
	case EPGXLoadingScreenState::WaitingClose: return TEXT("WaitingClose");
	case EPGXLoadingScreenState::FadingOut:    return TEXT("FadingOut");
	default:                                   return TEXT("Unknown");
	}
}

// ============================================================================
// Public API
// ============================================================================

FPGXLoadingResult UPGXLoadingSubsystem::RequestLoading(FGameplayTag ContextTag)
{
	if (!bInitialized)
	{
		return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::AssetLoadFailed,
			TEXT("Subsystem not initialized"));
	}

	// EN: Handle reentry policy / ES: Manejar politica de reentrada
	if (CurrentState != EPGXLoadingScreenState::Idle)
	{
		EPGXLoadingReentryPolicy Policy = LoadingConfig
			? LoadingConfig->ReentryPolicy
			: EPGXLoadingReentryPolicy::Restart;

		switch (Policy)
		{
		case EPGXLoadingReentryPolicy::Ignore:
			return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::AlreadyActive,
				TEXT("Loading screen already active (policy=Ignore)"));

		case EPGXLoadingReentryPolicy::Restart:
			PGX_LOG_INFO(LogPGXLoading, TEXT("Reentry: Cancelling current loading to restart with %s"),
				*ContextTag.ToString());
			// EN: Force close current, then continue to start new
			// ES: Forzar cierre del actual, luego continuar para iniciar nuevo
			StopWatchdog();
			StopEvaluationTicker();
			StopFadeAnimation();
			StopPSOTimeout();
			if (ActiveStrategy) { ActiveStrategy->Deactivate(); }
			if (OverlayManager && OverlayManager->IsOverlayActive())
			{
				OverlayManager->HideOverlay();
			}
			// EN: Restore input before restart — prevents permanent UI-only lock
			// ES: Restaurar input antes de reiniciar — previene bloqueo permanente en UI-only
			RestoreInputState();
			RecordHistory(EPGXLoadingResultCode::Cancelled);
			ActiveStrategy = nullptr;
			LoadedFadeCurve = nullptr;
			CurrentState = EPGXLoadingScreenState::Idle; // Direct reset, skip normal pipeline
			break;

		case EPGXLoadingReentryPolicy::Queue:
		{
			// EN: Queue reentry is not implemented. Return a typed Unsupported result so
			//     callers can distinguish this capability from an AlreadyActive conflict.
			// ES: La reentrada Queue no esta implementada. Retornar Unsupported permite
			//     distinguir esta capacidad de un conflicto AlreadyActive.
			const FString QueueUnsupportedReason = TEXT(
				"Reentry policy 'Queue' is not yet implemented. "
				"Use Ignore or Restart for now.");
			PGX_LOG_WARNING(LogPGXLoading, TEXT("[LoadingSubsystem] %s"),
				*QueueUnsupportedReason);
			return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::Unsupported,
				QueueUnsupportedReason);
		}
		}
	}

	// EN: Validate context tag / ES: Validar tag de contexto
	if (!ContextTag.IsValid())
	{
		ContextTag = TAG_PGX_Loading_Context_Default;
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("RequestLoading: %s"), *ContextTag.ToString());

	// EN: Begin pipeline / ES: Iniciar pipeline
	BeginPreparing(ContextTag);

	return FPGXLoadingResult::MakeSuccess(
		FString::Printf(TEXT("Loading requested: %s"), *ContextTag.ToString()));
}

FPGXLoadingResult UPGXLoadingSubsystem::ForceClose()
{
	if (CurrentState == EPGXLoadingScreenState::Idle)
	{
		return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::ForceClosed,
			TEXT("No loading screen active"));
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("ForceClose from state: %s"), *GetStateName(CurrentState));

	// EN: Stop all tickers / ES: Detener todos los tickers
	StopWatchdog();
	StopEvaluationTicker();
	StopFadeAnimation();
	StopPSOTimeout();

	// EN: If in FadingOut already, let it complete naturally
	// ES: Si ya esta en FadingOut, dejar que complete naturalmente
	if (CurrentState == EPGXLoadingScreenState::FadingOut)
	{
		return FPGXLoadingResult::MakeSuccess(TEXT("Already fading out"));
	}

	// EN: Force transition to FadingOut / ES: Forzar transicion a FadingOut
	if (CanTransitionTo(EPGXLoadingScreenState::FadingOut))
	{
		BeginFadeOut();
	}
	else
	{
		// EN: Emergency path: direct to Idle (from Preparing)
		// ES: Ruta de emergencia: directo a Idle (desde Preparing)
		FailLoading(EPGXLoadingResultCode::ForceClosed, TEXT("ForceClose from ") + GetStateName(CurrentState));
	}

	return FPGXLoadingResult::MakeSuccess(TEXT("Force close initiated"));
}

FPGXLoadingResult UPGXLoadingSubsystem::RequestSkip()
{
	if (CurrentState != EPGXLoadingScreenState::Active &&
		CurrentState != EPGXLoadingScreenState::WaitingClose)
	{
		return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::ForceClosed,
			TEXT("Skip only available in Active/WaitingClose states"));
	}

	// EN: Check profile allows skip / ES: Verificar que el perfil permita skip
	if (ResolvedProfile && !ResolvedProfile->bAllowSkip)
	{
		return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::AlreadyActive,
			TEXT("Skip not allowed by active profile"));
	}

	// EN: Check skip conditions / ES: Verificar condiciones de skip
	if (!bMinTimeElapsed || !bPSOReady)
	{
		PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Skip conditions not met: MinTime=%d, PSO=%d"),
			bMinTimeElapsed, bPSOReady);
		return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::AlreadyActive,
			TEXT("Skip conditions not met (MinTime or PSO pending)"));
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("Skip accepted — beginning fade out."));
	BeginFadeOut();

	return FPGXLoadingResult::MakeSuccess(TEXT("Skip accepted"));
}

float UPGXLoadingSubsystem::GetElapsedTime() const
{
	if (CurrentState == EPGXLoadingScreenState::Idle)
	{
		return 0.0f;
	}
	return static_cast<float>(FPlatformTime::Seconds() - LoadingStartTime);
}

bool UPGXLoadingSubsystem::IsProfileValid(FGameplayTag ContextTag) const
{
	if (const TSoftObjectPtr<UPGXLoadingProfile>* Found = ContextMap.Find(ContextTag))
	{
		return !Found->IsNull();
	}
	return false;
}

TArray<FGameplayTag> UPGXLoadingSubsystem::GetRegisteredContextTags() const
{
	TArray<FGameplayTag> Tags;
	ContextMap.GetKeys(Tags);
	return Tags;
}

// ============================================================================
// Strategy
// ============================================================================

UPGXLoadingStrategyBase* UPGXLoadingSubsystem::CreateStrategyForType(EPGXLoadingVisualType VisualType)
{
	UClass* StrategyClass = nullptr;

	switch (VisualType)
	{
	case EPGXLoadingVisualType::Minimal:
		StrategyClass = UPGXLoadingStrategy_Minimal::StaticClass();
		break;
	case EPGXLoadingVisualType::StaticImage:
		StrategyClass = UPGXLoadingStrategy_Image::StaticClass();
		break;
	case EPGXLoadingVisualType::Slideshow:
		StrategyClass = UPGXLoadingStrategy_Slideshow::StaticClass();
		break;
	case EPGXLoadingVisualType::MaterialAnimated:
		StrategyClass = UPGXLoadingStrategy_Material::StaticClass();
		break;
	case EPGXLoadingVisualType::Video:
		StrategyClass = UPGXLoadingStrategy_Video::StaticClass();
		break;
	case EPGXLoadingVisualType::Custom:
		if (ResolvedProfile && ResolvedProfile->CustomStrategyClass)
		{
			StrategyClass = ResolvedProfile->CustomStrategyClass;
		}
		else
		{
			PGX_LOG_WARNING(LogPGXLoading, TEXT("Custom VisualType but no CustomStrategyClass — falling back to Minimal."));
			StrategyClass = UPGXLoadingStrategy_Minimal::StaticClass();
		}
		break;
	default:
		StrategyClass = UPGXLoadingStrategy_Minimal::StaticClass();
		break;
	}

	if (StrategyClass)
	{
		UPGXLoadingStrategyBase* Strategy = NewObject<UPGXLoadingStrategyBase>(this, StrategyClass);
		PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy created: %s"), *StrategyClass->GetName());
		return Strategy;
	}

	return nullptr;
}

void UPGXLoadingSubsystem::CheckStrategyReady()
{
	// EN: Called from evaluation ticker while in Preparing state
	//     If strategy becomes ready, proceed to FadeIn
	// ES: Llamado desde ticker de evaluacion mientras esta en Preparing
	//     Si la estrategia esta lista, proceder a FadeIn
	if (CurrentState != EPGXLoadingScreenState::Preparing) return;

	if (ActiveStrategy && ActiveStrategy->IsReady())
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy ready — proceeding to FadeIn."));
		BeginFadeIn();
	}
}

// ============================================================================
// Pipeline
// ============================================================================

void UPGXLoadingSubsystem::BeginPreparing(FGameplayTag ContextTag)
{
	CurrentContextTag = ContextTag;
	LoadingStartTime = FPlatformTime::Seconds();
	PreparingStartTime = LoadingStartTime;
	ActiveVisualType = EPGXLoadingVisualType::Minimal;

	// EN: Resolve profile from context map / ES: Resolver perfil desde mapa de contextos
	ResolvedProfile = nullptr;
	if (const TSoftObjectPtr<UPGXLoadingProfile>* Found = ContextMap.Find(ContextTag))
	{
		ResolvedProfile = Found->Get();
	}

	if (ResolvedProfile)
	{
		ActiveVisualType = ResolvedProfile->DefaultVisualType;
		PGX_LOG_INFO(LogPGXLoading, TEXT("Profile resolved: '%s' (VisualType=%d, MinTime=%.1fs, AllowSkip=%d)"),
			*ResolvedProfile->GetName(),
			static_cast<int32>(ResolvedProfile->DefaultVisualType),
			ResolvedProfile->MinDisplayTime,
			ResolvedProfile->bAllowSkip);
	}
	else
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("No profile for context '%s' — using Minimal fallback."),
			*ContextTag.ToString());
	}

	// EN: Reset close conditions / ES: Resetear condiciones de cierre
	bMinTimeElapsed = false;
	bPostLoadFramesElapsed = false;
	bPostLoadMapReceived = false;
	PostLoadFrameCount = 0;
	PSOProgressValue = 0.0f;
	bPSOBound = false;
	bPSOBridgeWarmUpActive = false;

	// EN: Determine if we need to wait for PSO (profile override → config default)
	// ES: Determinar si necesitamos esperar PSO (override de perfil → default de config)
	bool bShouldWaitPSO = ResolvedProfile
		? ResolvedProfile->bWaitForPSO
		: (LoadingConfig ? LoadingConfig->bWaitForPSOByDefault : true);

	RequestPSOBridgeState();

	if (bShouldWaitPSO && bPSOBound && bPSOBridgeWarmUpActive)
	{
		bPSOReady = false;
		StartPSOTimeout();
		PGX_LOG_INFO(LogPGXLoading, TEXT("PSO message bridge wait enabled — loading will wait for warm-up completion."));
	}
	else
	{
		bPSOReady = true;
		PSOProgressValue = 1.0f;
	}

	// EN: Reset progress / ES: Resetear progreso
	CurrentProgress = FPGXLoadingProgress();
	CurrentProgress.StatusMessage = FText::FromString(TEXT("Loading..."));

	SetState(EPGXLoadingScreenState::Preparing);

	// EN: Capture and block input / ES: Capturar y bloquear input
	CaptureInputState();
	BlockInput();

	// EN: Start preparing watchdog / ES: Iniciar watchdog de preparacion
	float Timeout = LoadingConfig ? LoadingConfig->PreparingTimeout : 5.0f;
	StartWatchdog(Timeout);

	// EN: Fire started delegates / ES: Disparar delegados de inicio
	OnLoadingStarted.Broadcast(ContextTag);
	OnLoadingStartedNative.Broadcast(ContextTag);

	// EN: Create strategy based on resolved profile's VisualType / ES: Crear estrategia segun VisualType
	ActiveStrategy = CreateStrategyForType(ActiveVisualType);
	if (ActiveStrategy)
	{
		ActiveStrategy->InitializeStrategy(ResolvedProfile);

		// EN: If strategy is immediately ready, proceed to FadeIn
		// ES: Si la estrategia esta lista inmediatamente, proceder a FadeIn
		if (ActiveStrategy->IsReady())
		{
			BeginFadeIn();
		}
		// EN: Otherwise, start evaluation ticker to poll IsReady + watchdog for timeout
		// ES: Si no, iniciar ticker de evaluacion para consultar IsReady + watchdog para timeout
		else
		{
			StartEvaluationTicker();
		}
	}
	else
	{
		// EN: No strategy created — proceed with minimal overlay
		// ES: Sin estrategia — proceder con overlay minimo
		BeginFadeIn();
	}
}

void UPGXLoadingSubsystem::BeginFadeIn()
{
	StopWatchdog();

	SetState(EPGXLoadingScreenState::FadingIn);
	FadeStartTime = FPlatformTime::Seconds();

	// EN: Determine widget class from profile (if any) / ES: Determinar clase de widget del perfil
	TSubclassOf<UUserWidget> WidgetClass = nullptr;
	if (ResolvedProfile && ResolvedProfile->WidgetClassOverride)
	{
		WidgetClass = ResolvedProfile->WidgetClassOverride;
	}

	// EN: Show overlay starting at opacity 0 / ES: Mostrar overlay comenzando en opacidad 0
	int32 ZOrder = LoadingConfig ? LoadingConfig->OverlayZOrder : 1000;
	if (OverlayManager && !OverlayManager->IsOverlayActive())
	{
		OverlayManager->ShowOverlay(ZOrder, WidgetClass);
		OverlayManager->SetOverlayOpacity(0.0f);
	}

	// EN: Notify widget that fade in is starting / ES: Notificar al widget que inicia fade in
	if (OverlayManager)
	{
		UPGXLoadingWidget* LoadingWidget = Cast<UPGXLoadingWidget>(OverlayManager->GetActiveWidget());
		if (LoadingWidget)
		{
			FPGXFadeConfig FadeConf = ResolveFadeConfig();
			LoadingWidget->OnFadeInStarted(FadeConf.FadeInDuration);
		}
	}

	// EN: Start animated fade (0 → 1) / ES: Iniciar fade animado (0 → 1)
	StartFadeAnimation(/*bFadeIn=*/ true);
}

void UPGXLoadingSubsystem::ActivateOverlay()
{
	SetState(EPGXLoadingScreenState::Active);
	ActiveStartTime = FPlatformTime::Seconds();

	// EN: Ensure full opacity during Active / ES: Asegurar opacidad total en Active
	if (OverlayManager)
	{
		OverlayManager->SetOverlayOpacity(1.0f);
	}

	// EN: Activate strategy on the overlay widget / ES: Activar estrategia en el widget del overlay
	if (ActiveStrategy && OverlayManager)
	{
		UPGXLoadingWidget* LoadingWidget = Cast<UPGXLoadingWidget>(OverlayManager->GetActiveWidget());
		if (LoadingWidget)
		{
			ActiveStrategy->Activate(LoadingWidget);

			// EN: Notify widget of activation / ES: Notificar widget de activacion
			LoadingWidget->OnLoadingActivated(CurrentContextTag, ActiveVisualType);

			// EN: Wire skip button / ES: Enlazar boton skip
			LoadingWidget->BindSkipButton(FSimpleDelegate::CreateLambda([this]()
			{
				RequestSkip();
			}));

			// EN: Wire continue button for ManualClose policy
			//     Continue button is only visible when ClosePolicy == ManualOnly
			// ES: Enlazar boton continuar para politica ManualClose
			EPGXLoadingClosePolicy Policy = ResolvedProfile
				? ResolvedProfile->ClosePolicyOverride
				: (LoadingConfig ? LoadingConfig->DefaultClosePolicy : EPGXLoadingClosePolicy::Automatic);

			bool bShowContinue = (Policy == EPGXLoadingClosePolicy::ManualOnly);
			LoadingWidget->SetContinueButtonVisible(bShowContinue);
			if (bShowContinue)
			{
				LoadingWidget->BindContinueButton(FSimpleDelegate::CreateLambda([this]()
				{
					ForceClose();
				}));
			}
		}
		else
		{
			// EN: Widget is not UPGXLoadingWidget — strategy still activates with nullptr
			// ES: Widget no es UPGXLoadingWidget — estrategia se activa con nullptr
			ActiveStrategy->Activate(nullptr);
		}
	}

	// EN: Start evaluation ticker to check close conditions
	// ES: Iniciar ticker de evaluacion para verificar condiciones de cierre
	StartEvaluationTicker();
}

void UPGXLoadingSubsystem::BeginWaitingClose()
{
	SetState(EPGXLoadingScreenState::WaitingClose);
	WaitingStartTime = FPlatformTime::Seconds();

	// EN: Ensure full opacity during WaitingClose / ES: Asegurar opacidad total en WaitingClose
	if (OverlayManager)
	{
		OverlayManager->SetOverlayOpacity(1.0f);
	}

	// EN: Start waiting watchdog / ES: Iniciar watchdog de espera
	float Timeout = LoadingConfig ? LoadingConfig->WaitingCloseTimeout : 20.0f;
	StartWatchdog(Timeout);
}

void UPGXLoadingSubsystem::EvaluateCloseConditions()
{
	if (CurrentState != EPGXLoadingScreenState::Active &&
		CurrentState != EPGXLoadingScreenState::WaitingClose)
	{
		return;
	}

	// EN: Check MinDisplayTime — profile override takes priority over config default,
	//     and the platform-profile EnforcedMinLoadingScreenDuration acts as a FLOOR
	//     so platforms requiring a longer guaranteed display (consoles, certification
	//     requirements) cannot have their floor undercut by Profile / LoadingConfig
	//     overrides according to the active platform profile.
	// ES: Verificar MinDisplayTime — override del perfil tiene prioridad sobre el
	//     default de config, y el EnforcedMinLoadingScreenDuration platform-profile
	//     actua como FLOOR para que plataformas con display garantizado mas largo
	//     (consolas, certificacion) no vean su floor undercut por overrides de
	//     Profile / LoadingConfig.
	float MinTime = ResolvedProfile
		? ResolvedProfile->MinDisplayTime
		: (LoadingConfig ? LoadingConfig->DefaultMinDisplayTime : 1.0f);
	MinTime = FMath::Max(MinTime, EnforcedMinLoadingScreenDuration);
	double Elapsed = FPlatformTime::Seconds() - LoadingStartTime;
	bMinTimeElapsed = (Elapsed >= MinTime);

	// EN: Check PostLoad frames / ES: Verificar frames PostLoad
	if (bPostLoadMapReceived)
	{
		PostLoadFrameCount++;
		int32 RequiredFrames = LoadingConfig ? LoadingConfig->PostLoadFrameDelay : 2;
		bPostLoadFramesElapsed = (PostLoadFrameCount >= RequiredFrames);
	}

	// EN: Update progress with combined formula / ES: Actualizar progreso con formula combinada
	CurrentProgress.ElapsedTime = static_cast<float>(Elapsed);
	CurrentProgress.AssetProgress = 1.0f; // EN: Strategies report their own asset progress

	// EN: Combined progress: (1 - PSOWeight) * AssetProgress + PSOWeight * PSOProgress
	// ES: Progreso combinado: (1 - PesoPSO) * ProgresoAsset + PesoPSO * ProgresoPSO
	float PSOWeight = (LoadingConfig && !bPSOReady) ? LoadingConfig->PSOProgressWeight : 0.0f;
	CurrentProgress.TotalProgress = (1.0f - PSOWeight) * CurrentProgress.AssetProgress
		+ PSOWeight * PSOProgressValue;

	// EN: Forward progress to strategy / ES: Reenviar progreso a estrategia
	if (ActiveStrategy)
	{
		ActiveStrategy->OnProgressUpdated(CurrentProgress);
	}

	// EN: Broadcast progress delegates / ES: Emitir delegados de progreso
	OnLoadingProgressUpdated.Broadcast(CurrentProgress.TotalProgress, CurrentProgress.StatusMessage);
	OnLoadingProgressNative.Broadcast(CurrentProgress.TotalProgress, CurrentProgress.StatusMessage);

	// EN: Transition to WaitingClose if in Active and PostLoadMap received
	// ES: Transicion a WaitingClose si esta en Active y se recibio PostLoadMap
	if (CurrentState == EPGXLoadingScreenState::Active && bPostLoadMapReceived)
	{
		BeginWaitingClose();
		return;
	}

	// EN: Check all close conditions / ES: Verificar todas las condiciones de cierre
	if (CurrentState == EPGXLoadingScreenState::WaitingClose)
	{
		// EN: Profile override takes priority for close policy / ES: Override del perfil tiene prioridad
		EPGXLoadingClosePolicy Policy = ResolvedProfile
			? ResolvedProfile->ClosePolicyOverride
			: (LoadingConfig ? LoadingConfig->DefaultClosePolicy : EPGXLoadingClosePolicy::Automatic);

		if (Policy == EPGXLoadingClosePolicy::ManualOnly)
		{
			return; // EN: Only ForceClose can close / ES: Solo ForceClose puede cerrar
		}

		if (bMinTimeElapsed && bPSOReady && bPostLoadFramesElapsed)
		{
			PGX_LOG_INFO(LogPGXLoading, TEXT("All close conditions met — beginning fade out."));
			BeginFadeOut();
		}
	}
}

void UPGXLoadingSubsystem::BeginFadeOut()
{
	StopWatchdog();
	StopEvaluationTicker();
	StopPSOTimeout();

	SetState(EPGXLoadingScreenState::FadingOut);
	FadeStartTime = FPlatformTime::Seconds();

	// EN: Deactivate strategy before fading out (resources released during fade)
	// ES: Desactivar estrategia antes de fade out (recursos liberados durante fade)
	if (ActiveStrategy)
	{
		ActiveStrategy->Deactivate();
	}

	// EN: Notify widget that fade out is starting / ES: Notificar al widget que inicia fade out
	if (OverlayManager)
	{
		UPGXLoadingWidget* LoadingWidget = Cast<UPGXLoadingWidget>(OverlayManager->GetActiveWidget());
		if (LoadingWidget)
		{
			FPGXFadeConfig FadeConf = ResolveFadeConfig();
			LoadingWidget->OnFadeOutStarted(FadeConf.FadeOutDuration);
		}
	}

	// EN: Start animated fade (1 → 0) / ES: Iniciar fade animado (1 → 0)
	StartFadeAnimation(/*bFadeIn=*/ false);
}

void UPGXLoadingSubsystem::CompleteLoading()
{
	// EN: Strategy already deactivated in BeginFadeOut / ES: Estrategia ya desactivada en BeginFadeOut

	// EN: Notify widget of deactivation / ES: Notificar widget de desactivacion
	if (OverlayManager)
	{
		UPGXLoadingWidget* LoadingWidget = Cast<UPGXLoadingWidget>(OverlayManager->GetActiveWidget());
		if (LoadingWidget)
		{
			LoadingWidget->OnLoadingDeactivated();
		}
	}

	// EN: Hide overlay / ES: Ocultar overlay
	if (OverlayManager && OverlayManager->IsOverlayActive())
	{
		OverlayManager->HideOverlay();
	}

	// EN: Record history / ES: Registrar historial
	RecordHistory(EPGXLoadingResultCode::Success);

	// EN: Fire completed delegates / ES: Disparar delegados de completado
	if (LoadingHistory.Num() > 0)
	{
		const FPGXLoadingRecord& Record = LoadingHistory.Last();
		OnLoadingCompleted.Broadcast(Record);
		OnLoadingCompletedNative.Broadcast(Record);
	}

	// EN: Reset to Idle / ES: Resetear a Idle
	SetState(EPGXLoadingScreenState::Idle);

	// EN: Clean up active context / ES: Limpiar contexto activo
	CurrentContextTag = FGameplayTag();
	ActiveStrategy = nullptr;
	ResolvedProfile = nullptr;
	ActiveVisualType = EPGXLoadingVisualType::Minimal;
	LoadedFadeCurve = nullptr;

	// EN: Deferred input restore — wait 1 frame to prevent queued inputs from executing
	//     on the new level immediately after loading screen closes
	// ES: Restauracion diferida de input — esperar 1 frame para prevenir que inputs encolados
	//     se ejecuten en el nuevo nivel inmediatamente despues de cerrar la pantalla de carga
	ScheduleDeferredInputRestore();

	PGX_LOG_INFO(LogPGXLoading, TEXT("Loading completed successfully."));
}

void UPGXLoadingSubsystem::FailLoading(EPGXLoadingResultCode Code, const FString& Reason)
{
	PGX_LOG_WARNING(LogPGXLoading, TEXT("Loading failed: %s (Code=%d)"), *Reason, static_cast<int32>(Code));

	StopWatchdog();
	StopEvaluationTicker();
	StopFadeAnimation();
	StopPSOTimeout();

	// EN: Deactivate strategy / ES: Desactivar estrategia
	if (ActiveStrategy)
	{
		ActiveStrategy->Deactivate();
	}

	// EN: Hide overlay / ES: Ocultar overlay
	if (OverlayManager && OverlayManager->IsOverlayActive())
	{
		OverlayManager->HideOverlay();
	}

	// EN: Record history / ES: Registrar historial
	RecordHistory(Code);

	// EN: Fire failed delegates / ES: Disparar delegados de fallo
	OnLoadingFailed.Broadcast(Code, Reason);

	// EN: Direct to Idle (bypass state machine for emergency) / ES: Directo a Idle (bypass para emergencia)
	EPGXLoadingScreenState OldState = CurrentState;
	CurrentState = EPGXLoadingScreenState::Idle;
	StateEntryTime = FPlatformTime::Seconds();
	OnLoadingStateChanged.Broadcast(EPGXLoadingScreenState::Idle, OldState);
	OnLoadingStateChangedNative.Broadcast(EPGXLoadingScreenState::Idle, OldState);

	CurrentContextTag = FGameplayTag();
	ActiveStrategy = nullptr;
	ResolvedProfile = nullptr;
	ActiveVisualType = EPGXLoadingVisualType::Minimal;
	LoadedFadeCurve = nullptr;

	// EN: Deferred input restore / ES: Restauracion diferida de input
	ScheduleDeferredInputRestore();
}

void UPGXLoadingSubsystem::RecordHistory(EPGXLoadingResultCode Code)
{
	double Now = FPlatformTime::Seconds();

	FPGXLoadingRecord Record;
	Record.ContextTag = CurrentContextTag;
	Record.VisualType = ActiveVisualType;
	Record.Timestamp = FDateTime::Now();
	Record.TotalDuration = static_cast<float>(Now - LoadingStartTime);
	Record.PreparingDuration = static_cast<float>(
		(ActiveStartTime > 0.0 ? ActiveStartTime : Now) - PreparingStartTime);
	Record.ActiveDuration = static_cast<float>(
		(WaitingStartTime > 0.0 ? WaitingStartTime : Now) -
		(ActiveStartTime > 0.0 ? ActiveStartTime : Now));
	Record.WaitingDuration = static_cast<float>(
		(FadeStartTime > 0.0 ? FadeStartTime : Now) -
		(WaitingStartTime > 0.0 ? WaitingStartTime : Now));
	Record.FadeDuration = static_cast<float>(Now - (FadeStartTime > 0.0 ? FadeStartTime : Now));
	Record.ResultCode = Code;
	Record.bPSOWaited = !bPSOReady;
	Record.bTimedOut = (Code == EPGXLoadingResultCode::TimedOut);
	Record.bUserSkipped = false;

	LoadingHistory.Add(Record);

	// EN: Trim history / ES: Recortar historial
	int32 MaxDepth = LoadingConfig ? LoadingConfig->MaxHistoryDepth : 50;
	while (LoadingHistory.Num() > MaxDepth)
	{
		LoadingHistory.RemoveAt(0);
	}
}

// ============================================================================
// Integration Binding
// ============================================================================

void UPGXLoadingSubsystem::BindPSOMessageBridge()
{
	UnbindPSOMessageBridge();

	TWeakObjectPtr<ThisClass> WeakThis(this);
	PSOMessageHandles.Add(PGXBaseMessaging::Listen<FPGXBridgeLoadingState>(
		this,
		TAG_PGX_Loading_PSO_State.GetTag(),
		[WeakThis](FGameplayTag Channel, const FPGXBridgeLoadingState& Payload)
		{
			if (ThisClass* StrongThis = WeakThis.Get())
			{
				StrongThis->OnPSOBridgeState(Channel, Payload);
			}
		}));
	PSOMessageHandles.Add(PGXBaseMessaging::Listen<FPGXBridgeLoadingState>(
		this,
		TAG_PGX_Loading_PSO_Progress.GetTag(),
		[WeakThis](FGameplayTag Channel, const FPGXBridgeLoadingState& Payload)
		{
			if (ThisClass* StrongThis = WeakThis.Get())
			{
				StrongThis->OnPSOBridgeProgress(Channel, Payload);
			}
		}));
	PSOMessageHandles.Add(PGXBaseMessaging::Listen<FPGXBridgeLoadingState>(
		this,
		TAG_PGX_Loading_PSO_Complete.GetTag(),
		[WeakThis](FGameplayTag Channel, const FPGXBridgeLoadingState& Payload)
		{
			if (ThisClass* StrongThis = WeakThis.Get())
			{
				StrongThis->OnPSOBridgeComplete(Channel, Payload);
			}
		}));

	PGX_LOG_INFO(LogPGXLoading, TEXT("PSO message bridge listeners registered for Loading integration."));
	RequestPSOBridgeState();
}

void UPGXLoadingSubsystem::UnbindPSOMessageBridge()
{
	PGXBaseMessaging::UnregisterAll(PSOMessageHandles);
	bPSOBound = false;
	bPSOBridgeWarmUpActive = false;
}

void UPGXLoadingSubsystem::RequestPSOBridgeState()
{
	FGameplayTag QueryTag = FGameplayTag::RequestGameplayTag(FName(TEXT("PGX.PSO.Loading.QueryState")), false);
	if (!QueryTag.IsValid())
	{
		bPSOBound = false;
		bPSOBridgeWarmUpActive = false;
		return;
	}

	FPGXMessage Query;
	Query.MessageTag = QueryTag;
	Query.Owner = this;
	Query.Timestamp = FPlatformTime::Seconds();
	PGXBaseMessaging::Broadcast<FPGXMessage>(this, QueryTag, Query);
}

void UPGXLoadingSubsystem::BindLevelFlowSubsystem()
{
	// EN: Only bind if config enables auto-activation / ES: Solo enlazar si config habilita auto-activacion
	bool bAutoActivate = LoadingConfig ? LoadingConfig->bAutoActivateOnLevelFlow : true;
	if (!bAutoActivate) return;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UPGXLevelFlowSubsystem* LevelFlow = GI->GetSubsystem<UPGXLevelFlowSubsystem>();
	if (!LevelFlow)
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("LevelFlow subsystem not available — auto-activation disabled."));
		return;
	}

	LevelFlow->OnTransitionStartedNative.AddUObject(this, &ThisClass::OnLevelFlowStarted);
	LevelFlow->OnTransitionCompletedNative.AddUObject(this, &ThisClass::OnLevelFlowCompleted);

	PGX_LOG_INFO(LogPGXLoading, TEXT("LevelFlow subsystem bound for auto-activation."));
}

void UPGXLoadingSubsystem::UnbindLevelFlowSubsystem()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UPGXLevelFlowSubsystem* LevelFlow = GI->GetSubsystem<UPGXLevelFlowSubsystem>();
	if (LevelFlow)
	{
		LevelFlow->OnTransitionStartedNative.RemoveAll(this);
		LevelFlow->OnTransitionCompletedNative.RemoveAll(this);
	}
}

void UPGXLoadingSubsystem::BindNetworkFailureHandlers()
{
	if (!GEngine) return;

	NetworkFailureHandle = GEngine->OnNetworkFailure().AddUObject(
		this, &ThisClass::OnNetworkFailure);
	TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(
		this, &ThisClass::OnTravelFailure);

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Network failure handlers bound."));
}

void UPGXLoadingSubsystem::UnbindNetworkFailureHandlers()
{
	if (!GEngine) return;

	if (NetworkFailureHandle.IsValid())
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
		NetworkFailureHandle.Reset();
	}
	if (TravelFailureHandle.IsValid())
	{
		GEngine->OnTravelFailure().Remove(TravelFailureHandle);
		TravelFailureHandle.Reset();
	}
}

// ============================================================================
// Integration Listeners
// ============================================================================

void UPGXLoadingSubsystem::OnLevelFlowStarted(FGameplayTag LevelTag)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT("OnLevelFlowStarted: %s"), *LevelTag.ToString());

	// EN: If loading is already active, don't trigger another one
	// ES: Si la carga ya esta activa, no activar otra
	if (CurrentState != EPGXLoadingScreenState::Idle)
	{
		PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Loading already active — skipping auto-activation."));
		return;
	}

	// EN: Resolve context tag: check level CustomParams first, then config default
	// ES: Resolver tag de contexto: primero CustomParams del nivel, luego default de config
	FGameplayTag ContextTag;

	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UPGXLevelFlowSubsystem* LevelFlow = GI->GetSubsystem<UPGXLevelFlowSubsystem>();
		if (LevelFlow)
		{
			FPGXLevelEntry Entry;
			if (LevelFlow->ResolveLevelByTag(LevelTag, Entry))
			{
				const FString* LoadingContext = Entry.CustomParams.Find(FName("LoadingContext"));
				if (LoadingContext && !LoadingContext->IsEmpty())
				{
					ContextTag = FGameplayTag::RequestGameplayTag(FName(**LoadingContext), false);
					if (ContextTag.IsValid())
					{
						PGX_LOG_INFO(LogPGXLoading, TEXT("Level-specific loading context: %s"),
							*ContextTag.ToString());
					}
				}
			}
		}
	}

	// EN: Fallback to config default / ES: Fallback al default de config
	if (!ContextTag.IsValid())
	{
		ContextTag = LoadingConfig ? LoadingConfig->LevelFlowDefaultContext : FGameplayTag();
	}

	// EN: Final fallback to default tag / ES: Fallback final al tag default
	if (!ContextTag.IsValid())
	{
		ContextTag = TAG_PGX_Loading_Context_Default;
	}

	RequestLoading(ContextTag);
}

void UPGXLoadingSubsystem::OnLevelFlowCompleted(FGameplayTag LevelTag)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT("OnLevelFlowCompleted: %s (state=%s)"),
		*LevelTag.ToString(), *GetStateName(CurrentState));

	// EN: LevelFlow completion signals that the level load is done.
	//     If we're still in Active, this helps trigger WaitingClose.
	//     PostLoadMap is the primary trigger, but this provides redundancy.
	// ES: Completar LevelFlow senala que la carga de nivel termino.
	//     Si seguimos en Active, esto ayuda a activar WaitingClose.
	if (AcceptCompletionSignal())
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("LevelFlow completed — marking PostLoadMap as received."));
	}
}

void UPGXLoadingSubsystem::OnPSOBridgeState(FGameplayTag /*Channel*/, const FPGXBridgeLoadingState& Payload)
{
	bPSOBound = true;
	PSOProgressValue = FMath::Clamp(Payload.Progress, 0.0f, 1.0f);
	bPSOBridgeWarmUpActive = Payload.bIsLoading && PSOProgressValue < 1.0f;

	if (!bPSOBridgeWarmUpActive)
	{
		bPSOReady = true;
	}
}

void UPGXLoadingSubsystem::OnPSOBridgeProgress(FGameplayTag /*Channel*/, const FPGXBridgeLoadingState& Payload)
{
	bPSOBound = true;

	const float ClampedProgress = FMath::Clamp(Payload.Progress, 0.0f, 1.0f);
	PSOProgressValue = ClampedProgress;
	bPSOBridgeWarmUpActive = Payload.bIsLoading && ClampedProgress < 1.0f;
	if (!bPSOBridgeWarmUpActive)
	{
		bPSOReady = true;
	}

	if (CurrentState != EPGXLoadingScreenState::Idle)
	{
		OnPSOProgressUpdated(0, 0, ClampedProgress);
	}
}

void UPGXLoadingSubsystem::OnPSOBridgeComplete(FGameplayTag /*Channel*/, const FPGXBridgeLoadingState& Payload)
{
	bPSOBound = true;
	bPSOBridgeWarmUpActive = false;

	PSOProgressValue = FMath::Clamp(Payload.Progress, 0.0f, 1.0f);
	OnPSOCompleted();
}

void UPGXLoadingSubsystem::OnPSOProgressUpdated(int32 Completed, int32 Total, float Percent)
{
	if (CurrentState == EPGXLoadingScreenState::Idle) return;

	PSOProgressValue = Percent;

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("PSO progress: %d/%d (%.1f%%)"), Completed, Total, Percent * 100.0f);
}

void UPGXLoadingSubsystem::OnPSOCompleted()
{
	PGX_LOG_INFO(LogPGXLoading, TEXT("PSO warm-up completed (state=%s)."), *GetStateName(CurrentState));

	bPSOReady = true;
	PSOProgressValue = 1.0f;
	bPSOBridgeWarmUpActive = false;
	StopPSOTimeout();
}

void UPGXLoadingSubsystem::OnNetworkFailure(UWorld* /*World*/, UNetDriver* /*NetDriver*/, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	if (CurrentState == EPGXLoadingScreenState::Idle) return;

	PGX_LOG_WARNING(LogPGXLoading, TEXT("Network failure during loading (type=%d): %s — force closing."),
		static_cast<int32>(FailureType), *ErrorString);

	FailLoading(EPGXLoadingResultCode::ForceClosed,
		FString::Printf(TEXT("Network failure: %s"), *ErrorString));
}

void UPGXLoadingSubsystem::OnTravelFailure(UWorld* /*World*/, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	if (CurrentState == EPGXLoadingScreenState::Idle) return;

	PGX_LOG_WARNING(LogPGXLoading, TEXT("Travel failure during loading (type=%d): %s — force closing."),
		static_cast<int32>(FailureType), *ErrorString);

	FailLoading(EPGXLoadingResultCode::ForceClosed,
		FString::Printf(TEXT("Travel failure: %s"), *ErrorString));
}

void UPGXLoadingSubsystem::OnPostLoadMap(UWorld* World)
{
	if (!World) return;

	if (AcceptCompletionSignal())
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("PostLoadMap received during %s — enabling close evaluation."),
			*GetStateName(CurrentState));

	}
}

bool UPGXLoadingSubsystem::AcceptCompletionSignal()
{
	const bool bAcceptingState = CurrentState == EPGXLoadingScreenState::Preparing
		|| CurrentState == EPGXLoadingScreenState::FadingIn
		|| CurrentState == EPGXLoadingScreenState::Active
		|| CurrentState == EPGXLoadingScreenState::WaitingClose;
	if (!bAcceptingState || bPostLoadMapReceived)
	{
		return false;
	}
	bPostLoadMapReceived = true;
	PostLoadFrameCount = 0;
	return true;
}

// ============================================================================
// PSO Timeout
// ============================================================================

void UPGXLoadingSubsystem::StartPSOTimeout()
{
	StopPSOTimeout();

	float Timeout = LoadingConfig ? LoadingConfig->PSOWaitTimeout : 15.0f;

	PSOTimeoutHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(
			this, &ThisClass::OnPSOTimeoutTick),
		Timeout);

	PGX_LOG_INFO(LogPGXLoading, TEXT("PSO timeout started: %.1fs"), Timeout);
}

void UPGXLoadingSubsystem::StopPSOTimeout()
{
	if (PSOTimeoutHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PSOTimeoutHandle);
		PSOTimeoutHandle.Reset();
	}
}

bool UPGXLoadingSubsystem::OnPSOTimeoutTick(float /*DeltaTime*/)
{
	PGX_LOG_WARNING(LogPGXLoading, TEXT("PSO wait timeout — marking PSO as ready (forced)."));
	bPSOReady = true;
	PSOProgressValue = 1.0f;
	bPSOBridgeWarmUpActive = false;
	return false; // EN: One-shot / ES: Una sola vez
}

// ============================================================================
// Input Management
// ============================================================================

void UPGXLoadingSubsystem::CaptureInputState()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWorld* World = GI->GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	// EN: Capture cursor state / ES: Capturar estado del cursor
	InputSnapshot.bCursorVisible = PC->bShowMouseCursor;
	// EN: We approximate input mode since UE doesn't expose GetInputMode()
	// ES: Aproximamos el modo de input ya que UE no expone GetInputMode()
	InputSnapshot.bWasGameOnly = true;
	InputSnapshot.bWasUIOnly = false;
	InputSnapshot.bWasGameAndUI = false;
}

void UPGXLoadingSubsystem::BlockInput()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWorld* World = GI->GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	// EN: Switch to UI-only input mode to block gameplay input
	// ES: Cambiar a modo UIOnly para bloquear input de gameplay
	FInputModeUIOnly InputMode;
	PC->SetInputMode(InputMode);

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Input blocked (UI-only mode)."));
}

void UPGXLoadingSubsystem::RestoreInputState()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UWorld* World = GI->GetWorld();
	if (!World) return;

	// EN: Get controller dynamically — NEVER cached (SeamlessTravel safety)
	// ES: Obtener controller dinamicamente — NUNCA cacheado (seguridad SeamlessTravel)
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	// EN: Restore original input mode / ES: Restaurar modo de input original
	if (InputSnapshot.bWasUIOnly)
	{
		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
	}
	else if (InputSnapshot.bWasGameAndUI)
	{
		FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);
	}
	else
	{
		// EN: Default: game-only / ES: Por defecto: solo juego
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	PC->bShowMouseCursor = InputSnapshot.bCursorVisible;

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Input restored."));
}

// ============================================================================
// Watchdog
// ============================================================================

void UPGXLoadingSubsystem::StartWatchdog(float TimeoutSeconds)
{
	StopWatchdog();

	WatchdogHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(
			this, &ThisClass::OnWatchdogTick),
		TimeoutSeconds);
}

void UPGXLoadingSubsystem::StopWatchdog()
{
	if (WatchdogHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(WatchdogHandle);
		WatchdogHandle.Reset();
	}
}

bool UPGXLoadingSubsystem::OnWatchdogTick(float /*DeltaTime*/)
{
	PGX_LOG_WARNING(LogPGXLoading, TEXT("Watchdog triggered in state: %s"), *GetStateName(CurrentState));

	if (CurrentState == EPGXLoadingScreenState::Preparing)
	{
		// EN: Preparing timeout — deactivate current strategy, replace with Minimal, continue
		// ES: Timeout de preparacion — desactivar estrategia actual, reemplazar con Minimal, continuar
		PGX_LOG_WARNING(LogPGXLoading, TEXT("Preparing timeout — falling back to Minimal strategy."));
		if (ActiveStrategy)
		{
			ActiveStrategy->Deactivate();
		}
		ActiveStrategy = CreateStrategyForType(EPGXLoadingVisualType::Minimal);
		if (ActiveStrategy)
		{
			ActiveStrategy->InitializeStrategy(ResolvedProfile);
		}
		ActiveVisualType = EPGXLoadingVisualType::Minimal;
		StopEvaluationTicker();
		BeginFadeIn();
	}
	else if (CurrentState == EPGXLoadingScreenState::WaitingClose)
	{
		// EN: WaitingClose timeout — force close
		// ES: Timeout de WaitingClose — forzar cierre
		PGX_LOG_WARNING(LogPGXLoading, TEXT("WaitingClose timeout — forcing close."));
		BeginFadeOut();
	}

	return false; // EN: Don't repeat / ES: No repetir
}

// ============================================================================
// Evaluation Ticker
// ============================================================================

void UPGXLoadingSubsystem::StartEvaluationTicker()
{
	StopEvaluationTicker();

	// EN: Tick every 0.1s to evaluate close conditions
	// ES: Tick cada 0.1s para evaluar condiciones de cierre
	EvaluationTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(
			this, &ThisClass::OnEvaluationTick),
		0.1f);
}

void UPGXLoadingSubsystem::StopEvaluationTicker()
{
	if (EvaluationTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(EvaluationTickerHandle);
		EvaluationTickerHandle.Reset();
	}
}

bool UPGXLoadingSubsystem::OnEvaluationTick(float DeltaTime)
{
	// EN: While Preparing, check if strategy is ready / ES: En Preparing, verificar si estrategia esta lista
	if (CurrentState == EPGXLoadingScreenState::Preparing)
	{
		CheckStrategyReady();
	}

	// EN: While Active/WaitingClose, evaluate close conditions and update strategy
	// ES: En Active/WaitingClose, evaluar condiciones de cierre y actualizar estrategia
	if (CurrentState == EPGXLoadingScreenState::Active ||
		CurrentState == EPGXLoadingScreenState::WaitingClose)
	{
		EvaluateCloseConditions();

		if (ActiveStrategy)
		{
			ActiveStrategy->UpdateVisual(DeltaTime);
		}
	}

	// EN: Continue ticking while not Idle / ES: Continuar tick mientras no sea Idle
	return (CurrentState != EPGXLoadingScreenState::Idle);
}

// ============================================================================
// Fade Animation
// ============================================================================

FPGXFadeConfig UPGXLoadingSubsystem::ResolveFadeConfig() const
{
	// EN: Profile fade override takes priority, then config default, then hardcoded fallback
	// ES: Override de fade del perfil tiene prioridad, luego default del config, luego fallback
	if (ResolvedProfile)
	{
		return ResolvedProfile->FadeConfig;
	}
	if (LoadingConfig)
	{
		return LoadingConfig->DefaultFadeConfig;
	}
	return FPGXFadeConfig(); // EN: Defaults: FadeIn=0.3s, FadeOut=0.5s, no curve
}

float UPGXLoadingSubsystem::EvaluateFadeCurve(float Alpha) const
{
	// EN: If a FadeCurve is loaded, use it for non-linear interpolation
	// ES: Si hay una FadeCurve cargada, usarla para interpolacion no-lineal
	if (LoadedFadeCurve)
	{
		return LoadedFadeCurve->GetFloatValue(Alpha);
	}
	return Alpha; // EN: Linear fallback / ES: Fallback lineal
}

void UPGXLoadingSubsystem::StartFadeAnimation(bool bFadeIn)
{
	StopFadeAnimation();

	FPGXFadeConfig FadeConf = ResolveFadeConfig();

	FadeDuration = bFadeIn ? FadeConf.FadeInDuration : FadeConf.FadeOutDuration;
	FadeCurrentAlpha = bFadeIn ? 0.0f : 1.0f;
	FadeTargetAlpha = bFadeIn ? 1.0f : 0.0f;
	bFadeAnimating = true;

	// EN: If duration is effectively zero, complete instantly / ES: Si duracion es cero, completar al instante
	if (FadeDuration <= KINDA_SMALL_NUMBER)
	{
		FadeCurrentAlpha = FadeTargetAlpha;
		if (OverlayManager)
		{
			OverlayManager->SetOverlayOpacity(FadeTargetAlpha);
		}
		bFadeAnimating = false;

		if (bFadeIn)
		{
			ActivateOverlay();
		}
		else
		{
			CompleteLoading();
		}
		return;
	}

	// EN: Async load FadeCurve if set and not yet loaded / ES: Cargar curva si configurada y no cargada
	if (!FadeConf.FadeCurve.IsNull() && !LoadedFadeCurve)
	{
		UCurveFloat* Loaded = FadeConf.FadeCurve.Get();
		if (Loaded)
		{
			LoadedFadeCurve = Loaded;
		}
		else
		{
			// EN: Async load — animation starts and curve applies once loaded
			// ES: Carga async — animacion inicia y curva se aplica cuando se cargue
			FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
			FadeCurveLoadHandle = StreamableManager.RequestAsyncLoad(
				FadeConf.FadeCurve.ToSoftObjectPath(),
				FStreamableDelegate::CreateLambda([this, FadeConf]()
				{
					LoadedFadeCurve = FadeConf.FadeCurve.Get();
					if (LoadedFadeCurve)
					{
						PGX_LOG_VERBOSE(LogPGXLoading, TEXT("FadeCurve loaded: %s"), *LoadedFadeCurve->GetName());
					}
				}),
				FStreamableManager::AsyncLoadHighPriority
			);
		}
	}

	// EN: Set initial opacity / ES: Establecer opacidad inicial
	if (OverlayManager)
	{
		OverlayManager->SetOverlayOpacity(EvaluateFadeCurve(FadeCurrentAlpha));
	}

	// EN: Start per-frame ticker for smooth animation / ES: Iniciar ticker por frame para animacion suave
	FadeTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(
			this, &ThisClass::OnFadeTick),
		0.0f); // EN: Every frame / ES: Cada frame

	PGX_LOG_INFO(LogPGXLoading, TEXT("Fade animation started: %s over %.2fs (curve=%s)"),
		bFadeIn ? TEXT("IN") : TEXT("OUT"), FadeDuration,
		LoadedFadeCurve ? *LoadedFadeCurve->GetName() : TEXT("linear"));
}

void UPGXLoadingSubsystem::StopFadeAnimation()
{
	if (FadeTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FadeTickerHandle);
		FadeTickerHandle.Reset();
	}
	bFadeAnimating = false;
}

bool UPGXLoadingSubsystem::OnFadeTick(float DeltaTime)
{
	if (!bFadeAnimating || FadeDuration <= KINDA_SMALL_NUMBER)
	{
		bFadeAnimating = false;
		return false;
	}

	// EN: Advance alpha toward target / ES: Avanzar alpha hacia objetivo
	float Step = DeltaTime / FadeDuration;
	if (FadeTargetAlpha > FadeCurrentAlpha)
	{
		// EN: Fading in (0 → 1) / ES: Fade in (0 → 1)
		FadeCurrentAlpha = FMath::Min(FadeCurrentAlpha + Step, 1.0f);
	}
	else
	{
		// EN: Fading out (1 → 0) / ES: Fade out (1 → 0)
		FadeCurrentAlpha = FMath::Max(FadeCurrentAlpha - Step, 0.0f);
	}

	// EN: Apply curve-evaluated opacity / ES: Aplicar opacidad evaluada con curva
	float EvaluatedAlpha = EvaluateFadeCurve(FadeCurrentAlpha);
	if (OverlayManager)
	{
		OverlayManager->SetOverlayOpacity(EvaluatedAlpha);
	}

	// EN: Check if fade is complete / ES: Verificar si el fade esta completo
	bool bComplete = FMath::IsNearlyEqual(FadeCurrentAlpha, FadeTargetAlpha, KINDA_SMALL_NUMBER);
	if (bComplete)
	{
		// EN: Snap to exact target / ES: Ajustar al target exacto
		if (OverlayManager)
		{
			OverlayManager->SetOverlayOpacity(FadeTargetAlpha);
		}

		bFadeAnimating = false;

		if (FadeTargetAlpha >= 1.0f)
		{
			// EN: Fade in complete → proceed to Active / ES: Fade in completo → proceder a Active
			ActivateOverlay();
		}
		else
		{
			// EN: Fade out complete → finalize / ES: Fade out completo → finalizar
			CompleteLoading();
		}

		return false; // EN: Stop ticking / ES: Dejar de tickear
	}

	return true; // EN: Continue ticking / ES: Continuar tickeando
}

// ============================================================================
// Deferred Input Restore
// ============================================================================

void UPGXLoadingSubsystem::ScheduleDeferredInputRestore()
{
	// EN: Wait 1 frame after FadeOut complete before restoring input
	//     This prevents queued inputs from executing immediately on the new level
	// ES: Esperar 1 frame despues de FadeOut antes de restaurar input
	InputFlushHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(
			this, &ThisClass::OnInputFlushTick),
		0.0f); // EN: Next frame / ES: Siguiente frame
}

bool UPGXLoadingSubsystem::OnInputFlushTick(float /*DeltaTime*/)
{
	RestoreInputState();
	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Deferred input restore completed (1-frame flush)."));
	return false; // EN: One-shot / ES: Una sola vez
}

// ============================================================================
// Console Commands
// ============================================================================

void UPGXLoadingSubsystem::ExecuteConsoleCommand(const FString& CommandName, const TArray<FString>& Args, UWorld* World)
{
	if (CommandName == TEXT("pgx.loading.close"))
	{
		FPGXLoadingResult Result = ForceClose();
		PGX_LOG_INFO(LogPGXLoading, TEXT("ForceClose: %s — %s"),
			Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAIL"), *Result.Description);
		return;
	}
	if (CommandName == TEXT("pgx.loading.config"))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== PGX Loading Config ==="));
		if (LoadingConfig)
		{
			PGX_LOG_INFO(LogPGXLoading, TEXT("Config: %s"), *LoadingConfig->GetName());
			PGX_LOG_INFO(LogPGXLoading, TEXT("ClosePolicy: %d, ReentryPolicy: %d"),
				static_cast<int32>(LoadingConfig->DefaultClosePolicy),
				static_cast<int32>(LoadingConfig->ReentryPolicy));
			PGX_LOG_INFO(LogPGXLoading, TEXT("FadeIn: %.2fs, FadeOut: %.2fs"),
				LoadingConfig->DefaultFadeConfig.FadeInDuration,
				LoadingConfig->DefaultFadeConfig.FadeOutDuration);
			PGX_LOG_INFO(LogPGXLoading, TEXT("MinDisplayTime: %.1fs"), LoadingConfig->DefaultMinDisplayTime);
			PGX_LOG_INFO(LogPGXLoading, TEXT("PreparingTimeout: %.1fs, WaitingCloseTimeout: %.1fs"),
				LoadingConfig->PreparingTimeout, LoadingConfig->WaitingCloseTimeout);
			PGX_LOG_INFO(LogPGXLoading, TEXT("PSO: Wait=%d, Timeout=%.1fs, Weight=%.2f"),
				LoadingConfig->bWaitForPSOByDefault,
				LoadingConfig->PSOWaitTimeout, LoadingConfig->PSOProgressWeight);
			PGX_LOG_INFO(LogPGXLoading, TEXT("LevelFlow: AutoActivate=%d"),
				LoadingConfig->bAutoActivateOnLevelFlow);
		}
		else
		{
			PGX_LOG_INFO(LogPGXLoading, TEXT("No config found — using defaults."));
		}
		return;
	}
	if (CommandName == TEXT("pgx.loading.debug"))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== PGX Loading Debug ==="));
		PGX_LOG_INFO(LogPGXLoading, TEXT("State: %s | FadeAlpha: %.3f | FadeAnimating: %d"),
			*GetStateName(CurrentState), FadeCurrentAlpha, bFadeAnimating);
		PGX_LOG_INFO(LogPGXLoading, TEXT("FadeDuration: %.2fs | FadeTarget: %.1f"),
			FadeDuration, FadeTargetAlpha);
		PGX_LOG_INFO(LogPGXLoading, TEXT("FadeCurve: %s"),
			LoadedFadeCurve ? *LoadedFadeCurve->GetName() : TEXT("None (linear)"));
		PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy: %s | Ready: %d"),
			ActiveStrategy ? *ActiveStrategy->GetClass()->GetName() : TEXT("None"),
			ActiveStrategy ? ActiveStrategy->IsReady() : false);
		PGX_LOG_INFO(LogPGXLoading, TEXT("Profile: %s"),
			ResolvedProfile ? *ResolvedProfile->GetName() : TEXT("None"));
		PGX_LOG_INFO(LogPGXLoading, TEXT("Close: MinTime=%d PSO=%d(%.0f%%) PostFrames=%d PostMap=%d"),
			bMinTimeElapsed, bPSOReady, PSOProgressValue * 100.0f,
			bPostLoadFramesElapsed, bPostLoadMapReceived);
		PGX_LOG_INFO(LogPGXLoading, TEXT("PSO Bound: %d | LevelFlow Auto: %d"),
			bPSOBound,
			LoadingConfig ? LoadingConfig->bAutoActivateOnLevelFlow : true);
		if (ResolvedProfile)
		{
			PGX_LOG_INFO(LogPGXLoading, TEXT("Profile FadeIn: %.2fs | FadeOut: %.2fs | MinDisplay: %.1fs"),
				ResolvedProfile->FadeConfig.FadeInDuration,
				ResolvedProfile->FadeConfig.FadeOutDuration,
				ResolvedProfile->MinDisplayTime);
		}
		return;
	}
	if (CommandName == TEXT("pgx.loading.history"))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== PGX Loading History (%d records) ==="),
			LoadingHistory.Num());
		for (int32 i = LoadingHistory.Num() - 1; i >= 0 && i >= LoadingHistory.Num() - 10; --i)
		{
			const FPGXLoadingRecord& R = LoadingHistory[i];
			PGX_LOG_INFO(LogPGXLoading,
				TEXT("  [%d] %s | Total=%.2fs (Prep=%.2f Act=%.2f Wait=%.2f) | Code=%d PSO=%d Timeout=%d"),
				i, *R.ContextTag.ToString(), R.TotalDuration,
				R.PreparingDuration, R.ActiveDuration, R.WaitingDuration,
				static_cast<int32>(R.ResultCode), R.bPSOWaited, R.bTimedOut);
		}
		return;
	}
	if (CommandName == TEXT("pgx.loading.profiles"))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== PGX Loading Profiles ==="));
		PGX_LOG_INFO(LogPGXLoading, TEXT("Discovered: %d profiles, %d context mappings"),
			DiscoveredProfiles.Num(), ContextMap.Num());

		for (const UPGXLoadingProfile* Prof : DiscoveredProfiles)
		{
			if (!Prof) continue;
			PGX_LOG_INFO(LogPGXLoading, TEXT("  Profile: %s (VisualType=%d, Tags=%d, Tips=%d, Images=%d)"),
				*Prof->GetName(),
				static_cast<int32>(Prof->DefaultVisualType),
				Prof->ContextTags.Num(),
				Prof->Tips.Num(),
				Prof->BackgroundImages.Num());
		}

		PGX_LOG_INFO(LogPGXLoading, TEXT("--- Context Map ---"));
		for (const auto& Pair : ContextMap)
		{
			const UPGXLoadingProfile* Prof = Pair.Value.Get();
			PGX_LOG_INFO(LogPGXLoading, TEXT("  %s → %s"),
				*Pair.Key.ToString(),
				Prof ? *Prof->GetName() : TEXT("(unloaded)"));
		}
		return;
	}
	if (CommandName == TEXT("pgx.loading.request"))
	{
		FGameplayTag Tag = TAG_PGX_Loading_Context_Default;
		if (Args.Num() > 0)
		{
			Tag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
			if (!Tag.IsValid())
			{
				PGX_LOG_WARNING(LogPGXLoading, TEXT("Invalid tag: %s — using Default"), *Args[0]);
				Tag = TAG_PGX_Loading_Context_Default;
			}
		}
		FPGXLoadingResult Result = RequestLoading(Tag);
		PGX_LOG_INFO(LogPGXLoading, TEXT("RequestLoading: %s — %s"),
			Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAIL"), *Result.Description);
		return;
	}
	if (CommandName == TEXT("pgx.loading.simulate"))
	{
		// EN: Request loading with default context — MinTime will keep it visible
		// ES: Solicitar loading con contexto default — MinTime lo mantendra visible
		float Delay = 3.0f;
		if (Args.Num() > 0)
		{
			Delay = FCString::Atof(*Args[0]);
		}
		PGX_LOG_INFO(LogPGXLoading, TEXT("Simulating loading for %.1fs..."), Delay);
		RequestLoading(TAG_PGX_Loading_Context_Default);
		return;
	}
	if (CommandName == TEXT("pgx.loading.simulate.pso"))
	{
		if (CurrentState == EPGXLoadingScreenState::Idle)
		{
			PGX_LOG_INFO(LogPGXLoading, TEXT("No loading active — request loading first."));
			return;
		}
		float Delay = 5.0f;
		if (Args.Num() > 0)
		{
			Delay = FCString::Atof(*Args[0]);
		}
		bPSOReady = false;
		PSOProgressValue = 0.0f;
		StopPSOTimeout();

		// EN: Start a one-shot ticker that restores PSO ready after delay
		// ES: Iniciar ticker one-shot que restaura PSO listo despues del delay
		PSOTimeoutHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda([this](float)
			{
				PGX_LOG_INFO(LogPGXLoading, TEXT("Simulated PSO delay complete — PSO marked ready."));
				bPSOReady = true;
				PSOProgressValue = 1.0f;
				return false;
			}),
			Delay);

		PGX_LOG_INFO(LogPGXLoading, TEXT("Simulating PSO delay: %.1fs — bPSOReady=false"), Delay);
		return;
	}
	if (CommandName == TEXT("pgx.loading.skip"))
	{
		FPGXLoadingResult Result = RequestSkip();
		PGX_LOG_INFO(LogPGXLoading, TEXT("RequestSkip: %s — %s"),
			Result.bSuccess ? TEXT("SUCCESS") : TEXT("FAIL"), *Result.Description);
		return;
	}
	if (CommandName == TEXT("pgx.loading.status"))
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("=== PGX Loading Screen Status ==="));
		PGX_LOG_INFO(LogPGXLoading, TEXT("State: %s"), *GetStateName(CurrentState));
		PGX_LOG_INFO(LogPGXLoading, TEXT("Context: %s"),
			CurrentContextTag.IsValid() ? *CurrentContextTag.ToString() : TEXT("None"));
		PGX_LOG_INFO(LogPGXLoading, TEXT("VisualType: %d"), static_cast<int32>(ActiveVisualType));
		PGX_LOG_INFO(LogPGXLoading, TEXT("Elapsed: %.2fs"), GetElapsedTime());
		PGX_LOG_INFO(LogPGXLoading, TEXT("Close Conditions: MinTime=%d PSO=%d PostFrames=%d PostMap=%d"),
			bMinTimeElapsed, bPSOReady, bPostLoadFramesElapsed, bPostLoadMapReceived);
		PGX_LOG_INFO(LogPGXLoading, TEXT("Profiles: %d, Contexts: %d, History: %d"),
			DiscoveredProfiles.Num(), ContextMap.Num(), LoadingHistory.Num());
		PGX_LOG_INFO(LogPGXLoading, TEXT("Config: %s"),
			LoadingConfig ? *LoadingConfig->GetName() : TEXT("DEFAULT"));
		return;
	}
}


// ============================================================================
// Profile Integration / Integración de Profile
// ============================================================================

void UPGXLoadingSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& Profile)
{
	// EN: Read the platform-profile loading budget
	//     LoadingBudgets and ASSIGN them to subsystem members so the runtime
	//     gate enforces MinDuration through EvaluateCloseConditions. MaxConcurrent is
	//     retained as the configured concurrency budget for loaders that consume it.
	// ES: Leer LoadingBudgets del perfil de plataforma
	//     de plataforma y ASIGNARLOS a miembros del subsistema para que el gate
	//     runtime aplica MinDuration via EvaluateCloseConditions. MaxConcurrent
	//     queda como budget configurado para loaders que lo consuman.
	EnforcedMaxConcurrentAsyncLoads = 0;
	EnforcedMinLoadingScreenDuration = 0.0f;

	if (auto* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
	{
		if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
		{
			const auto& B = PlatformCfg->LoadingBudgets;
			EnforcedMaxConcurrentAsyncLoads = B.MaxConcurrentAsyncLoads;
			EnforcedMinLoadingScreenDuration = B.MinLoadingScreenDuration;
		}
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LoadingSubsystem] Profile constraints applied — MaxAsyncLoads=%d, MinDuration=%.1fs, RAM=%lld MB, VRAM=%lld MB"),
		EnforcedMaxConcurrentAsyncLoads, EnforcedMinLoadingScreenDuration,
		Profile.Budgets.RAM_MB, Profile.Budgets.VRAM_MB);
}

void UPGXLoadingSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}
