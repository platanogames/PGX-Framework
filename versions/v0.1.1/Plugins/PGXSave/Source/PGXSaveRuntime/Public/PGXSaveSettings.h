// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "PGXSaveSettings.generated.h"

class UDataTable;

// ============================================================================
// EN: Save System settings — appears in Project Settings > PGX > Save System.
//     Provides deterministic config resolution: assign the DataTable here
//     instead of relying on AssetRegistry auto-discovery.
//
// ES: Settings del Sistema de Save — aparece en Project Settings > PGX > Save System.
//     Provee resolucion determinista de config: asignar el DataTable aqui
//     en vez de depender del auto-discovery de AssetRegistry.
// ============================================================================

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Save System"))
class PGXSAVERUNTIME_API UPGXSaveSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("SaveSystem"); }

	/**
	 * EN: DataTable with save context rows (FPGXSaveContextRow).
	 *     Each row maps a context GameplayTag to a UPGXSaveConfig DA.
	 *     If empty, falls back to AssetRegistry scan (deprecated).
	 * ES: DataTable con filas de contexto de save (FPGXSaveContextRow).
	 *     Cada fila mapea un GameplayTag de contexto a un DA UPGXSaveConfig.
	 *     Si esta vacio, hace fallback a escaneo de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Save",
		meta = (DisplayName = "Save Context Table"))
	TSoftObjectPtr<UDataTable> SaveContextTable;

	/**
	 * EN: Enable verbose logging for config resolution.
	 * ES: Habilitar logging verboso para resolucion de config.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Save|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;

	/**
	 * EN: Cap on the number of FPGXSaveOperationRecord entries retained in the
	 *     subsystem's operation history ring buffer (oldest evicted). Set to 0
	 *     to disable history retention. Resolved at subsystem Initialize and
	 *     by GetDebugSnapshot consumers (inspector, console, automation tests).
	 *     Project-wide data-driven setting.
	 *
	 * ES: Cap del numero de entradas FPGXSaveOperationRecord retenidas en el ring
	 *     buffer de historial de operaciones del subsistema (mas antiguo evictado).
	 *     Set a 0 para deshabilitar retencion de historial. Resuelto al Initialize
	 *     del subsistema y por consumers de GetDebugSnapshot (inspector, consola,
	 *     tests de automation). Setting project-wide y data-driven.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Save|Debug",
		meta = (DisplayName = "Max Operation History", ClampMin = "0", ClampMax = "10000"))
	int32 MaxOperationHistory = 100;
};
