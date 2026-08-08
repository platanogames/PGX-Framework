// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXLevelFlowTypes.h"
#include "PGXLevelFlowBlueprintLibrary.generated.h"

class UPGXLevelFlowSubsystem;
class APGXLevelFlowActor;

/**
 * EN: Blueprint Function Library for the PGX LevelFlow system.
 *     Exposes level transition control, query, sub-level management,
 *     and actor access via static WorldContext-dispatch nodes.
 *
 * ES: Blueprint Function Library para el sistema PGX LevelFlow.
 *     Expone control de transiciones de nivel, consultas, gestion de sub-niveles,
 *     y acceso al actor via nodos estaticos con WorldContext-dispatch.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXLevelFlowBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Transition Control (PGX|LevelFlow)
	// ES: Control de Transiciones (PGX|LevelFlow)
	// ========================================================================

	/** EN: Request a level transition by tag / ES: Solicitar transicion de nivel por tag */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow", meta = (WorldContext = "WorldContextObject"))
	static FPGXLevelFlowResult RequestLevelTransition(const UObject* WorldContextObject, FGameplayTag LevelTag, UObject* Source = nullptr);

	/** EN: Cancel an active transition / ES: Cancelar una transicion activa */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow", meta = (WorldContext = "WorldContextObject"))
	static FPGXLevelFlowResult CancelLevelTransition(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Query (PGX|LevelFlow|Query)
	// ES: Consultas (PGX|LevelFlow|Query)
	// ========================================================================

	/** EN: Get current pipeline state / ES: Obtener estado actual del pipeline */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static EPGXLevelFlowState GetTransitionState(const UObject* WorldContextObject);

	/** EN: Get the tag of the current level / ES: Obtener el tag del nivel actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow", meta = (WorldContext = "WorldContextObject"))
	static FGameplayTag GetCurrentLevelTag(const UObject* WorldContextObject);

	/** EN: Get the tag of the previous level / ES: Obtener el tag del nivel anterior */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static FGameplayTag GetPreviousLevelTag(const UObject* WorldContextObject);

	/** EN: Check if a transition is active / ES: Verificar si hay una transicion activa */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static bool IsLevelTransitionActive(const UObject* WorldContextObject);

	/** EN: Get current transition progress (0.0 - 1.0) / ES: Obtener progreso de transicion */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static float GetLevelTransitionProgress(const UObject* WorldContextObject);

	/** EN: Resolve a level tag to its entry data / ES: Resolver un tag de nivel a sus datos */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static bool ResolveLevelByTag(const UObject* WorldContextObject, FGameplayTag LevelTag, FPGXLevelEntry& OutEntry);

	/** EN: Get all registered level tags / ES: Obtener todos los tags de nivel registrados */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FGameplayTag> GetRegisteredLevelTags(const UObject* WorldContextObject);

	/** EN: Get transition history / ES: Obtener historial de transiciones */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXLevelTransitionRecord> GetTransitionHistory(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Sub-Level Management (PGX|LevelFlow|SubLevel)
	// ES: Gestion de Sub-Niveles (PGX|LevelFlow|SubLevel)
	// ========================================================================

	/** EN: Load a sub-level by tag / ES: Cargar un sub-nivel por tag */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|SubLevel", meta = (WorldContext = "WorldContextObject"))
	static FPGXLevelFlowResult RequestSubLevelLoad(const UObject* WorldContextObject, FGameplayTag SubLevelTag);

	/** EN: Unload a sub-level by tag / ES: Descargar un sub-nivel por tag */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|SubLevel", meta = (WorldContext = "WorldContextObject"))
	static FPGXLevelFlowResult RequestSubLevelUnload(const UObject* WorldContextObject, FGameplayTag SubLevelTag);

	/** EN: Check if a sub-level is loaded / ES: Verificar si un sub-nivel esta cargado */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|SubLevel", meta = (WorldContext = "WorldContextObject"))
	static bool IsSubLevelLoaded(const UObject* WorldContextObject, FGameplayTag SubLevelTag);

	/** EN: Get all loaded sub-levels / ES: Obtener todos los sub-niveles cargados */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|SubLevel", meta = (WorldContext = "WorldContextObject"))
	static TArray<FGameplayTag> GetLoadedSubLevels(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Actor & Debug (PGX|LevelFlow|Debug)
	// ES: Actor y Debug (PGX|LevelFlow|Debug)
	// ========================================================================

	/** EN: Get the LevelFlowActor in the current level / ES: Obtener el LevelFlowActor del nivel actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|Debug", meta = (WorldContext = "WorldContextObject"))
	static APGXLevelFlowActor* GetCurrentLevelFlowActor(const UObject* WorldContextObject);

	/** EN: Get total registered level count / ES: Obtener total de niveles registrados */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|Debug", meta = (WorldContext = "WorldContextObject"))
	static int32 GetRegisteredLevelCount(const UObject* WorldContextObject);

	/** EN: Get discovered profile count / ES: Obtener cantidad de perfiles descubiertos */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|Debug", meta = (WorldContext = "WorldContextObject"))
	static int32 GetDiscoveredProfileCount(const UObject* WorldContextObject);

	/** EN: Get pending UE shader compilations (no WorldContext needed) / ES: Obtener compilaciones de shader pendientes */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|LevelFlow|Debug")
	static int32 GetPendingShaderCompilations();

private:
	static UPGXLevelFlowSubsystem* GetSubsystem(const UObject* WorldContextObject);
};
