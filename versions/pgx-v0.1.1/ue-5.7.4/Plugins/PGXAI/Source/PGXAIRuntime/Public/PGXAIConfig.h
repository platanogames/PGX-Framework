// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXAIConfig.generated.h"

/**
 * EN: Config DataAsset for the PGX AI system.
 *     Defines perception defaults, squad limits, and debug settings.
 *
 * ES: Config DataAsset para el sistema de IA PGX.
 *     Define percepcion por defecto, limites de escuadron y ajustes de debug.
 */
UCLASS(BlueprintType)
class PGXAIRUNTIME_API UPGXAIConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: Interval in seconds between perception updates / ES: Intervalo en segundos entre actualizaciones de percepcion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI", meta = (ClampMin = "0.05"))
	float PerceptionUpdateInterval = 0.2f;

	/** EN: Maximum number of members per squad / ES: Numero maximo de miembros por escuadron */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI", meta = (ClampMin = "1", ClampMax = "64"))
	int32 MaxSquadSize = 8;

	/** EN: Default sight perception radius / ES: Radio de percepcion visual por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI", meta = (ClampMin = "0.0"))
	float DefaultSightRadius = 2000.0f;

	/** EN: Default hearing perception radius / ES: Radio de percepcion auditiva por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI", meta = (ClampMin = "0.0"))
	float DefaultHearingRadius = 1500.0f;

	/** EN: Tick interval for behavior tree updates in seconds / ES: Intervalo de tick para actualizaciones de behavior tree en segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI", meta = (ClampMin = "0.016"))
	float BehaviorTreeTickInterval = 0.1f;

	/** EN: Enable AI debug drawing in development builds / ES: Habilitar dibujo de debug de IA en builds de desarrollo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI")
	bool bEnableDebugDrawAI = false;

	// ========================================================================
	// EN: Alert / Detection Policy. These authored values define the alert-state boundary.
	// ES: Politica de Alerta / Deteccion. Estos valores definen el limite del estado de alerta.
	// ========================================================================

	/** EN: How fast the alert level decays per second when no stimuli are received. 0 = no decay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI|Alert", meta = (ClampMin = "0.0"))
	float AlertDecayPerSecond = 0.1f;

	/** EN: When true, agents whose alert level reaches zero suspend ticking until a new stimulus
	 *      arrives (lets distant/inactive squads sleep without losing their state). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI|Alert")
	bool bAllowSleepSuspension = true;

	/** EN: Stimulus confidence threshold above which an agent transitions from Calm to
	 *      Investigating. Range [0,1]; 0 = transition on any stimulus, 1 = never transition. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI|Alert",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InvestigateConfidenceThreshold = 0.3f;

	/** EN: Stimulus confidence threshold above which an agent transitions to Combat. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|AI|Alert",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CombatConfidenceThreshold = 0.7f;
};
