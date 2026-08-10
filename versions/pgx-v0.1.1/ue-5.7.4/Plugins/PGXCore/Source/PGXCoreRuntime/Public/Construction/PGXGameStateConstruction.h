// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Construction/PGXClassConstruction.h"
#include "PGXGameStateConstruction.generated.h"

/**
 * EN: Construction DA for GameState. Defines initial state tags applied
 *     when the game state is constructed.
 * ES: DA de construccion para GameState. Define tags de estado iniciales
 *     aplicados cuando se construye el game state.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXGameStateConstruction : public UPGXClassConstruction
{
	GENERATED_BODY()

public:
	/** EN: Initial state tags applied to the game state / ES: Tags de estado iniciales */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|GameState")
	FGameplayTagContainer InitialStateTags;
};
