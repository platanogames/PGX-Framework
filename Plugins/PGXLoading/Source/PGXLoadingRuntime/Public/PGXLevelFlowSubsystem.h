// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Interfaces/PGXTaggedRegistry.h"
#include "PGXLevelFlowTypes.h"
#include "PGXLevelFlowDelegates.h"
#include "Engine/StreamableManager.h"
#include "Containers/Ticker.h"
#include "PGXLevelFlowSubsystem.generated.h"

class UPGXLevelFlowConfig;
class UPGXLevelProfile;
class APGXLevelFlowActor;
class ULevelStreamingDynamic;

/**
 * EN: Core LevelFlow subsystem — orchestrates level transitions with data-driven profiles,
 *     deterministic timing (condition + timeout), and automatic GameFlow integration.
 *     Lives on GameInstance (survives level transitions).
 *
 *     Pipeline: Idle → Preparing → Loading → Transitioning → PostLoad → Complete
 *
 *     Auto-discovers UPGXLevelProfile and UPGXLevelFlowConfig DAs via AssetRegistry scan.
 *     Same discovery pattern as PGXSave, PGXGameFlow, and PGXPSO.
 *
 * ES: Subsistema central de LevelFlow — orquesta transiciones de nivel con perfiles data-driven,
 *     temporalizacion determinista (condicion + timeout), e integracion automatica con GameFlow.
 *     Vive en GameInstance (sobrevive transiciones de nivel).
 *
 *     Pipeline: Idle → Preparing → Loading → Transitioning → PostLoad → Complete
 *
 *     Auto-descubre UPGXLevelProfile y UPGXLevelFlowConfig DAs via AssetRegistry scan.
 *     Mismo patron de descubrimiento que PGXSave, PGXGameFlow y PGXPSO.
 */
