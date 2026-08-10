// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "PGXLevelFlowSettings.generated.h"

class UPGXLevelFlowConfig;
class UDataTable;

// ============================================================================
// EN: LevelFlow System settings — appears in Project Settings > PGX > Level Flow.
//     Provides deterministic config resolution: assign the DA and DataTable here
//     instead of relying on AssetRegistry auto-discovery.
//
// ES: Settings del Sistema LevelFlow — aparece en Project Settings > PGX > Level Flow.
//     Provee resolucion determinista de config: asignar el DA y DataTable aqui
//     en vez de depender del auto-discovery de AssetRegistry.
// ============================================================================

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Level Flow"))
class PGXLOADINGRUNTIME_API UPGXLevelFlowSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("LevelFlow"); }

	/**
	 * EN: The active LevelFlow config DA. If empty, falls back to AssetRegistry discovery (deprecated).
	 * ES: El DA de config de LevelFlow activo. Si esta vacio, hace fallback a discovery de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "LevelFlow",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXLevelFlowConfig> ActiveConfig;

	/**
	 * EN: DataTable with level catalog rows (FPGXLevelCatalogRow).
	 *     Each row maps a catalog GameplayTag to a UPGXLevelProfile DA.
	 *     If empty, falls back to AssetRegistry scan (deprecated).
	 * ES: DataTable con filas de catalogo de niveles (FPGXLevelCatalogRow).
	 *     Cada fila mapea un GameplayTag de catalogo a un DA UPGXLevelProfile.
	 *     Si esta vacio, hace fallback a escaneo de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "LevelFlow|Catalogs",
		meta = (DisplayName = "Level Catalog Table"))
	TSoftObjectPtr<UDataTable> LevelCatalogTable;

	/**
	 * EN: Enable verbose logging for config resolution.
	 * ES: Habilitar logging verboso para resolucion de config.
	 */
	UPROPERTY(config, EditAnywhere, Category = "LevelFlow|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;
};
