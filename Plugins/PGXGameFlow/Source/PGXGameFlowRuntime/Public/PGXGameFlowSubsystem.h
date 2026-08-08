// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Messages/PGXMessage.h"
#include "PGXGameFlowTypes.h"
#include "PGXGameFlowDelegates.h"
#include "PGXGameFlowSubsystem.generated.h"

class UPGXGameFlowConfig;
class UPGXFlowRulesConfig;

/**
 * EN: Core GameFlow subsystem — manages 8 independent FSM channels driven by GameplayTags.
 *     Provides data-driven transition rules, two-layer validation (Allowed + Disallowed),
 *     history tracking, and delegate broadcasting for both Blueprint and C++ listeners.
 *
 * ES: Subsistema central de GameFlow — gestiona 8 canales FSM independientes manejados por GameplayTags.
 *     Proporciona reglas de transicion data-driven, validacion de dos capas (Allowed + Disallowed),
 *     tracking de historial, y broadcasting de delegados para listeners Blueprint y C++.
 */
UCLASS(BlueprintType)
class PGXGAMEFLOWRUNTIME_API UPGXGameFlowSubsystem : public UPGXGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ========================================================================
	// Dynamic Delegates (Blueprint-assignable, one per channel)
	// EN: BP users bind selectively to only their channel → zero noise
	// ES: Usuarios BP enlazan selectivamente solo su canal → cero ruido
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "PGX|GameFlow|Delegates")
	FOnPGXFlowGlobalChanged OnFlowGlobalChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|GameFlow|Delegates")
	FOnPGXFlowUIChanged OnFlowUIChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|GameFlow|Delegates")
	FOnPGXFlowCharactersChanged OnFlowCharactersChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|GameFlow|Delegates")
	FOnPGXFlowAIChanged OnFlowAIChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|GameFlow|Delegates")
	FOnPGXFlowCamerasChanged OnFlowCamerasChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|GameFlow|Delegates")
	FOnPGXFlowSystemsChanged OnFlowSystemsChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|GameFlow|Delegates")
	FOnPGXFlowLevelLogicChanged OnFlowLevelLogicChanged;

	UPROPERTY(BlueprintAssignable, Category = "PGX|GameFlow|Delegates")
	FOnPGXFlowActorsChanged OnFlowActorsChanged;

	// ========================================================================
	// Native Delegate (C++ only — single generic with channel param)
	// EN: C++ listeners get all channels via one delegate and filter internally
	// ES: Listeners C++ reciben todos los canales via un delegado y filtran internamente
	// ========================================================================

	FOnPGXFlowStateChangedNative OnFlowStateChangedNative;

	// ========================================================================
	// Set API — State Mutation
	// ========================================================================

	/**
	 * EN: Set the state of a channel. Validates, applies, and broadcasts.
	 * ES: Establece el estado de un canal. Valida, aplica y hace broadcast.
	 */
	FPGXFlowResult SetStateByTag(EPGXFlowChannel Channel, FGameplayTag FlowTag, UObject* Source = nullptr);

	/**
	 * EN: Validate ALL tags first, then apply sequentially. Atomic validation.
	 * ES: Validar TODOS los tags primero, luego aplicar secuencialmente. Validacion atomica.
	 */
	FPGXFlowResult SetBatchSequentialStateByTag(EPGXFlowChannel Channel, const TArray<FGameplayTag>& FlowTags, UObject* Source = nullptr);

	/**
	 * EN: Break tag container into array, validate ALL, then apply sequentially.
	 * ES: Romper contenedor de tags en array, validar TODOS, luego aplicar secuencialmente.
	 */
	FPGXFlowResult SetBatchStateByTag(EPGXFlowChannel Channel, const FGameplayTagContainer& FlowTags, UObject* Source = nullptr);

	/**
	 * EN: Revert channel to its previous state (if bAllowRevert is true in current rule).
	 * ES: Revertir canal a su estado anterior (si bAllowRevert es true en la regla actual).
	 */
	FPGXFlowResult RevertToPreviousFlow(EPGXFlowChannel Channel, UObject* Source = nullptr);

	// ========================================================================
	// Validation API — Query without mutation
	// ========================================================================

	/**
	 * EN: Check if a transition is valid for the given channel (does NOT apply it).
	 * ES: Verifica si una transicion es valida para el canal dado (NO la aplica).
	 */
	FPGXFlowResult CanChangeByTag(EPGXFlowChannel Channel, FGameplayTag FlowTag) const;

	/**
	 * EN: Validate an entire batch of transitions (simulated sequential).
	 * ES: Validar un lote completo de transiciones (simulado secuencial).
	 */
	FPGXFlowResult CanBatchChangeByTag(EPGXFlowChannel Channel, const TArray<FGameplayTag>& FlowTags) const;

	/**
	 * EN: Check if the given tag matches the channel's current state.
	 * ES: Verifica si el tag dado coincide con el estado actual del canal.
	 */
	bool IsCurrentFlowTag(EPGXFlowChannel Channel, FGameplayTag FlowTag) const;

	/**
	 * EN: Check if the current state's rule allows revert.
	 * ES: Verifica si la regla del estado actual permite revertir.
	 */
	bool CheckCanRevert(EPGXFlowChannel Channel) const;

	// ========================================================================
	// Query API — Data Access
	// ========================================================================

	/** EN: Get current state tag for a channel / ES: Obtener tag de estado actual de un canal */
	FGameplayTag GetCurrentFlowTag(EPGXFlowChannel Channel) const;

	/** EN: Get previous state tag for a channel / ES: Obtener tag de estado anterior de un canal */
	FGameplayTag GetLastFlowTag(EPGXFlowChannel Channel) const;

	/** EN: Get the transition history for a channel / ES: Obtener el historial de transiciones de un canal */
	TArray<FPGXFlowHistoryEntry> GetChannelHistory(EPGXFlowChannel Channel) const;

	/**
	 * EN: Get the rule and allowed/disallowed destinations for a specific state tag.
	 * ES: Obtener la regla y destinos permitidos/prohibidos para un tag de estado especifico.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow")
	bool GetAllowedTransitionByTag(EPGXFlowChannel Channel, FGameplayTag FlowTag,
		FPGXFlowRule& OutRule) const;

	/**
	 * EN: Get the rule for the channel's CURRENT state.
	 * ES: Obtener la regla para el estado ACTUAL del canal.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow")
	bool GetAllowedTransitionByCurrentFlowTag(EPGXFlowChannel Channel,
		FPGXFlowRule& OutRule) const;

	// ========================================================================
	// Utility
	// ========================================================================

	/** EN: Get channel display name / ES: Obtener nombre de display del canal */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow")
	static FString GetChannelName(EPGXFlowChannel Channel);

	/** EN: Test-visible branch matcher; exact match or destination descendant of branch. */
	static bool DoesTagMatchBranchForTesting(const FGameplayTag& Destination, const FGameplayTag& Branch);

	/** EN: Check if subsystem is initialized / ES: Verifica si el subsistema esta inicializado */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow")
	bool IsInitialized() const { return bIsInitialized; }

