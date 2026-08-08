// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXGameFlowTypes.h"
#include "PGXGameFlowConfig.generated.h"

/**
 * EN: Global config DataAsset for the PGX GameFlow system.
 *     Defines context tag, initial channel states, history depth, and logging behavior.
 *     Auto-discovered by UPGXGameFlowSubsystem via AssetRegistry at Initialize().
 *
 * ES: Config DataAsset global para el sistema PGX GameFlow.
 *     Define tag de contexto, estados iniciales por canal, profundidad de historial y comportamiento de logging.
 *     Auto-descubierto por UPGXGameFlowSubsystem via AssetRegistry en Initialize().
 */
UCLASS(BlueprintType)
class PGXGAMEFLOWRUNTIME_API UPGXGameFlowConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/**
	 * EN: Context tag that identifies this config (e.g. "GameFlow.Config.Default").
	 *     Used for discovery and duplicate detection.
	 * ES: Tag de contexto que identifica este config (e.g. "GameFlow.Config.Default").
	 *     Usado para descubrimiento y deteccion de duplicados.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow")
	FGameplayTag ContextTag;

	/**
	 * EN: Initial state tags per channel. Applied at subsystem initialization.
	 *     Channels not listed here start with an invalid (empty) tag.
	 * ES: Tags de estado inicial por canal. Aplicados en la inicializacion del subsistema.
	 *     Canales no listados aqui inician con un tag invalido (vacio).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow")
	TMap<EPGXFlowChannel, FGameplayTag> InitialChannelStates;

	/**
	 * EN: Maximum entries in each channel's history array.
	 *     Oldest entries are trimmed when this limit is exceeded.
	 * ES: Maximo de entradas en el array de historial de cada canal.
	 *     Las entradas mas antiguas se eliminan cuando se excede este limite.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow", meta = (ClampMin = "1", ClampMax = "1000"))
	int32 MaxHistoryDepth = 50;

	/** EN: Log all state transitions to output log / ES: Registrar todas las transiciones de estado en el log de salida */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow")
	bool bLogTransitions = true;

	/**
	 * EN: Allow dev/editor console commands that mutate GameFlow state.
	 *     Shipping builds always reject console mutation regardless of this value.
	 * ES: Permite comandos de consola dev/editor que mutan estado GameFlow.
	 *     Builds Shipping siempre rechazan mutacion por consola aunque este valor sea true.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow|Console")
	bool bAllowConsoleMutations = false;

	/**
	 * EN: Deterministic duplicate-rules policy. HighestPriorityWins is the production default.
	 * ES: Politica deterministica para reglas duplicadas. HighestPriorityWins es el default de produccion.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow|Rules")
	EPGXFlowDuplicateRulesPolicy DuplicateRulesPolicy = EPGXFlowDuplicateRulesPolicy::HighestPriorityWins;

	/** EN: Enable verbose debug logging (validation details, rule lookups) / ES: Habilitar logging verbose de debug (detalles de validacion, busqueda de reglas) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow")
	bool bVerboseDebug = false;
};
