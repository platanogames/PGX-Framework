// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXPSOTypes.h"
#include "PGXPSOBlueprintLibrary.generated.h"

class UPGXPSOSubsystem;

/**
 * EN: Blueprint Function Library for the PGX PSO system.
 *     Exposes warm-up control, context management, cache ops, and native
 *     shader pipeline access via static WorldContext-dispatch nodes.
 * ES: Blueprint Function Library para el sistema PGX PSO.
 *     Expone control de warm-up, gestion de contexto, ops de cache y acceso
 *     nativo al pipeline de shaders via nodos estaticos con WorldContext-dispatch.
 */
UCLASS()
class PGXPSORUNTIME_API UPGXPSOBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Warm-Up Control (PGX|PSO)
	// ES: Control de Warm-Up (PGX|PSO)
	// ========================================================================

	/** EN: Request warm-up for all discovered configs / ES: Solicitar warm-up para todas las configs descubiertas */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO", meta = (WorldContext = "WorldContextObject"))
	static bool RequestPSOWarmUp(const UObject* WorldContextObject);

	/** EN: Request warm-up for entries matching a specific context tag / ES: Solicitar warm-up para entradas que coincidan con un tag de contexto */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Advanced", meta = (WorldContext = "WorldContextObject"))
	static bool RequestPSOWarmUpForContext(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/** EN: Pause active warm-up / ES: Pausar warm-up activo */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO", meta = (WorldContext = "WorldContextObject"))
	static void PausePSOWarmUp(const UObject* WorldContextObject);

	/** EN: Resume paused warm-up / ES: Reanudar warm-up pausado */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO", meta = (WorldContext = "WorldContextObject"))
	static void ResumePSOWarmUp(const UObject* WorldContextObject);

	/** EN: Cancel active warm-up / ES: Cancelar warm-up activo */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO", meta = (WorldContext = "WorldContextObject"))
	static void CancelPSOWarmUp(const UObject* WorldContextObject);

	/** EN: Get current warm-up state / ES: Obtener estado actual de warm-up */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|PSO|Query", meta = (WorldContext = "WorldContextObject"))
	static EPGXPSOWarmUpState GetPSOWarmUpState(const UObject* WorldContextObject);

	/** EN: Get current warm-up progress snapshot / ES: Obtener snapshot de progreso de warm-up actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|PSO|Query", meta = (WorldContext = "WorldContextObject"))
	static FPGXPSOWarmUpProgress GetPSOProgress(const UObject* WorldContextObject);

	/** EN: Get warm-up progress as percentage (0.0 - 100.0) / ES: Obtener progreso de warm-up como porcentaje (0.0 - 100.0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|PSO|Query", meta = (WorldContext = "WorldContextObject"))
	static float GetPSOProgressPercent(const UObject* WorldContextObject);

	/** EN: Check if warm-up is complete / ES: Verificar si el warm-up esta completo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|PSO|Query", meta = (WorldContext = "WorldContextObject"))
	static bool IsPSOWarmUpComplete(const UObject* WorldContextObject);

	/** EN: Check if cache has unsaved PSOs / ES: Verificar si el cache tiene PSOs sin guardar */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|PSO|Query", meta = (WorldContext = "WorldContextObject"))
	static bool IsPSOCacheDirty(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Context Management (PGX|PSO|Context)
	// ES: Gestion de Contexto (PGX|PSO|Context)
	// ========================================================================

	/** EN: Add a context tag for filtering / ES: Agregar un tag de contexto para filtrado */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Advanced", meta = (WorldContext = "WorldContextObject"))
	static void AddPSOContext(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/** EN: Remove a context tag / ES: Remover un tag de contexto */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Advanced", meta = (WorldContext = "WorldContextObject"))
	static void RemovePSOContext(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/** EN: Get all active context tags / ES: Obtener todos los tags de contexto activos */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|PSO|Advanced", meta = (WorldContext = "WorldContextObject"))
	static TArray<FGameplayTag> GetActivePSOContexts(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Cache Operations (PGX|PSO|Cache)
	// ES: Operaciones de Cache (PGX|PSO|Cache)
	// ========================================================================

	/** EN: Save PSO cache to disk / ES: Guardar cache PSO a disco */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Advanced", meta = (WorldContext = "WorldContextObject"))
	static void SavePSOCacheToDisk(const UObject* WorldContextObject);

	/** EN: Get number of discovered PSO configs / ES: Obtener numero de configs PSO descubiertas */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|PSO|Debug", meta = (WorldContext = "WorldContextObject"))
	static int32 GetDiscoveredPSOConfigCount(const UObject* WorldContextObject);

	/** EN: Get pending UE native PSO compilations remaining / ES: Obtener compilaciones PSO nativas de UE pendientes */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|PSO|Debug")
	static int32 GetPendingPSOCompilations();

	// ========================================================================
	// EN: Native Shader Cache (PGX|PSO|Native)
	// ES: Cache de Shaders Nativo (PGX|PSO|Native)
	// ========================================================================

	/** EN: Pause UE shader pipeline cache batching / ES: Pausar batching del cache de pipeline de shaders de UE */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Native")
	static void PauseShaderCacheBatching();

	/** EN: Resume UE shader pipeline cache batching / ES: Reanudar batching del cache de pipeline de shaders de UE */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Native")
	static void ResumeShaderCacheBatching();

private:
	/** EN: Get PSO subsystem from world context / ES: Obtener subsistema PSO desde world context */
	static UPGXPSOSubsystem* GetSubsystem(const UObject* WorldContextObject);
};
