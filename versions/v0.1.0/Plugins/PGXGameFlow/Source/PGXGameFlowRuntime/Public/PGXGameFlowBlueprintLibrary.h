// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXGameFlowTypes.h"
#include "PGXGameFlowBlueprintLibrary.generated.h"

class UPGXGameFlowSubsystem;

/**
 * EN: Blueprint Function Library for the PGX GameFlow system.
 *     Provides static BP nodes for all GameFlow operations via WorldContext dispatch.
 *     Thin wrappers that resolve the subsystem from GameInstance and delegate calls.
 *
 * ES: Blueprint Function Library para el sistema PGX GameFlow.
 *     Provee nodos BP estaticos para todas las operaciones de GameFlow via dispatch WorldContext.
 *     Wrappers delgados que resuelven el subsistema desde GameInstance y delegan las llamadas.
 */
UCLASS()
class PGXGAMEFLOWRUNTIME_API UPGXGameFlowBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Set API
	// ========================================================================

	/** EN: Set the state of a channel / ES: Establecer el estado de un canal */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow", meta = (WorldContext = "WorldContextObject"))
	static FPGXFlowResult SetStateByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FGameplayTag FlowTag, UObject* Source = nullptr);

	/** EN: Validate then apply sequential batch / ES: Validar y luego aplicar lote secuencial */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Advanced", meta = (WorldContext = "WorldContextObject"))
	static FPGXFlowResult SetBatchSequentialStateByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, const TArray<FGameplayTag>& FlowTags, UObject* Source = nullptr);

	/** EN: Break container then validate and apply sequentially / ES: Romper contenedor, validar y aplicar secuencialmente */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Advanced", meta = (WorldContext = "WorldContextObject"))
	static FPGXFlowResult SetBatchStateByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, const FGameplayTagContainer& FlowTags, UObject* Source = nullptr);

	/** EN: Revert to previous state / ES: Revertir al estado anterior */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow", meta = (WorldContext = "WorldContextObject"))
	static FPGXFlowResult RevertToPreviousFlow(const UObject* WorldContextObject, EPGXFlowChannel Channel, UObject* Source = nullptr);

	// ========================================================================
	// Validation API
	// ========================================================================

	/** EN: Check if transition is valid (no mutation) / ES: Verificar si la transicion es valida (sin mutacion) */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Advanced", meta = (WorldContext = "WorldContextObject"))
	static FPGXFlowResult CanChangeByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FGameplayTag FlowTag);

	/** EN: Validate a full batch of transitions / ES: Validar un lote completo de transiciones */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Advanced", meta = (WorldContext = "WorldContextObject"))
	static FPGXFlowResult CanBatchChangeByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, const TArray<FGameplayTag>& FlowTags);

	/** EN: Check if tag matches current state / ES: Verificar si el tag coincide con el estado actual */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow", meta = (WorldContext = "WorldContextObject"))
	static bool IsCurrentFlowTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FGameplayTag FlowTag);

	/** EN: Check if revert is allowed / ES: Verificar si se permite revertir */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Advanced", meta = (WorldContext = "WorldContextObject"))
	static bool CheckCanRevert(const UObject* WorldContextObject, EPGXFlowChannel Channel);

	// ========================================================================
	// Query API
	// ========================================================================

	/** EN: Get current state tag / ES: Obtener tag de estado actual */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow", meta = (WorldContext = "WorldContextObject"))
	static FGameplayTag GetCurrentFlowTag(const UObject* WorldContextObject, EPGXFlowChannel Channel);

	/** EN: Get previous state tag / ES: Obtener tag de estado anterior */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static FGameplayTag GetLastFlowTag(const UObject* WorldContextObject, EPGXFlowChannel Channel);

	/** EN: Get transition history / ES: Obtener historial de transiciones */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXFlowHistoryEntry> GetChannelHistory(const UObject* WorldContextObject, EPGXFlowChannel Channel);

	/** EN: Get rule for a specific state tag / ES: Obtener regla para un tag de estado especifico */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static bool GetAllowedTransitionByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FGameplayTag FlowTag, FPGXFlowRule& OutRule);

	/** EN: Get rule for current state / ES: Obtener regla para estado actual */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static bool GetAllowedTransitionByCurrentFlowTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FPGXFlowRule& OutRule);

	// ========================================================================
	// Utility
	// ========================================================================

	/** EN: Get channel display name / ES: Obtener nombre de display del canal */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Query")
	static FString GetChannelName(EPGXFlowChannel Channel);

	/** EN: Check if subsystem is initialized / ES: Verificar si el subsistema esta inicializado */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static bool IsGameFlowInitialized(const UObject* WorldContextObject);

private:
	/** EN: Helper — resolve subsystem from WorldContext / ES: Helper — resolver subsistema desde WorldContext */
	static UPGXGameFlowSubsystem* GetSubsystem(const UObject* WorldContextObject);
};
