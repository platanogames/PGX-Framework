// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXGameFlowTypes.h"
#include "PGXFlowRulesConfig.generated.h"

/**
 * EN: Per-channel rules DataAsset for the PGX GameFlow system.
 *     Contains a TMap of transition rules keyed by origin state tag.
 *     One instance per channel (e.g. DA_GlobalRulesGameFlow, DA_UIRulesGameFlow).
 *     Auto-discovered by UPGXGameFlowSubsystem via AssetRegistry at Initialize().
 *
 * ES: DataAsset de reglas por canal para el sistema PGX GameFlow.
 *     Contiene un TMap de reglas de transicion con key del tag de estado de origen.
 *     Una instancia por canal (e.g. DA_GlobalRulesGameFlow, DA_UIRulesGameFlow).
 *     Auto-descubierto por UPGXGameFlowSubsystem via AssetRegistry en Initialize().
 */
UCLASS(BlueprintType)
class PGXGAMEFLOWRUNTIME_API UPGXFlowRulesConfig : public UPGXConfigDataAsset, public IPGXObservable
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
	 * EN: Which channel these rules apply to.
	 * ES: A que canal aplican estas reglas.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow")
	EPGXFlowChannel Channel = EPGXFlowChannel::Global;

	/**
	 * EN: Deterministic conflict priority when multiple rule configs target the same channel.
	 *     Higher priority wins when the active GameFlow config uses HighestPriorityWins.
	 * ES: Prioridad deterministica de conflicto cuando varias configs apuntan al mismo canal.
	 *     Mayor prioridad gana cuando la config activa usa HighestPriorityWins.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow", meta = (ClampMin = "0"))
	int32 ConflictPriority = 0;

	/**
	 * EN: Transition rules. Key = origin state tag, Value = rule defining allowed/disallowed destinations.
	 *     When a channel is in state X and wants to transition, the subsystem looks up X in this map.
	 *     If no entry exists → permissive (transition allowed by default).
	 * ES: Reglas de transicion. Key = tag de estado origen, Value = regla que define destinos permitidos/prohibidos.
	 *     Cuando un canal esta en estado X y quiere transicionar, el subsistema busca X en este mapa.
	 *     Si no existe entrada → permisivo (transicion permitida por defecto).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow")
	TMap<FGameplayTag, FPGXFlowRule> FlowRules;
};
