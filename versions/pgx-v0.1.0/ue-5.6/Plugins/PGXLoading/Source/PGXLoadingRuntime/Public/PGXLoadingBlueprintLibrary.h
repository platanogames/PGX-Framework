// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXLoadingTypes.h"
#include "PGXLoadingBlueprintLibrary.generated.h"

class UPGXLoadingSubsystem;

/**
 * EN: Blueprint Function Library for the PGX Loading Screen system.
 *     Exposes loading control, query, profile info, and history
 *     via static WorldContext-dispatch nodes.
 *
 * ES: Blueprint Function Library para el sistema PGX Loading Screen.
 *     Expone control de carga, consultas, info de perfiles, e historial
 *     via nodos estaticos con WorldContext-dispatch.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXLoadingBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Loading Control (PGX|Loading)
	// ES: Control de Carga (PGX|Loading)
	// ========================================================================

	/** EN: Request loading screen by context tag / ES: Solicitar pantalla de carga por tag */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading", meta = (WorldContext = "WorldContextObject"))
	static FPGXLoadingResult RequestLoading(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/** EN: Force close loading screen from any state / ES: Forzar cierre desde cualquier estado */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading", meta = (WorldContext = "WorldContextObject"))
	static FPGXLoadingResult ForceCloseLoading(const UObject* WorldContextObject);

	/** EN: Request skip (validates conditions before closing) / ES: Solicitar skip */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading", meta = (WorldContext = "WorldContextObject"))
	static FPGXLoadingResult RequestSkip(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Query (PGX|Loading|Query)
	// ES: Consultas (PGX|Loading|Query)
	// ========================================================================

	/** EN: Is any loading screen currently active? / ES: Hay alguna pantalla de carga activa? */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Query", meta = (WorldContext = "WorldContextObject"))
	static bool IsLoadingActive(const UObject* WorldContextObject);

	/** EN: Get current state machine state / ES: Obtener estado actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Query", meta = (WorldContext = "WorldContextObject"))
	static EPGXLoadingScreenState GetLoadingScreenState(const UObject* WorldContextObject);

	/** EN: Get the context tag of the active loading screen / ES: Obtener tag de contexto activo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Query", meta = (WorldContext = "WorldContextObject"))
	static FGameplayTag GetCurrentLoadingContext(const UObject* WorldContextObject);

	/** EN: Get current progress information / ES: Obtener informacion de progreso */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Query", meta = (WorldContext = "WorldContextObject"))
	static FPGXLoadingProgress GetLoadingProgress(const UObject* WorldContextObject);

	/** EN: Get elapsed time since loading started / ES: Obtener tiempo transcurrido */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Query", meta = (WorldContext = "WorldContextObject"))
	static float GetLoadingElapsedTime(const UObject* WorldContextObject);

	/** EN: Get the active visual type / ES: Obtener tipo visual activo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Query", meta = (WorldContext = "WorldContextObject"))
	static EPGXLoadingVisualType GetActiveVisualType(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Advanced (PGX|Loading|Advanced)
	// ES: Avanzado (PGX|Loading|Advanced)
	// ========================================================================

	/** EN: Check if a profile exists for the given context tag / ES: Verificar si existe perfil */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Advanced", meta = (WorldContext = "WorldContextObject"))
	static bool IsLoadingProfileValid(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/** EN: Get number of discovered profiles / ES: Obtener cantidad de perfiles */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Advanced", meta = (WorldContext = "WorldContextObject"))
	static int32 GetDiscoveredProfileCount(const UObject* WorldContextObject);

	/** EN: Get all registered context tags / ES: Obtener todos los tags de contexto */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Advanced", meta = (WorldContext = "WorldContextObject"))
	static TArray<FGameplayTag> GetRegisteredContextTags(const UObject* WorldContextObject);

	// ========================================================================
	// EN: History & Debug (PGX|Loading|Debug)
	// ES: Historial y Debug (PGX|Loading|Debug)
	// ========================================================================

	/** EN: Get loading history records / ES: Obtener historial de carga */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Debug", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXLoadingRecord> GetLoadingHistory(const UObject* WorldContextObject);

	/** EN: Get duration of the last completed loading / ES: Obtener duracion de la ultima carga */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Loading|Debug", meta = (WorldContext = "WorldContextObject"))
	static float GetLastLoadingDuration(const UObject* WorldContextObject);

private:
	static UPGXLoadingSubsystem* GetSubsystem(const UObject* WorldContextObject);
};
