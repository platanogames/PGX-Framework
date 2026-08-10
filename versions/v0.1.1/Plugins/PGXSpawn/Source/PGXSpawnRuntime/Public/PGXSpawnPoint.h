// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "PGXSpawnTypes.h"
#include "PGXSpawnPoint.generated.h"

/**
 * EN: Configurable spawn point actor. Produces generic spawn requests for the world subsystem.
 * ES: Actor de punto de spawn configurable. Produce peticiones genericas de spawn para el subsystem de mundo.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXSPAWNRUNTIME_API APGXSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APGXSpawnPoint();

	/** EN: The actor class to spawn. / ES: La clase de actor a spawnear. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Spawn")
	TSubclassOf<AActor> SpawnClass;

	/** EN: Optional authored identity/source tag for this point. / ES: Tag opcional de identidad/fuente para este punto. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Spawn", meta = (Categories = "PGX.Spawn.Source"))
	FGameplayTag SpawnPointTag;

	/** EN: Cooldown time in seconds between respawns. / ES: Tiempo de cooldown en segundos entre respawns. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Spawn", meta = (ClampMin = "0.0"))
	float RespawnCooldown = 5.0f;

	/** EN: Maximum number of simultaneously active spawns. / ES: Numero maximo de spawns activos simultaneamente. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Spawn", meta = (ClampMin = "0"))
	int32 MaxActiveSpawns = 1;

	UFUNCTION(BlueprintPure, Category = "PGX|Spawn")
	FPGXSpawnRequest BuildSpawnRequest() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Spawn")
	FPGXSpawnResult ValidateSpawnPointPolicy() const;
};