UCLASS(BlueprintType)
class PGXLOADINGRUNTIME_API UPGXLevelFlowSubsystem : public UPGXGameInstanceSubsystem, public IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ========================================================================
	// Dynamic Delegates (Blueprint-assignable)
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "PGX|LevelFlow|Delegates")
	FOnPGXTransitionStarted OnTransitionStarted;

	UPROPERTY(BlueprintAssignable, Category = "PGX|LevelFlow|Delegates")
	FOnPGXTransitionProgress OnTransitionProgress;

	UPROPERTY(BlueprintAssignable, Category = "PGX|LevelFlow|Delegates")
	FOnPGXTransitionCompleted OnTransitionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "PGX|LevelFlow|Delegates")
	FOnPGXTransitionFailed OnTransitionFailed;

	UPROPERTY(BlueprintAssignable, Category = "PGX|LevelFlow|Delegates")
	FOnPGXSubLevelLoaded OnSubLevelLoadedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "PGX|LevelFlow|Delegates")
	FOnPGXSubLevelUnloaded OnSubLevelUnloadedDelegate;

	// ========================================================================
	// Native Delegates (C++ / Slate / Editor)
	// ========================================================================

	FOnPGXTransitionStartedNative OnTransitionStartedNative;
	FOnPGXTransitionCompletedNative OnTransitionCompletedNative;
	FOnPGXLevelFlowStateChangedNative OnLevelFlowStateChangedNative;

	// ========================================================================
	// Transition API — State Mutation
	// ========================================================================

	/**
	 * EN: Request a level transition by tag — the primary entry point.
	 *     Validates, resolves the tag to a level entry, and starts the async pipeline.
	 * ES: Solicita una transicion de nivel por tag — el punto de entrada principal.
	 *     Valida, resuelve el tag a una entrada de nivel, e inicia el pipeline async.
	 */
	FPGXLevelFlowResult RequestLevelTransition(FGameplayTag LevelTag, UObject* Source = nullptr);

	/**
	 * EN: Cancel an active transition. Works in Preparing, Loading, and PostLoad states.
	 *     During Transitioning (OpenLevel called), cancellation is not possible.
	 * ES: Cancela una transicion activa. Funciona en estados Preparing, Loading, y PostLoad.
	 *     Durante Transitioning (OpenLevel llamado), la cancelacion no es posible.
	 */
	FPGXLevelFlowResult CancelTransition();

	// ========================================================================
	// Query API
	// ========================================================================

	/** EN: Current pipeline state / ES: Estado actual del pipeline */
	EPGXLevelFlowState GetTransitionState() const { return CurrentState; }

	/** EN: Tag of the level we're currently in / ES: Tag del nivel en el que estamos */
	FGameplayTag GetCurrentLevelTag() const { return CurrentLevelTag; }

	/** EN: Tag of the previous level / ES: Tag del nivel anterior */
	FGameplayTag GetPreviousLevelTag() const { return PreviousLevelTag; }

	/** EN: Resolve a tag to its level data / ES: Resolver un tag a sus datos de nivel */
	bool ResolveLevelByTag(FGameplayTag LevelTag, FPGXLevelEntry& OutEntry) const;

	/** EN: Get all registered level tags from all discovered profiles / ES: Obtener todos los tags de nivel registrados */
	TArray<FGameplayTag> GetRegisteredLevelTags() const;

	// ========================================================================
	// EN: IPGXTaggedRegistry adoption — MergedLevelCatalog facade
	// ES: Adopcion IPGXTaggedRegistry — fachada de MergedLevelCatalog
	// ========================================================================

	/** EN: True if MergedLevelCatalog contains the tag / ES: True si MergedLevelCatalog contiene el tag. */
	bool HasEntryByTag(FGameplayTag Tag) const override;

	/** EN: Number of registered levels in the merged catalog / ES: Numero de niveles registrados en el catalogo merged. */
	int32 GetCount() const override;

	/** EN: Snapshot of registered level tags from the merged catalog / ES: Snapshot de tags de nivel registrados del catalogo merged. */
	void GetSnapshot(TArray<FGameplayTag>& OutTags) const override;

	/** EN: Get transition history / ES: Obtener historial de transiciones */
	TArray<FPGXLevelTransitionRecord> GetTransitionHistory() const { return TransitionHistory; }

	/** EN: Check if a transition is currently active / ES: Verificar si hay una transicion activa */
	bool IsTransitionActive() const { return CurrentState != EPGXLevelFlowState::Idle; }

	/** EN: Get current transition progress (0.0 - 1.0) / ES: Obtener progreso de transicion actual */
	float GetTransitionProgress() const { return TransitionProgressValue; }

	/** EN: Check if subsystem is initialized / ES: Verificar si el subsistema esta inicializado */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow")
	bool IsInitialized() const { return bIsInitialized; }

	/** EN: Get discovered profile count / ES: Obtener cantidad de perfiles descubiertos */
	int32 GetDiscoveredProfileCount() const { return DiscoveredProfiles.Num(); }

	/** EN: Get total registered level count / ES: Obtener cantidad total de niveles registrados */
	int32 GetRegisteredLevelCount() const { return MergedLevelCatalog.Num(); }

	// ========================================================================
	// Sub-Level API
	// ========================================================================

	/** EN: Load a sub-level by tag within the current level / ES: Cargar un sub-nivel por tag */
	FPGXLevelFlowResult RequestSubLevelLoad(FGameplayTag SubLevelTag);

	/** EN: Unload a sub-level by tag / ES: Descargar un sub-nivel por tag */
	FPGXLevelFlowResult RequestSubLevelUnload(FGameplayTag SubLevelTag);

	/** EN: Check if a sub-level is currently loaded / ES: Verificar si un sub-nivel esta cargado */
	bool IsSubLevelLoaded(FGameplayTag SubLevelTag) const;

	/** EN: Get all currently loaded sub-levels / ES: Obtener todos los sub-niveles cargados */
	TArray<FGameplayTag> GetLoadedSubLevels() const;


	// ========================================================================
	// Actor Registration (called by APGXLevelFlowActor)
	// ========================================================================

	void RegisterLevelFlowActor(APGXLevelFlowActor* Actor);
	void UnregisterLevelFlowActor(APGXLevelFlowActor* Actor);

	/** EN: Get the active actor for the current level (may be null) / ES: Obtener el actor activo del nivel actual */
	APGXLevelFlowActor* GetCurrentLevelFlowActor() const;

