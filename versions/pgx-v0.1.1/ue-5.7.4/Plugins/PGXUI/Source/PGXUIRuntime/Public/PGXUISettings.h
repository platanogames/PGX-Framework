// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXUISettings.generated.h"

class UPGXUIConfig;

/**
 * EN: Discovery strategy for the active UI Config DA.
 *     - `AssetRegistryScan`: deprecated fallback consulted only when `ActiveConfig` is unset.
 *     - `Manual`: reserved with no runtime implementation at preview.
 * ES: Estrategia de descubrimiento del Config DA activo de UI.
 */
UENUM(BlueprintType)
enum class EPGXUIDiscoveryMode : uint8
{
	/** EN: Scan AssetRegistry for UPGXUIConfig DAs (deprecated; only consulted when ActiveConfig is unset). */
	AssetRegistryScan UMETA(DisplayName = "Asset Registry Scan"),

	/** EN: [Reserved] Manual mode — explicit config paths. Not consumed at runtime to runtime. */
	Manual            UMETA(DisplayName = "Manual (Reserved)")
};

/**
 * EN: Project Settings for the PGX UI system. Settings-first canonical resolution path consistent
 *     with the PGX-wide convention validated across PGXMessage / PGXPSO / PGXInput / PGXAI / PGXColony
 *     baselines. **Runtime consumption is NOT CONSUMED AT RUNTIME at preview**: the
 *     preview (`29a68b9`) UPGXUISubsystem reads its `UPGXUIConfig` defaults directly via
 *     `GetMutableDefault<UPGXUIConfig>()`-equivalent flow. This Settings UDeveloperSettings ships
 *     the SHAPE so future runners can populate `ActiveConfig` and so cross-plugin consumers see a
 *     uniform Settings surface; the runtime integration (presentation policy resolution from
 *     Config DA / Settings / Object DA) will start consuming `ActiveConfig`.
 *
 * ES: Project Settings del sistema PGXUI. Resolucion Settings-first canonica. **Consumo en runtime
 *     NO esta cableado todavia en preview** — el subsistema preview lee defaults
 *     de UPGXUIConfig directamente. una futura actualizacion cableara el consumo de ActiveConfig.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "PGX UI System"))
class PGXUIRUNTIME_API UPGXUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Config Resolution
	// ES: Resolucion de Config
	// ========================================================================

	/**
	 * EN: Canonical UI config DA setting — declared at preview as the future
	 *     deterministic primary, but **runtime consumption is NOT CONSUMED AT RUNTIME**. UPGXUISubsystem
	 *     does not load `ActiveConfig` today; the preview service objects consume
	 *     `UPGXUIConfig` defaults directly. Presentation policy resolution is not performed by the current runtime.
	 *     This UPROPERTY is the SHAPE that future consumers will read; populating it now has no
	 *     runtime effect, but doing so is harmless and forward-compatible.
	 * ES: Setting de DA canonica de config UI. preview declara la forma; consumo
	 *     runtime cableado en update planned work dedicada.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Config",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXUIConfig> ActiveConfig;

	// ========================================================================
	// EN: Discovery (Fallback Path — declared, not consumed at runtime)
	// ES: Descubrimiento (Path de Fallback — declarado, todavia no cableado)
	// ========================================================================

	/**
	 * EN: Discovery strategy declared at preview as the eventual fallback path
	 *     consulted when `ActiveConfig` is unset. **Runtime consumption is NOT CONSUMED AT RUNTIME** — the
	 *     subsystem does not invoke AssetRegistry scan today. `AssetRegistryScan` is the future
	 *     deprecated fallback (per the PGX-wide Settings-first canonical), `Manual` is reserved
	 *     with no runtime implementation. Setting kept exposed so projects can pre-populate; the
	 *     runtime integration will start consuming it.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Discovery",
		meta = (ToolTip = "Declares configuration metadata; runtime does not consume this setting."))
	EPGXUIDiscoveryMode DiscoveryMode = EPGXUIDiscoveryMode::AssetRegistryScan;

	/** EN: Enable verbose logging for config resolution. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;

	// ========================================================================
	// EN: UDeveloperSettings Interface
	// ES: Interfaz UDeveloperSettings
	// ========================================================================

	FName GetCategoryName() const override { return TEXT("PGX"); }
};
