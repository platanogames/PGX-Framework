// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXColonyConfig.generated.h"

/**
 * Authored Colony configuration values for capacity, role weighting, morale
 * and dormancy. The current survivor registry does not consume this asset yet.
 */
UCLASS(BlueprintType)
class PGXCOLONYRUNTIME_API UPGXColonyConfig : public UPGXConfigDataAsset, public IPGXObservable
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

	// ========================================================================
	// EN: Capacity Policy (Development Preview shape)
	// ES: Politica de Capacidad (Development Preview shape)
	// ========================================================================

	/** Maximum authored survivor capacity for a settlement. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Colony|Capacity",
		meta = (ClampMin = "1", ClampMax = "256"))
	int32 MaxSurvivorsPerSettlement = 32;

	/** Default authored role-distribution weight for the Worker role. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Colony|Capacity",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultWorkerRoleWeight = 0.6f;

	// ========================================================================
	// Authored morale policy. Runtime simulation is not included.
	// ========================================================================

	/** EN: How fast morale decays per second when basic needs are unmet. 0 = no decay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Colony|Morale",
		meta = (ClampMin = "0.0"))
	float DefaultMoraleDecayRate = 0.05f;

	/** EN: Morale value below which the Conflict event policy may fire. Range [0,1]. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Colony|Morale",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ConflictMoraleThreshold = 0.25f;

	// ========================================================================
	// EN: Remote Simulation / Dormancy (configuration shape only; runtime simulation is not included)
	// ES: Simulacion Remota / Dormancy (remote simulation diferido)
	// ========================================================================

	/** EN: Wall-clock seconds of inactivity after which a remote settlement enters dormant tick. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Colony|Dormancy",
		meta = (ClampMin = "5.0"))
	float DormancyThresholdSeconds = 60.0f;

	/** EN: When true, dormant settlements queue events for catch-up replay on next observation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Colony|Dormancy")
	bool bQueueDormantEventsForCatchUp = true;
};
