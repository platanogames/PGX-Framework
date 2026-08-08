// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "Observability/PGXObservable.h"
#include "PGXLogSettings.generated.h"

class UPGXLogConfig;
class UPGXLogDomainConfig;

// ============================================================================
// EN: Log System settings — appears in Project Settings > PGX > Log System.
//     Provides deterministic config resolution: assign the DAs here instead of
//     relying on AssetRegistry auto-discovery.
//
// ES: Settings del Sistema de Log — aparece en Project Settings > PGX > Log System.
//     Provee resolucion determinista de config: asignar los DAs aqui en vez de
//     depender del auto-discovery de AssetRegistry.
// ============================================================================

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Log System"))
class PGXCORERUNTIME_API UPGXLogSettings : public UPGXSettings, public IPGXObservable
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("LogSystem"); }

	//~ Begin IPGXObservable (delegates to PGXCoreObservability helpers)
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/**
	 * EN: The active Log config DA (dashboard profile). If empty, falls back to AssetRegistry discovery (deprecated).
	 * ES: El DA de config de Log activo (perfil de dashboard). Si esta vacio, hace fallback a discovery de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Log",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXLogConfig> ActiveConfig;

	/**
	 * EN: Log Domain config DAs. Assign all domain configs here for deterministic resolution.
	 *     If empty, falls back to AssetRegistry scan (deprecated).
	 * ES: DAs de config de Log Domain. Asignar todos los configs de dominio aqui para resolucion determinista.
	 *     Si esta vacio, hace fallback a escaneo de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Log|Domains",
		meta = (DisplayName = "Domain Configs"))
	TArray<TSoftObjectPtr<UPGXLogDomainConfig>> DomainConfigs;

	/**
	 * EN: Enable verbose logging for config resolution.
	 * ES: Habilitar logging verboso para resolucion de config.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Log|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;
};
