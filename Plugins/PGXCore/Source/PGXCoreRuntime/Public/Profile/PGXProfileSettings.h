// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "PGXProfileSettings.generated.h"

class UPGXProjectProfileConfig;

// ============================================================================
// EN: Profile System settings — appears in Project Settings > PGX > Profile System.
//     Controls which profile DA is active and resolution behavior.
//
// ES: Settings del Sistema de Profile — aparece en Project Settings > PGX > Profile System.
//     Controla cual DA de profile esta activo y el comportamiento de resolucion.
// ============================================================================

UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Profile System"))
class PGXCORERUNTIME_API UPGXProfileSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	/**
	 * EN: The active Project Profile config asset. If empty, safe defaults are used.
	 * ES: El config asset de Project Profile activo. Si esta vacio, se usan defaults seguros.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Profile",
		meta = (AllowedClasses = "/Script/PGXCoreRuntime.PGXProjectProfileConfig"))
	TSoftObjectPtr<UPGXProjectProfileConfig> ActiveProfile;

	/**
	 * EN: When no profile DA is assigned, use safe permissive defaults (all allowed, no budgets).
	 *     If false and no profile is assigned, the subsystem will log a warning and remain unresolved.
	 * ES: Cuando no hay DA de profile asignado, usar defaults permisivos seguros.
	 *     Si es false y no hay profile, el subsistema logueara un warning y permanecera sin resolver.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Profile")
	bool bUseSafeDefaultsWhenNoProfile = true;

	/**
	 * EN: Enable verbose logging during profile resolution. Useful for debugging override application.
	 * ES: Habilitar logging verboso durante la resolucion del profile. Util para debuggear aplicacion de overrides.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Profile|Debug")
	bool bVerboseProfileResolution = false;
};
