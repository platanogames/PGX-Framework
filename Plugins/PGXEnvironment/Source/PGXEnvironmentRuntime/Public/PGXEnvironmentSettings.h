// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "PGXEnvironmentSettings.generated.h"

class UPGXEnvironmentConfig;

/**
 * Environment settings exposed under Project Settings > PGX > Environment.
 * ActiveConfig is resolved once during subsystem initialization. An empty or
 * unloadable reference leaves the subsystem active without authored zones.
 * Asset Registry discovery and cross-plugin bridges are not included.
 */

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Environment System"))
class PGXENVIRONMENTRUNTIME_API UPGXEnvironmentSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	FName GetSectionName() const override { return TEXT("Environment"); }

	/** Active environment configuration. No Asset Registry fallback is performed. */
	UPROPERTY(config, EditAnywhere, Category = "Environment",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXEnvironmentConfig> ActiveConfig;

	/**
	 * EN: Enable verbose logging for config resolution + zone register/
	 *     threshold-transition events. Off by default to keep logs quiet
	 *     in shipping.
	 * ES: Habilitar logging verboso para resolucion de config + eventos de
	 *     zona register / transicion de umbral. Off por defecto para
	 *     mantener logs silenciosos en shipping.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Environment|Diagnostics",
		meta = (DisplayName = "Verbose Environment Debug"))
	bool bVerboseEnvironmentDebug = false;
};
