// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXSpawnTypes.h"
#include "PGXSpawnConfig.generated.h"

/**
 * EN: Config DataAsset for the PGX Spawn system.
 *     Defines spawn limits, wave timing, and pooling behavior.
 *
 * ES: Config DataAsset para el sistema de spawn PGX.
 *     Define limites de spawn, temporizacion de oleadas y comportamiento de pooling.
 */
UCLASS(BlueprintType)
class PGXSPAWNRUNTIME_API UPGXSpawnConfig : public UPGXConfigDataAsset, public IPGXObservable
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

	/** EN: Maximum concurrent spawned actors / ES: Maximo de actores spawneados concurrentes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Spawn", meta = (ClampMin = "1"))
	int32 MaxConcurrentActors = 50;

	/** EN: Default delay between waves in seconds / ES: Retraso por defecto entre oleadas en segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Spawn", meta = (ClampMin = "0.0"))
	float DefaultWaveDelay = 5.0f;

	/** EN: Use object pooling for spawned actors / ES: Usar pooling de objetos para actores spawneados */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Spawn")
	bool bUsePoolingForSpawns = true;

	/** EN: Initial pool size pre-allocated per spawn class. / ES: Tamano inicial del pool pre-asignado por clase de spawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Spawn", meta = (ClampMin = "0", EditCondition = "bUsePoolingForSpawns"))
	int32 ObjectPoolInitialSizePerClass = 4;

	/** EN: Threshold (0.0-1.0) of MaxConcurrentActors that fires OnBudgetWarning. / ES: Umbral (0.0-1.0) de MaxConcurrentActors que dispara OnBudgetWarning. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Spawn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BudgetWarningThresholdPercent = 0.8f;

	/** EN: Conditions applied to every spawn request (condition implementation). / ES: Condiciones aplicadas a cada peticion de spawn (condition implementation). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Spawn", meta = (TitleProperty = "ConditionTag"))
	TArray<FPGXSpawnConditionDefinition> GlobalConditions;

	/** EN: Interval in seconds to check spawn conditions / ES: Intervalo en segundos para verificar condiciones de spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Spawn", meta = (ClampMin = "0.1"))
	float SpawnCheckInterval = 0.5f;

	/** EN: Maximum distance from players to allow spawning / ES: Distancia maxima desde jugadores para permitir spawning */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Spawn", meta = (ClampMin = "0.0"))
	float MaxSpawnDistance = 10000.0f;

	/** EN: Minimum distance from players to spawn (avoids pop-in) / ES: Distancia minima desde jugadores para spawnear (evita pop-in) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Spawn", meta = (ClampMin = "0.0"))
	float MinSpawnDistance = 500.0f;
};
