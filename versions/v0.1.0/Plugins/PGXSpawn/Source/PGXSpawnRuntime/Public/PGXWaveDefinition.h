// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXDataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXWaveDefinition.generated.h"

/**
 * EN: Data asset defining a spawn wave configuration.
 * ES: Data asset que define la configuracion de una oleada de spawn.
 */
UCLASS(BlueprintType)
class PGXSPAWNRUNTIME_API UPGXWaveDefinition : public UPGXDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	static const FName SchemaVersion;

	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: Unique name identifier for this wave. / ES: Nombre identificador unico para esta oleada. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	FName WaveName;

	/** EN: GameplayTag used by CancelWave to look up and cancel this wave. / ES: GameplayTag usado por CancelWave para buscar y cancelar esta oleada. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn", meta = (Categories = "PGX.Spawn.Wave"))
	FGameplayTag WaveTag;

	/** EN: Default class to spawn for each wave spawn request. / ES: Clase por defecto a spawnear para cada peticion de spawn de la oleada. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<AActor> DefaultSpawnClass;

	/** EN: Total number of actors to spawn in this wave. / ES: Numero total de actores a spawnear en esta oleada. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	int32 TotalSpawnCount = 10;

	/** EN: Time interval in seconds between individual spawns. / ES: Intervalo de tiempo en segundos entre spawns individuales. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	float SpawnInterval = 1.0f;
};
