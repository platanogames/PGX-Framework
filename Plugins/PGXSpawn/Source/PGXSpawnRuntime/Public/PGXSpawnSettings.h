// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "PGXSpawnSettings.generated.h"

class UPGXSpawnConfig;

/**
 * EN: Project Settings surface for PGXSpawn baseline policy. Mirrors the
 *     Settings-first SSOT pattern used by PGXTrade / PGXEnvironment /
 *     PGXLevelFlow. ActiveConfig points to the resolved UPGXSpawnConfig DA
 *     that UPGXSpawnSubsystem loads at Initialize via
 *     PGX::ResolveSingleConfig<UPGXSpawnConfig>().
 *
 *     If empty, UPGXSpawnSubsystem::EnsureRuntimeObjects() creates a transient
 *     default config (the legacy test/dev fallback) — production projects
 *     should always assign a real DA in Project Settings > PGX > Spawn.
 *
 *     AssetRegistry auto-discovery is deprecated; the helper logs a warning
 *     if it auto-resolves. This setting class does not depend on it.
 *
 * ES: Superficie de Project Settings para la politica baseline de PGXSpawn.
 *     Espeja el patron SSOT Settings-first usado por PGXTrade / PGXEnvironment
 *     / PGXLevelFlow. ActiveConfig apunta al DA UPGXSpawnConfig resuelto que
 *     UPGXSpawnSubsystem carga al Initialize via
 *     PGX::ResolveSingleConfig<UPGXSpawnConfig>().
 *
 *     Si esta vacio, UPGXSpawnSubsystem::EnsureRuntimeObjects() crea un config
 *     default transient (el fallback test/dev legado) — los proyectos de
 *     produccion deben siempre asignar un DA real en Project Settings > PGX > Spawn.
 *
 *     El auto-discovery de AssetRegistry esta deprecado; el helper loguea
 *     warning si auto-resuelve. Esta clase de setting no depende de el.
 */
UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Spawn System"))
class PGXSPAWNRUNTIME_API UPGXSpawnSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("Spawn"); }

	/**
	 * EN: The active PGXSpawn config DA. Consumed by
	 *     UPGXSpawnSubsystem::Initialize via PGX::ResolveSingleConfig.
	 *     If empty, the subsystem falls back to a transient default
	 *     (EnsureRuntimeObjects). Production projects MUST assign a real DA.
	 *
	 * ES: El DA de config PGXSpawn activo. Consumido por
	 *     UPGXSpawnSubsystem::Initialize via PGX::ResolveSingleConfig.
	 *     Si esta vacio, el subsystem cae a un default transient
	 *     (EnsureRuntimeObjects). Proyectos de produccion DEBEN asignar un DA real.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Spawn",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXSpawnConfig> ActiveConfig;
};
