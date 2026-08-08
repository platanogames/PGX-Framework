// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Construction/PGXClassConstruction.h"
#include "PGXActorConstruction.generated.h"

/**
 * EN: Construction DA for APGXActorBase. Enables PGX integration for generic actors.
 *     All fields inherited from UPGXClassConstruction — Components, Save, Bridge, Tags.
 * ES: DA de construccion para APGXActorBase. Habilita integracion PGX para actores genericos.
 *     Todos los campos heredados de UPGXClassConstruction.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXActorConstruction : public UPGXClassConstruction
{
	GENERATED_BODY()
};
