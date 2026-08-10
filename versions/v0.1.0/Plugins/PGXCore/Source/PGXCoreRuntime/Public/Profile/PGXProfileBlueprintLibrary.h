// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Profile/PGXProfileTypes.h"
#include "PGXProfileBlueprintLibrary.generated.h"

// ============================================================================
// EN: Blueprint Function Library for the PGX Profile System.
//     Provides ~12 static query nodes for Blueprint graphs.
//     All methods resolve the subsystem via WorldContext.
//
// ES: Libreria de Funciones Blueprint para el Sistema de Profile PGX.
//     Proporciona ~12 nodos estaticos de consulta para grafos Blueprint.
//     Todos los metodos resuelven el subsistema via WorldContext.
// ============================================================================

class UPGXProfileSubsystem;

UCLASS()
class PGXCORERUNTIME_API UPGXProfileBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Profile State
	// ========================================================================

	/** EN: Get the full resolved profile / ES: Obtener el profile resuelto completo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile",
		meta = (WorldContext = "WorldContextObject"))
	static FPGXResolvedProfile GetProjectProfile(const UObject* WorldContextObject);

	/** EN: Check if the profile is resolved and ready / ES: Verificar si el profile esta resuelto y listo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsProfileResolved(const UObject* WorldContextObject);

	/** EN: Get the current profile state / ES: Obtener el estado actual del profile */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile",
		meta = (WorldContext = "WorldContextObject"))
	static EPGXProfileState GetProfileState(const UObject* WorldContextObject);

	// ========================================================================
	// Capability Queries
	// ========================================================================

	/** EN: Check if a named capability is enabled / ES: Verificar si una capacidad nombrada esta habilitada */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile|Capabilities",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsCapabilityEnabled(const UObject* WorldContextObject, FName CapabilityName);

	// ========================================================================
	// Feature Queries
	// ========================================================================

	/** EN: Check if a named feature is allowed / ES: Verificar si un feature nombrado esta permitido */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile|Features",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsFeatureAllowed(const UObject* WorldContextObject, FName FeatureName);

	// ========================================================================
	// Budget Queries
	// ========================================================================

	/** EN: Get a named budget value (0 = unlimited) / ES: Obtener valor de presupuesto (0 = ilimitado) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile|Budgets",
		meta = (WorldContext = "WorldContextObject"))
	static int64 GetBudgetValue(const UObject* WorldContextObject, FName BudgetName);

	// ========================================================================
	// Identity Queries
	// ========================================================================

	/** EN: Get the project mode / ES: Obtener el modo del proyecto */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile|Identity",
		meta = (WorldContext = "WorldContextObject"))
	static EPGXProjectMode GetProjectMode(const UObject* WorldContextObject);

	/** EN: Check if a target platform is active / ES: Verificar si una plataforma objetivo esta activa */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile|Identity",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsTargetPlatformActive(const UObject* WorldContextObject, EPGXTargetPlatform Platform);

	/** EN: Get all active target platforms / ES: Obtener todas las plataformas objetivo activas */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile|Identity",
		meta = (WorldContext = "WorldContextObject"))
	static TArray<EPGXTargetPlatform> GetActiveTargets(const UObject* WorldContextObject);

	/** EN: Get the restriction level / ES: Obtener el nivel de restriccion */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile|Identity",
		meta = (WorldContext = "WorldContextObject"))
	static EPGXRestrictionLevel GetRestrictionLevel(const UObject* WorldContextObject);

	/** EN: Get the build context / ES: Obtener el contexto de build */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile|Identity",
		meta = (WorldContext = "WorldContextObject"))
	static EPGXBuildContext GetBuildContext(const UObject* WorldContextObject);

	// ========================================================================
	// Persistence Queries
	// ========================================================================

	/** EN: Get persistence backend for a system / ES: Obtener backend de persistencia para un sistema */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Profile|Policies",
		meta = (WorldContext = "WorldContextObject"))
	static EPGXPersistenceBackend GetPersistenceBackend(const UObject* WorldContextObject, FName SystemName);

private:
	static UPGXProfileSubsystem* GetSubsystem(const UObject* WorldContextObject);
};
