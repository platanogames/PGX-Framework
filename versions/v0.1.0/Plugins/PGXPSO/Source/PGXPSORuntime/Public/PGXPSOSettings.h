// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXPSOTypes.h"
#include "Trace/PGXTraceTypes.h"
#include "PGXPSOSettings.generated.h"

class UDataTable;

/**
 * EN: Project Settings for the PGX PSO System.
 *     Complements Config DAs with project-level settings that persist in .ini.
 *     Accessible in Editor: Project Settings > PGX > PSO System
 * ES: Project Settings para el Sistema PSO de PGX.
 *     Complementa Config DAs con settings de proyecto que persisten en .ini.
 *     Accesible en Editor: Project Settings > PGX > PSO System
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "PGX PSO System"))
class PGXPSORUNTIME_API UPGXPSOSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Config Resolution
	// ES: Resolucion de Config
	// ========================================================================

	/**
	 * EN: Canonical DataTable that maps config GameplayTags to UPGXPSOWarmUpConfig DAs
	 *     (each row is `FPGXPSOConfigRow`). When set, this is the deterministic discovery
	 *     path the subsystem consumes at Initialize; `DiscoveryMode` is NOT consulted.
	 *     When empty, the subsystem falls back to AssetRegistry scan (deprecated path
	 *     kept for backwards compatibility — see `DiscoveryMode`).
	 * ES: DataTable canonica que mapea GameplayTags de config a DAs UPGXPSOWarmUpConfig
	 *     (cada fila es `FPGXPSOConfigRow`). Cuando esta asignada, es el path de discovery
	 *     deterministico que el subsystem consume en Initialize; `DiscoveryMode` NO se
	 *     consulta. Cuando esta vacia, se cae al escaneo AssetRegistry (path deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Config",
		meta = (DisplayName = "PSO Config Table"))
	TSoftObjectPtr<UDataTable> PSOConfigTable;

	// ========================================================================
	// EN: Discovery (Fallback Path)
	// ES: Descubrimiento (Path de Fallback)
	// ========================================================================

	/** EN: Discovery strategy ONLY consulted when `PSOConfigTable` is unset.
	 *      The canonical resolution path is `PSOConfigTable` (DataTable, deterministic).
	 *      `AssetRegistryScan` is the deprecated fallback used when no DataTable is
	 *      assigned. `Manual` mode is reserved and currently has no
	 *      runtime implementation; selecting it falls through to AssetRegistry scan.
	 *  ES: Estrategia de discovery SOLO consultada cuando `PSOConfigTable` esta vacio.
	 *      El path canonico de resolucion es `PSOConfigTable` (DataTable, deterministico).
	 *      `AssetRegistryScan` es el fallback deprecated usado cuando no hay DataTable
	 *      asignada. `Manual` esta reservado y no tiene implementacion
	 *      en runtime; al seleccionarlo se cae a AssetRegistry scan. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery",
		meta = (ToolTip = "Only consulted when PSOConfigTable is unset. AssetRegistryScan is the deprecated fallback path; Manual mode is reserved (no runtime impl)."))
	EPGXPSODiscoveryMode DiscoveryMode = EPGXPSODiscoveryMode::AssetRegistryScan;

	/** EN: [Reserved] Explicit Config DA paths for the Manual discovery mode. Manual mode has
	 *      no runtime implementation today; this list is parsed but not consumed. Kept for the
	 *      shape of Manual mode even though the current runtime does not consume it.
	 *  ES: [Reservado] Rutas explicitas a Config DAs para el modo Manual. Manual no tiene
	 *      implementacion en runtime; esta lista se parsea pero no se consume. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery",
		meta = (EditCondition = "DiscoveryMode != EPGXPSODiscoveryMode::AssetRegistryScan", EditConditionHides,
		ToolTip = "[Reserved] Manual mode is not yet wired into runtime; entries here are not consumed."))
	TArray<FSoftObjectPath> ExplicitConfigPaths;

	// ========================================================================
	// EN: Recorder (Editor-only)
	// ES: Grabador (Solo editor)
	// ========================================================================

	/** EN: Hitch threshold for the recorder in ms / ES: Umbral de hitch para el grabador en ms */
	UPROPERTY(config, EditAnywhere, Category = "Recorder",
		meta = (ClampMin = "1.0"))
	float HitchThresholdMs = 8.0f;

	// ========================================================================
	// EN: Cache
	// ES: Cache
	// ========================================================================

	/** EN: When true, the subsystem calls `SaveCacheToDisk()` on `Deinitialize()` if the cache
	 *      is dirty. Save is performed via `FShaderPipelineCache::SavePipelineFileCache(Incremental)`
	 *      using the engine's standard pipeline cache directory; project-side `CacheDirectory`
	 *      below is exposed for completeness but is currently NOT routed through to the engine
	 *      save path (engine default location is used). Default: true.
	 *  ES: Si true, el subsystem llama a `SaveCacheToDisk()` en `Deinitialize()` cuando el cache
	 *      esta dirty. El guardado usa `FShaderPipelineCache::SavePipelineFileCache(Incremental)`
	 *      con la ruta de cache estandar del engine; `CacheDirectory` abajo se expone pero NO
	 *      se rutea actualmente hacia el path de guardado del engine. Default: true. */
	UPROPERTY(config, EditAnywhere, Category = "Cache",
		meta = (ToolTip = "If true, save the PSO cache on subsystem Deinitialize when dirty. Engine default cache path is used."))
	bool bAutoSaveCacheOnShutdown = true;

	/** EN: [Reserved] Project-relative directory for PSO cache files. Currently NOT routed to
	 *      the engine save path — `FShaderPipelineCache::SavePipelineFileCache` uses the engine
	 *      default location. Setting kept exposed for forward compatibility when the runtime
	 *      adds an explicit override hook.
	 *  ES: [Reservado] Directorio (relativo a proyecto) para archivos de cache PSO. Actualmente
	 *      NO se rutea al path de guardado del engine. */
	UPROPERTY(config, EditAnywhere, Category = "Cache",
		meta = (ToolTip = "[Reserved] CacheDirectory override is not yet wired to the engine save path."))
	FString CacheDirectory = TEXT("PSOCache");

	// ========================================================================
	// EN: Traceability
	// ES: Trazabilidad
	// ========================================================================

	/** EN: Trace configuration for the PSO system / ES: Configuracion de traza para el sistema PSO */
	UPROPERTY(config, EditAnywhere, Category = "Trace")
	FPGXTraceConfig TraceConfig;

	// ========================================================================
	// EN: UDeveloperSettings Interface
	// ES: Interfaz UDeveloperSettings
	// ========================================================================

	FName GetCategoryName() const override { return TEXT("PGX"); }
};
