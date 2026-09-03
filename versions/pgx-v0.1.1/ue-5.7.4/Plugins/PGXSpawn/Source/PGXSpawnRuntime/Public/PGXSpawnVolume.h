// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGXSpawnVolume.generated.h"

/**
 * EN: Spawn volume that spawns actors at random positions within its bounds.
 * ES: Volumen de spawn que spawnea actores en posiciones aleatorias dentro de sus limites.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXSPAWNRUNTIME_API APGXSpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	APGXSpawnVolume();

	/** EN: The actor class to spawn inside this volume. / ES: La clase de actor a spawnear dentro de este volumen. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<AActor> SpawnClass;

	/** EN: Maximum number of spawns allowed within this volume. / ES: Numero maximo de spawns permitidos dentro de este volumen. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 MaxSpawnsInVolume = 5;
};
