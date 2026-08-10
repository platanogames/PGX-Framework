// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "Observability/PGXObservable.h"
#include "PGXLoadingSettings.generated.h"

class UPGXLoadingConfig;
class UDataTable;

// ============================================================================
// EN: Loading System settings — appears in Project Settings > PGX > Loading System.
//     Provides deterministic config resolution: assign the DA and DataTable here
//     instead of relying on AssetRegistry auto-discovery.
//
// ES: Settings del Sistema de Loading — aparece en Project Settings > PGX > Loading System.
//     Provee resolucion determinista de config: asignar el DA y DataTable aqui
//     en vez de depender del auto-discovery de AssetRegistry.
// ============================================================================

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Loading System"))
class PGXLOADINGRUNTIME_API UPGXLoadingSettings : public UPGXSettings, public IPGXObservable
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("Loading"); }

	// EN: IPGXObservable contract — direct-inline implementation.
	// ES: Contrato IPGXObservable — implementacion direct-inline.
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;

	/**
	 * EN: The active Loading config DA. If empty, falls back to AssetRegistry discovery (deprecated).
	 * ES: El DA de config de Loading activo. Si esta vacio, hace fallback a discovery de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Loading",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXLoadingConfig> ActiveConfig;

	/**
	 * EN: DataTable with loading profile rows (FPGXLoadingProfileRow).
	 *     Each row maps a context GameplayTag to a UPGXLoadingProfile DA.
	 *     If empty, falls back to AssetRegistry scan (deprecated).
	 * ES: DataTable con filas de profile de loading (FPGXLoadingProfileRow).
	 *     Cada fila mapea un GameplayTag de contexto a un DA UPGXLoadingProfile.
	 *     Si esta vacio, hace fallback a escaneo de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Loading|Profiles",
		meta = (DisplayName = "Loading Profile Table"))
	TSoftObjectPtr<UDataTable> LoadingProfileTable;

	/**
	 * EN: Enable verbose logging for config resolution.
	 * ES: Habilitar logging verboso para resolucion de config.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Loading|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;
};
