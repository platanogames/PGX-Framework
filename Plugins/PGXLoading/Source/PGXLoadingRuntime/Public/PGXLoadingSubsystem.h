// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Messages/PGXMessage.h"
#include "PGXLoadingTypes.h"
#include "PGXLoadingDelegates.h"
#include "Engine/StreamableManager.h"
#include "Containers/Ticker.h"
#include "PGXLoadingSubsystem.generated.h"

class UPGXLoadingConfig;
class UPGXLoadingProfile;
class UPGXViewportOverlayManager;
class UPGXLoadingStrategyBase;
class UPGXLoadingWidget;
class UPGXLevelFlowSubsystem;
class UCurveFloat;
class UNetDriver;
struct FPGXBridgeLoadingState;

/**
 * EN: Core Loading Screen subsystem — orchestrates persistent loading overlays that survive
 *     level transitions. Data-driven via Config DAs and Loading Profiles.
 *     6-state deterministic state machine with validated transitions.
 *
 *     Lives on GameInstance (survives OpenLevel). Coordinates with LevelFlow (optional),
 *     PSO warm-up (optional), and GameFlow (one-directional activation).
 *
 *     Pipeline: Idle → Preparing → FadingIn → Active → WaitingClose → FadingOut → Idle
 *
 *     Auto-discovers UPGXLoadingConfig and UPGXLoadingProfile DAs via AssetRegistry scan.
 *     Same discovery pattern as PGXSave, PGXGameFlow, PGXPSO, and PGXLevelFlow.
 *
 * ES: Subsistema core de Pantalla de Carga — orquesta overlays persistentes de carga que sobreviven
 *     transiciones de nivel. Data-driven via Config DAs y Loading Profiles.
 *     Maquina de estados determinista de 6 estados con transiciones validadas.
 *
 *     Vive en GameInstance (sobrevive OpenLevel). Coordina con LevelFlow (opcional),
 *     PSO warm-up (opcional), y GameFlow (activacion unidireccional).
 */
UCLASS(BlueprintType)
class PGXLOADINGRUNTIME_API UPGXLoadingSubsystem : public UPGXGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ========================================================================
	// EN: Dynamic Delegates (Blueprint-assignable)
	// ES: Delegados dinamicos (asignables en Blueprint)
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "PGX|Loading|Delegates")
	FOnPGXLoadingStarted OnLoadingStarted;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Loading|Delegates")
	FOnPGXLoadingProgressUpdated OnLoadingProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Loading|Delegates")
	FOnPGXLoadingCompleted OnLoadingCompleted;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Loading|Delegates")
	FOnPGXLoadingFailed OnLoadingFailed;

	UPROPERTY(BlueprintAssignable, Category = "PGX|Loading|Delegates")
	FOnPGXLoadingStateChanged OnLoadingStateChanged;

	// ========================================================================
	// EN: Native Delegates (C++ / Slate / Editor)
	// ES: Delegados nativos (C++ / Slate / Editor)
	// ========================================================================

	FOnPGXLoadingStartedNative OnLoadingStartedNative;
	FOnPGXLoadingProgressNative OnLoadingProgressNative;
	FOnPGXLoadingCompletedNative OnLoadingCompletedNative;
	FOnPGXLoadingStateChangedNative OnLoadingStateChangedNative;

	// ========================================================================
	// EN: Public API — Loading Control / ES: API Publica — Control
	// ========================================================================

	/** EN: Request loading screen by context tag / ES: Solicitar pantalla por tag */
	FPGXLoadingResult RequestLoading(FGameplayTag ContextTag);

	/** EN: Force close loading screen from any state / ES: Forzar cierre desde cualquier estado */
	FPGXLoadingResult ForceClose();

	/** EN: Request skip (validates conditions before closing) / ES: Solicitar skip */
	FPGXLoadingResult RequestSkip();

	// ========================================================================
	// EN: Public API — Query / ES: API Publica — Consulta
	// ========================================================================

	/** EN: Is any loading screen currently active? / ES: Hay alguna pantalla de carga activa? */
	bool IsLoadingActive() const { return CurrentState != EPGXLoadingScreenState::Idle; }

	/** EN: Get current state machine state / ES: Obtener estado actual */
	EPGXLoadingScreenState GetCurrentState() const { return CurrentState; }

	/** EN: Get the context tag of the active loading screen / ES: Obtener tag de contexto activo */
	FGameplayTag GetCurrentContext() const { return CurrentContextTag; }

	/** EN: Get current progress information / ES: Obtener informacion de progreso */
	FPGXLoadingProgress GetProgress() const { return CurrentProgress; }

	/** EN: Get elapsed time since loading started / ES: Obtener tiempo transcurrido */
	float GetElapsedTime() const;

	/** EN: Get loading history records / ES: Obtener historial de carga */
	TArray<FPGXLoadingRecord> GetLoadingHistory() const { return LoadingHistory; }

	/** EN: Get number of discovered profiles / ES: Obtener cantidad de perfiles descubiertos */
	int32 GetDiscoveredProfileCount() const { return DiscoveredProfiles.Num(); }

	/** EN: Check if a profile exists for the given context tag / ES: Verificar si existe perfil para el tag */
	bool IsProfileValid(FGameplayTag ContextTag) const;

	/** EN: Get the active visual type (Minimal if no strategy) / ES: Obtener tipo visual activo */
	EPGXLoadingVisualType GetActiveVisualType() const { return ActiveVisualType; }

	/** EN: Get all registered context tags / ES: Obtener todos los tags de contexto registrados */
	TArray<FGameplayTag> GetRegisteredContextTags() const;

	/** EN: Check if subsystem is initialized / ES: Verificar si el subsistema esta inicializado */
	UFUNCTION(BlueprintPure, Category = "PGX|Loading")
	bool IsInitialized() const { return bInitialized; }