#if WITH_DEV_AUTOMATION_TESTS
	/** EN: Test harness only — inject transient config without touching project assets. */
	void InjectTestConfig(UPGXGameFlowConfig* TestConfig);

	/** EN: Test harness only — inject transient per-channel rules configs. */
	void InjectTestRulesConfigs(const TArray<UPGXFlowRulesConfig*>& TestRulesConfigs);

	/** EN: Test harness only — rebuild rule cache after injected settings change. */
	void RebuildRulesCacheForTesting();

	/** EN: Test harness only — reset all channel runtime state. */
	void ResetChannelStatesForTesting();

	/** EN: Test harness only — inspect selected rules config. */
	const UPGXFlowRulesConfig* GetRulesConfigForTesting(EPGXFlowChannel Channel) const;

	/** EN: Test harness only — inspect resolved runtime history budget. */
	int32 GetResolvedMaxHistoryDepthForTesting() const;

	/** EN: Test harness only — inspect console mutation gate. */
	bool IsConsoleMutationAllowedForTesting() const;

	/** EN: Test harness only — force a resolved history clamp and trim existing history. */
	void OverrideResolvedMaxHistoryDepthForTesting(int32 MaxHistoryDepth);
#endif

private:
	// ========================================================================
	// Profile Integration
	// ========================================================================

	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);

	// ========================================================================
	// Internal — Discovery & Cache
	// ========================================================================

	void DiscoverConfigs();
	void BuildRulesCache();
	void ApplyInitialStates();
	void ResolveRuntimeBudgets();
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	// ========================================================================
	// Internal — Validation Engine
	// ========================================================================

	/**
	 * EN: String-based branch check. Returns true if Destination equals Rule OR is a descendant.
	 *     Uses string comparison (NOT MatchesTag) — works for ALL tags regardless of registration.
	 * ES: Verificacion de rama basada en strings. Retorna true si Destination es igual a Rule O es descendiente.
	 *     Usa comparacion de strings (NO MatchesTag) — funciona para TODOS los tags sin importar registro.
	 */
	static bool IsInBranch(const FGameplayTag& Destination, const FGameplayTag& Rule);

	const FPGXFlowRule* FindRuleForCurrentState(EPGXFlowChannel Channel) const;
	const FPGXFlowRule* FindRuleForTag(EPGXFlowChannel Channel, const FGameplayTag& Tag) const;

	static bool RunAllowedCheck(const FGameplayTag& Destination, const TArray<FGameplayTag>& AllowedDestinations);
	static bool RunDisallowedCheck(const FGameplayTag& Destination, const TArray<FGameplayTag>& DisallowedTagQueries);

	FPGXFlowResult ValidateTransitionFromState(EPGXFlowChannel Channel, const FGameplayTag& CurrentState, const FGameplayTag& Destination) const;
	FPGXFlowResult ValidateTransition(EPGXFlowChannel Channel, const FGameplayTag& Destination) const;

	// ========================================================================
	// Internal — State Mutation
	// ========================================================================

	void ApplyTransition(EPGXFlowChannel Channel, const FGameplayTag& NewTag);
	void BroadcastDelegates(EPGXFlowChannel Channel, const FGameplayTag& OldTag, const FGameplayTag& FlowTag, UObject* Source);
	void PublishBridgeMessage(EPGXFlowChannel Channel, const FGameplayTag& OldTag, const FGameplayTag& NewTag, UObject* Source);
	void RegisterLoadingBridgeListeners();
	void UnregisterLoadingBridgeListeners();
	void HandleLoadingSetStateMessage(FGameplayTag Channel, const FPGXMessage& Message);
	void HandleLoadingRevertMessage(FGameplayTag Channel, const FPGXMessage& Message);

	// ========================================================================
	// Internal — Helpers
	// ========================================================================

	bool IsValidChannel(EPGXFlowChannel Channel) const
	{
		return static_cast<int32>(Channel) >= 0 && static_cast<int32>(Channel) < PGX_FLOW_CHANNEL_COUNT;
	}

	int32 GetResolvedMaxHistoryDepth() const;
	bool IsConsoleMutationAllowed() const;

	// ========================================================================
	// Internal — State
	// ========================================================================

	FPGXFlowChannelState ChannelStates[PGX_FLOW_CHANNEL_COUNT];

	UPROPERTY()
	TMap<EPGXFlowChannel, TObjectPtr<UPGXFlowRulesConfig>> ChannelRulesMap;

	UPROPERTY()
	TObjectPtr<UPGXGameFlowConfig> ActiveConfig;

	UPROPERTY()
	TArray<TObjectPtr<UPGXGameFlowConfig>> DiscoveredConfigs;

	UPROPERTY()
	TArray<TObjectPtr<UPGXFlowRulesConfig>> DiscoveredRulesConfigs;

	TArray<IConsoleObject*> RegisteredCommands;

	FPGXMessageListenerHandle LoadingSetStateListenerHandle;
	FPGXMessageListenerHandle LoadingRevertListenerHandle;

	bool bIsInitialized = false;
	int32 ResolvedMaxHistoryDepth = 0;

	static TWeakObjectPtr<UPGXGameFlowSubsystem> CachedInstance;

public:
	static UPGXGameFlowSubsystem* GetCachedInstance() { return CachedInstance.Get(); }
};
