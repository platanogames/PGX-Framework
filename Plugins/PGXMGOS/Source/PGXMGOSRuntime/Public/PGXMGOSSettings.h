// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "PGXMGOSSettings.generated.h"

class UPGXGCObserverConfig;

// ============================================================================
// EN: MGOS System settings — appears in Project Settings > PGX > MGOS.
//     Provides deterministic config resolution: assign the DA here instead of
//     relying on AssetRegistry auto-discovery.
//
// ES: Settings del Sistema MGOS — aparece en Project Settings > PGX > MGOS.
//     Provee resolucion determinista de config: asignar el DA aqui en vez de
//     depender del auto-discovery de AssetRegistry.
// ============================================================================

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX MGOS"))
class PGXMGOSRUNTIME_API UPGXMGOSSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("MGOS"); }

	/**
	 * EN: The active MGOS (GC Observer) config DA. If empty, falls back to AssetRegistry discovery (deprecated).
	 * ES: El DA de config de MGOS (GC Observer) activo. Si esta vacio, hace fallback a discovery de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "MGOS",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXGCObserverConfig> ActiveConfig;

	/**
	 * EN: Enable verbose logging for config resolution.
	 * ES: Habilitar logging verboso para resolucion de config.
	 */
	UPROPERTY(config, EditAnywhere, Category = "MGOS|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;
};
