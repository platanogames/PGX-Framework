// Copyright PGX Framework. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXAbilitySettings.generated.h"

class UPGXAbilityConfig;

/**
 * EN: Project Settings for the PGX Ability system. Settings-first canonical resolution path,
 *     consistent with the PGX-wide convention (UPGXAISettings / UPGXEnvironmentSettings /
 *     UPGXInputConfig). `ActiveConfig` is the deterministic primary. Accessible in Editor:
 *     Project Settings > PGX > Ability.
 * ES: Project Settings para el sistema PGX Ability. Resolucion Settings-first canonica,
 *     consistente con la convencion PGX-wide. Accesible en Editor: Project Settings > PGX > Ability.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "PGX Ability System"))
class PGXABILITYRUNTIME_API UPGXAbilitySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * EN: The active Ability config DA. Consumed by `UPGXAbilitySubsystem::Initialize` via
	 *     `LoadSynchronous` on this soft-pointer. If empty, the subsystem runs with
	 *     `ActiveConfig=nullptr` and emits a Log at startup; no AssetRegistry scan-fallback.
	 * ES: El DA de config Ability activo. Si esta vacio el subsistema corre con
	 *     ActiveConfig=nullptr y emite Log en startup.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Config", meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXAbilityConfig> ActiveConfig;

	/** EN: Enable verbose logging for config resolution. / ES: Habilitar logging verboso para resolucion de config. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery|Debug", meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;

	FName GetCategoryName() const override { return TEXT("PGX"); }
};
