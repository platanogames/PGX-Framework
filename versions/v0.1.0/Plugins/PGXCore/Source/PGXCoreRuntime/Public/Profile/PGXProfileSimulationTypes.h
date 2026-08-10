// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Profile/PGXProfileTypes.h"
#include "PGXProfileSimulationTypes.generated.h"

// ============================================================================
// EN: Editor Simulation Types — structs for temporarily overriding profile
//     values during editor sessions without modifying the source DataAsset.
//     Note: USTRUCT must NOT be inside #if WITH_EDITOR (UHT limitation).
//     The struct exists in all builds but is only USED in editor code.
//
// ES: Tipos de Simulacion del Editor — structs para sobreescribir temporalmente
//     valores del profile durante sesiones del editor sin modificar el DataAsset
//     fuente. Nota: USTRUCT NO debe estar dentro de #if WITH_EDITOR (limitacion UHT).
//     El struct existe en todos los builds pero solo se USA en codigo del editor.
// ============================================================================

/**
 * EN: Container for all simulation overrides. Each section can be independently
 *     toggled to override only specific layers of the resolved profile.
 *
 * ES: Contenedor para todos los overrides de simulacion. Cada seccion puede
 *     activarse independientemente para sobreescribir solo capas especificas
 *     del profile resuelto.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXEditorSimulationOverrides
{
	GENERATED_BODY()

	// ---- Platform Override / Override de Plataforma ----

	/** EN: If true, override the active target platform / ES: Si true, sobreescribir la plataforma objetivo activa */
	UPROPERTY(EditAnywhere, Category = "PGX|Profile|Simulation|Platform")
	bool bOverridePlatform = false;

	/** EN: The simulated platform (only used if bOverridePlatform is true) / ES: La plataforma simulada (solo si bOverridePlatform es true) */
	UPROPERTY(EditAnywhere, Category = "PGX|Profile|Simulation|Platform", meta = (EditCondition = "bOverridePlatform", EditConditionHides))
	EPGXTargetPlatform SimulatedPlatform = EPGXTargetPlatform::PC;

	// ---- Build Context Override / Override de Contexto de Build ----

	/** EN: If true, override the build context / ES: Si true, sobreescribir el contexto de build */
	UPROPERTY(EditAnywhere, Category = "PGX|Profile|Simulation|Build")
	bool bOverrideBuildContext = false;

	/** EN: The simulated build context (only used if bOverrideBuildContext is true) / ES: El contexto de build simulado (solo si bOverrideBuildContext es true) */
	UPROPERTY(EditAnywhere, Category = "PGX|Profile|Simulation|Build", meta = (EditCondition = "bOverrideBuildContext", EditConditionHides))
	EPGXBuildContext SimulatedBuildContext = EPGXBuildContext::Development;

	// ---- Budget Override / Override de Presupuesto ----

	/** EN: If true, override budget values / ES: Si true, sobreescribir valores de presupuesto */
	UPROPERTY(EditAnywhere, Category = "PGX|Profile|Simulation|Budgets")
	bool bOverrideBudgets = false;

	/** EN: Simulated budgets (only used if bOverrideBudgets is true) / ES: Presupuestos simulados (solo si bOverrideBudgets es true) */
	UPROPERTY(EditAnywhere, Category = "PGX|Profile|Simulation|Budgets", meta = (EditCondition = "bOverrideBudgets", EditConditionHides))
	FPGXProfileBudgets SimulatedBudgets;

	// ---- Feature Override / Override de Features ----

	/** EN: If true, override feature matrix / ES: Si true, sobreescribir matriz de features */
	UPROPERTY(EditAnywhere, Category = "PGX|Profile|Simulation|Features")
	bool bOverrideFeatures = false;

	/** EN: Simulated feature matrix (only used if bOverrideFeatures is true) / ES: Matriz de features simulada (solo si bOverrideFeatures es true) */
	UPROPERTY(EditAnywhere, Category = "PGX|Profile|Simulation|Features", meta = (EditCondition = "bOverrideFeatures", EditConditionHides))
	FPGXProfileFeatureMatrix SimulatedFeatures;

	/** EN: Check if any override is active / ES: Verificar si algun override esta activo */
	bool HasAnyOverride() const
	{
		return bOverridePlatform || bOverrideBuildContext || bOverrideBudgets || bOverrideFeatures;
	}

	/** EN: Reset all overrides to inactive / ES: Resetear todos los overrides a inactivo */
	void Reset()
	{
		bOverridePlatform = false;
		bOverrideBuildContext = false;
		bOverrideBudgets = false;
		bOverrideFeatures = false;
	}
};
