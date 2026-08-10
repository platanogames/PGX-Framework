// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Construction/PGXClassConstruction.h"
#include "PGXPlayerStateConstruction.generated.h"

/**
 * EN: Construction DA for PlayerState. Defines initial player tags
 *     applied when a player joins.
 * ES: DA de construccion para PlayerState. Define tags de jugador iniciales
 *     aplicados cuando un jugador se une.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXPlayerStateConstruction : public UPGXClassConstruction
{
	GENERATED_BODY()

public:
	/** EN: Initial player tags / ES: Tags iniciales del jugador */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|PlayerState")
	FGameplayTagContainer InitialPlayerTags;
};
