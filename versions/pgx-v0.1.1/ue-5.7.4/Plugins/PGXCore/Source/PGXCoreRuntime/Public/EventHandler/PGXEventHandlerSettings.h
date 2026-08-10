// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "Observability/PGXObservable.h"
#include "PGXEventHandlerSettings.generated.h"

class UPGXEventHandlerConfig;

// ============================================================================
// EN: EventHandler System settings — appears in Project Settings > PGX > Event Handler.
//     Provides deterministic config resolution: assign the DA here instead of
//     relying on AssetRegistry auto-discovery.
//
// ES: Settings del Sistema EventHandler — aparece en Project Settings > PGX > Event Handler.
//     Provee resolucion determinista de config: asignar el DA aqui en vez de
//     depender del auto-discovery de AssetRegistry.
// ============================================================================

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Event Handler"))
class PGXCORERUNTIME_API UPGXEventHandlerSettings : public UPGXSettings, public IPGXObservable
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("EventHandler"); }

	//~ Begin IPGXObservable (delegates to PGXCoreObservability helpers)
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/**
	 * EN: The active EventHandler config DA. If empty, falls back to AssetRegistry discovery (deprecated).
	 * ES: El DA de config de EventHandler activo. Si esta vacio, hace fallback a discovery de AssetRegistry (deprecated).
	 */
	UPROPERTY(config, EditAnywhere, Category = "EventHandler",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXEventHandlerConfig> ActiveConfig;

	/**
	 * EN: Enable verbose logging for config resolution.
	 * ES: Habilitar logging verboso para resolucion de config.
	 */
	UPROPERTY(config, EditAnywhere, Category = "EventHandler|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;
};
