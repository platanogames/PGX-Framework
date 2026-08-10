// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "PGXGameFlowSettings.generated.h"

class UPGXGameFlowConfig;
class UDataTable;

// ============================================================================
// EN: GameFlow System settings — appears in Project Settings > PGX > Game Flow.
//     Provides deterministic config resolution: assign the DA and DataTable here
//     instead of relying on AssetRegistry auto-discovery.
//
// ES: Settings del Sistema GameFlow — aparece en Project Settings > PGX > Game Flow.
//     Provee resolucion determinista de config: asignar el DA y DataTable aqui
//     en vez de depender del auto-discovery de AssetRegistry.
// ============================================================================

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Game Flow"))
class PGXGAMEFLOWRUNTIME_API UPGXGameFlowSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("GameFlow"); }

	/**
	 * EN: The active GameFlow config DA. If empty, falls back to AssetRegistry discovery (deprecated).
	 * ES: El DA de config de GameFlow activo. Si esta vacio, hace fallback a discovery de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "GameFlow",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXGameFlowConfig> ActiveConfig;

	/**
	 * EN: DataTable with flow rules rows (FPGXFlowRulesRow).
	 *     Each row maps a flow channel to a UPGXFlowRulesConfig DA.
	 *     If empty, falls back to AssetRegistry scan (deprecated).
	 * ES: DataTable con filas de reglas de flujo (FPGXFlowRulesRow).
	 *     Cada fila mapea un canal de flujo a un DA UPGXFlowRulesConfig.
	 *     Si esta vacio, hace fallback a escaneo de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "GameFlow|Rules",
		meta = (DisplayName = "Flow Rules Table"))
	TSoftObjectPtr<UDataTable> FlowRulesTable;

	/**
	 * EN: Enable verbose logging for config resolution.
	 * ES: Habilitar logging verboso para resolucion de config.
	 */
	UPROPERTY(config, EditAnywhere, Category = "GameFlow|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;
};