private:
	// ========================================================================
	// Profile Integration
	// ========================================================================

	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);

	// ========================================================================
	// Internal — Discovery & Catalog
	// ========================================================================

	void DiscoverConfigs();
	void MergeLevelCatalogs();

	// ========================================================================
	// Internal — State Machine
	// ========================================================================

	void SetTransitionState(EPGXLevelFlowState NewState);

	// ========================================================================
	// Internal — Transition Pipeline
	// ========================================================================

	void StartAsyncLoad();
	void OnAsyncLoadComplete();
	void ExecuteOpenLevel();
	void OnPostLoadMap(UWorld* NewWorld);
	void StartPostLoadTiming();
	bool OnTimingTick(float DeltaTime);
	void CompleteTransition(bool bTimedOut);
	void FailTransition(EPGXLevelFlowResultCode Code, const FString& Reason);

	// ========================================================================
	// Internal — GameFlow Integration
	// ========================================================================

	void SetGameFlowLoading();
	void SetGameFlowOnEnter();
	void RevertGameFlow();
	void PublishGameFlowSetState(FGameplayTag TargetStateTag);

	// ========================================================================
	// Internal — Helpers
	// ========================================================================

	FString ResolveLevelPath(const FPGXLevelEntry& Entry) const;
	const FPGXTransitionTiming& GetActiveTiming() const;
	void BroadcastProgressUpdate();
	void RecordTransition(bool bTimedOut, EPGXLevelFlowResultCode Code);

	// ========================================================================
	// Internal — Console Commands
	// ========================================================================

	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	// ========================================================================
	// Internal — State
	// ========================================================================

	// Pipeline state
	EPGXLevelFlowState CurrentState = EPGXLevelFlowState::Idle;
	FGameplayTag CurrentLevelTag;
	FGameplayTag PreviousLevelTag;
	float TransitionProgressValue = 0.0f;

	// Active transition context
	FPGXLevelEntry ActiveTransitionEntry;
	FGameplayTag ActiveTransitionTargetTag;
	FGameplayTag ActiveTransitionFromTag;
	double TransitionStartTime = 0.0;
	TSharedPtr<FStreamableHandle> ActiveStreamableHandle;
	FTSTicker::FDelegateHandle TimingTickerHandle;
	float PostLoadElapsed = 0.0f;
	int32 ShadersOnEntry = 0;

	// Discovered data
	TMap<FGameplayTag, FPGXLevelEntry> MergedLevelCatalog;

	UPROPERTY()
	TArray<TObjectPtr<UPGXLevelProfile>> DiscoveredProfiles;

	UPROPERTY()
	TObjectPtr<UPGXLevelFlowConfig> ActiveConfig;

	// History
	TArray<FPGXLevelTransitionRecord> TransitionHistory;

	// Actor
	TWeakObjectPtr<APGXLevelFlowActor> CurrentLevelFlowActorRef;

	// Sub-levels: Tag → streaming level handle for real unload
	TMap<FGameplayTag, TWeakObjectPtr<ULevelStreamingDynamic>> LoadedSubLevels;

	// EN: Platform-profile budget enforcement (active platform-profile contract).
	//     EnforcedStreamingPool_MB applies r.Streaming.PoolSize CVar when > 0
	//     so platform LoadingBudgets streaming caps actually take effect (prior
	//     impl logged the value and dropped it). EnforcedMaxLoadedSubLevels
	//     gates RequestSubLevel: when > 0 and LoadedSubLevels.Num() reaches the
	//     cap, the call returns LoadFailed with an explicit budget reason
	//     instead of silently exceeding the platform limit. Both default 0 =
	//     "no platform constraint applied".
	// ES: Enforcement de budgets definido por el perfil de plataforma. EnforcedStreamingPool_MB aplica el CVar
	//     r.Streaming.PoolSize cuando > 0 para que los caps de streaming de
	//     LoadingBudgets de plataforma realmente tengan efecto (la impl previa
	//     logueaba el valor y lo descartaba). EnforcedMaxLoadedSubLevels gatea
	//     RequestSubLevel: cuando > 0 y LoadedSubLevels.Num() alcanza el cap,
	//     la llamada retorna LoadFailed con razon explicita de budget en lugar
	//     de exceder silenciosamente el limite de plataforma. Ambos default 0
	//     = "sin restriccion de plataforma aplicada".
	int32 EnforcedStreamingPool_MB = 0;
	int32 EnforcedMaxLoadedSubLevels = 0;

	// Console
	TArray<IConsoleObject*> RegisteredCommands;

	// World transition detection
	FDelegateHandle PostLoadMapDelegateHandle;

	// Flags
	bool bIsInitialized = false;

	// Cached instance
	static TWeakObjectPtr<UPGXLevelFlowSubsystem> CachedInstance;

public:
	static UPGXLevelFlowSubsystem* GetCachedInstance() { return CachedInstance.Get(); }
};
