// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Profile/PGXProfileTypes.h"
#include "Profile/PGXProfileOverrides.h"
#include "Profile/PGXPlatformConfig.h"
#include "PGXProjectProfileConfig.generated.h"

class UPGXPlatformConfig;

// ============================================================================
// EN: Project Profile Config DataAsset — the "constitution" of a PGX project.
//     Defines the base 5-layer profile plus per-platform and per-build overrides.
//     Discoverable via AssetRegistry. Selected in Project Settings > PGX > Profile.
//
// ES: Config DataAsset de Profile de Proyecto — la "constitucion" de un proyecto PGX.
//     Define el profile base de 5 capas mas overrides por plataforma y por build.
//     Descubrible via AssetRegistry. Seleccionado en Project Settings > PGX > Profile.
// ============================================================================

UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXProjectProfileConfig : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Base Profile — 5 Layers
	// ========================================================================

	/** EN: Layer 1 — Project identity / ES: Capa 1 — Identidad del proyecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Profile|Identity")
	FPGXProfileIdentity Identity;

	/** EN: Layer 2 — Base capabilities / ES: Capa 2 — Capacidades base */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Profile|Capabilities")
	FPGXProfileCapabilities BaseCapabilities;

	/** EN: Layer 3 — Base policies / ES: Capa 3 — Politicas base */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Profile|Policies", meta = (AdvancedDisplay))
	FPGXProfilePolicies BasePolicies;

	/** EN: Layer 4 — Base budgets / ES: Capa 4 — Presupuestos base */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Profile|Budgets", meta = (AdvancedDisplay))
	FPGXProfileBudgets BaseBudgets;

	/** EN: Layer 5 — Base feature matrix / ES: Capa 5 — Matriz de features base */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Profile|Features", meta = (AdvancedDisplay))
	FPGXProfileFeatureMatrix BaseFeatures;

	// ========================================================================
	// Platform Overrides
	// EN: Per-platform adjustments. Applied during resolution for active targets.
	// ES: Ajustes por plataforma. Aplicados durante resolucion para targets activos.
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Profile|Overrides", meta = (AdvancedDisplay))
	TMap<EPGXTargetPlatform, FPGXPlatformOverride> PlatformOverrides;

	// ========================================================================
	// Build Overrides
	// EN: Per-build-context adjustments. Applied during resolution for current build.
	// ES: Ajustes por contexto de build. Aplicados durante resolucion para el build actual.
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Profile|Overrides", meta = (AdvancedDisplay))
	TMap<EPGXBuildContext, FPGXBuildOverride> BuildOverrides;

	// ========================================================================
	// Platform Configs (v2.0)
	// EN: Per-platform configuration DAs with per-system budgets and traits.
	//     Loaded during profile resolution. Keyed by target platform.
	// ES: DAs de configuracion por plataforma con presupuestos por sistema y traits.
	//     Cargados durante resolucion de profile. Indexados por plataforma.
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Profile|PlatformConfigs", meta = (AdvancedDisplay))
	TMap<EPGXTargetPlatform, TSoftObjectPtr<UPGXPlatformConfig>> PlatformConfigs;
};
