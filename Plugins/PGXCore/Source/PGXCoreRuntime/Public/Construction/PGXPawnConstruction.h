// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Construction/PGXClassConstruction.h"
#include "PGXPawnConstruction.generated.h"

/**
 * EN: Construction DA for Pawn. Minimal — pawns are simpler than characters.
 *     Component injection and system configs are inherited from base.
 * ES: DA de construccion para Pawn. Minimal — los pawns son mas simples que characters.
 *     Inyeccion de componentes y configs de sistema se heredan de la base.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXPawnConstruction : public UPGXClassConstruction
{
	GENERATED_BODY()

public:
	/** EN: Whether this pawn uses default movement / ES: Si este pawn usa movimiento por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|Pawn")
	bool bUseDefaultMovement = true;
};
