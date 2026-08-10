// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Construction/PGXClassConstruction.h"
#include "PGXCharacterConstruction.generated.h"

/**
 * EN: Construction DA for Character. Defines movement defaults and
 *     optional ability-system config slot.
 * ES: DA de construccion para Character. Define valores de movimiento
 *     por defecto y slot opcional de config del sistema de habilidades.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXCharacterConstruction : public UPGXClassConstruction
{
	GENERATED_BODY()

public:
	/** EN: Optional ability-system config / ES: Config opcional del sistema de habilidades */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|Character")
	TSoftObjectPtr<UPGXConfigDataAsset> AbilityConfig;

	/** EN: Default walking speed / ES: Velocidad de caminata por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|Character", meta = (ClampMin = "0.0"))
	float DefaultWalkSpeed = 600.0f;

	/** EN: Default running speed / ES: Velocidad de carrera por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|Character", meta = (ClampMin = "0.0"))
	float DefaultRunSpeed = 1200.0f;
};