#if WITH_DEV_AUTOMATION_TESTS
	float GetPSOProgressValueForTesting() const { return PSOProgressValue; }
	bool IsPSOReadyForTesting() const { return bPSOReady; }
	bool IsPSOBridgeBoundForTesting() const { return bPSOBound; }
	bool IsPSOBridgeWarmUpActiveForTesting() const { return bPSOBridgeWarmUpActive; }
#endif

	// ========================================================================
	// EN: Static Access / ES: Acceso estatico
	// ========================================================================

	static UPGXLoadingSubsystem* GetCachedInstance() { return CachedInstance.Get(); }

private:
	// ========================================================================
	// EN: Profile Integration / ES: Integracion con Profile
	// ========================================================================

	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);

	// ========================================================================
	// EN: Internal — Discovery / ES: Descubrimiento
	// ========================================================================

	void DiscoverConfigs();
	void DiscoverProfiles();
	void MergeContextMappings();

	// ========================================================================
	// EN: Internal — State Machine / ES: Maquina de estados
	// ========================================================================

	void SetState(EPGXLoadingScreenState NewState);
	bool CanTransitionTo(EPGXLoadingScreenState NewState) const;
	FString GetStateName(EPGXLoadingScreenState State) const;

	// ========================================================================
	// EN: Internal — Strategy / ES: Estrategia
	// ========================================================================

	UPGXLoadingStrategyBase* CreateStrategyForType(EPGXLoadingVisualType VisualType);
	void CheckStrategyReady();

	// ========================================================================
	// EN: Internal — Pipeline / ES: Pipeline
	// ========================================================================

	void BeginPreparing(FGameplayTag ContextTag);
	void BeginFadeIn();
	void ActivateOverlay();
	void BeginWaitingClose();
	void EvaluateCloseConditions();
	void BeginFadeOut();
	void CompleteLoading();
	void FailLoading(EPGXLoadingResultCode Code, const FString& Reason);
	void RecordHistory(EPGXLoadingResultCode Code);

	// ========================================================================
	// EN: Internal — Integration Binding / ES: Binding de integracion
	// ========================================================================

	void BindPSOMessageBridge();
	void UnbindPSOMessageBridge();
	void RequestPSOBridgeState();
	void BindLevelFlowSubsystem();
	void UnbindLevelFlowSubsystem();
	void BindNetworkFailureHandlers();
	void UnbindNetworkFailureHandlers();

	// ========================================================================
	// EN: Internal — Integration Listeners / ES: Listeners de integracion
	// ========================================================================

	void OnLevelFlowStarted(FGameplayTag LevelTag);
	void OnLevelFlowCompleted(FGameplayTag LevelTag);
	void OnPSOBridgeState(FGameplayTag Channel, const FPGXBridgeLoadingState& Payload);
	void OnPSOBridgeProgress(FGameplayTag Channel, const FPGXBridgeLoadingState& Payload);
	void OnPSOBridgeComplete(FGameplayTag Channel, const FPGXBridgeLoadingState& Payload);
	void OnPSOProgressUpdated(int32 Completed, int32 Total, float Percent);
	void OnPSOCompleted();
	void OnPostLoadMap(UWorld* World);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void OnTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

	// ========================================================================
	// EN: Internal — PSO Timeout / ES: Timeout PSO
	// ========================================================================

	void StartPSOTimeout();
	void StopPSOTimeout();
	bool OnPSOTimeoutTick(float DeltaTime);

	// ========================================================================
	// EN: Internal — Input Management / ES: Gestion de input
	// ========================================================================

	void CaptureInputState();
	void BlockInput();
	void RestoreInputState();

	// ========================================================================
	// EN: Internal — Watchdog / ES: Watchdog
	// ========================================================================

	void StartWatchdog(float TimeoutSeconds);
	void StopWatchdog();
	bool OnWatchdogTick(float DeltaTime);

	// ========================================================================
	// EN: Internal — Evaluation Ticker / ES: Ticker de evaluacion
	// ========================================================================

	void StartEvaluationTicker();
	void StopEvaluationTicker();
	bool OnEvaluationTick(float DeltaTime);

	// ========================================================================
	// EN: Internal — Fade Animation / ES: Animacion de Fade
	// ========================================================================

	void StartFadeAnimation(bool bFadeIn);
	void StopFadeAnimation();
	bool OnFadeTick(float DeltaTime);
	FPGXFadeConfig ResolveFadeConfig() const;
	float EvaluateFadeCurve(float Alpha) const;

	// ========================================================================
	// EN: Internal — Deferred Input Restore / ES: Restauracion diferida de input
	// ========================================================================

	void ScheduleDeferredInputRestore();
	bool OnInputFlushTick(float DeltaTime);

	// ========================================================================
	// EN: Internal — Console Commands / ES: Comandos de consola
	// ========================================================================

	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	// ========================================================================
	// EN: Internal — State / ES: Estado interno
	// ========================================================================

	// Config
	UPROPERTY()
	TObjectPtr<UPGXLoadingConfig> LoadingConfig;

	UPROPERTY()
	TObjectPtr<UPGXViewportOverlayManager> OverlayManager;

	UPROPERTY()
	TObjectPtr<UPGXLoadingStrategyBase> ActiveStrategy;

	// Discovered profiles
	UPROPERTY()
	TArray<TObjectPtr<UPGXLoadingProfile>> DiscoveredProfiles;

	// EN: Currently resolved profile for active loading / ES: Perfil resuelto actualmente
	UPROPERTY()
	TObjectPtr<UPGXLoadingProfile> ResolvedProfile;

	// State machine
	EPGXLoadingScreenState CurrentState = EPGXLoadingScreenState::Idle;
	FGameplayTag CurrentContextTag;
	FPGXLoadingProgress CurrentProgress;
	FPGXInputStateSnapshot InputSnapshot;
	EPGXLoadingVisualType ActiveVisualType = EPGXLoadingVisualType::Minimal;

	// Context resolution
	TMap<FGameplayTag, TSoftObjectPtr<UPGXLoadingProfile>> ContextMap;

	// History
	TArray<FPGXLoadingRecord> LoadingHistory;

	// Handles
	TSharedPtr<FStreamableHandle> ActiveLoadHandle;
	TSharedPtr<FStreamableHandle> FadeCurveLoadHandle;
	FTSTicker::FDelegateHandle WatchdogHandle;
	FTSTicker::FDelegateHandle EvaluationTickerHandle;
	FTSTicker::FDelegateHandle FadeTickerHandle;
	FTSTicker::FDelegateHandle InputFlushHandle;
	FTSTicker::FDelegateHandle PSOTimeoutHandle;
	FDelegateHandle PostLoadMapHandle;
	FDelegateHandle NetworkFailureHandle;
	FDelegateHandle TravelFailureHandle;
	TArray<FPGXMessageListenerHandle> PSOMessageHandles;

	// Timing
	double StateEntryTime = 0.0;
	double LoadingStartTime = 0.0;
	double PreparingStartTime = 0.0;
	double ActiveStartTime = 0.0;
	double WaitingStartTime = 0.0;
	double FadeStartTime = 0.0;
	int32 PostLoadFrameCount = 0;

	// Close conditions
	bool bPSOReady = true;
	bool bMinTimeElapsed = false;
	bool bPostLoadFramesElapsed = false;
	bool bPostLoadMapReceived = false;

	// PSO integration state
	float PSOProgressValue = 0.0f;
	bool bPSOBound = false;
	bool bPSOBridgeWarmUpActive = false;

	// EN: Platform-profile budget enforcement (active platform-profile contract).
	//     EnforcedMinLoadingScreenDuration acts as a FLOOR on the
	//     EvaluateCloseConditions MinTime so platforms that need a longer
	//     guaranteed display (consoles, certification requirements) cannot have
	//     their floor undercut by Profile or LoadingConfig overrides.
	//     EnforcedMaxConcurrentAsyncLoads is stored for the future async-loader
	//     implementation; it is consumed at the gate when real async loading lands.
	//     Both default 0 = "no platform constraint applied".
	// ES: Enforcement de budgets definido por el perfil de plataforma. EnforcedMinLoadingScreenDuration actua como un FLOOR
	//     sobre el MinTime de EvaluateCloseConditions para que plataformas que
	//     necesitan un display garantizado mas largo (consolas, requisitos de
	//     certificacion) no puedan ver su floor undercut por overrides de Profile
	//     o LoadingConfig. EnforcedMaxConcurrentAsyncLoads se almacena para la
	//     futura implementacion del async-loader; se consume en el gate cuando el
	//     async loading real aterrice. Ambos default 0 = "sin restriccion de
	//     plataforma aplicada".
	float EnforcedMinLoadingScreenDuration = 0.0f;
	int32 EnforcedMaxConcurrentAsyncLoads = 0;

	// Fade animation state
	float FadeCurrentAlpha = 0.0f;
	float FadeTargetAlpha = 0.0f;
	float FadeDuration = 0.0f;
	bool bFadeAnimating = false;

	UPROPERTY()
	TObjectPtr<UCurveFloat> LoadedFadeCurve;

	// Flags
	bool bInitialized = false;

	// Console
	TArray<IConsoleObject*> RegisteredCommands;

	// Cached instance
	static TWeakObjectPtr<UPGXLoadingSubsystem> CachedInstance;
};
