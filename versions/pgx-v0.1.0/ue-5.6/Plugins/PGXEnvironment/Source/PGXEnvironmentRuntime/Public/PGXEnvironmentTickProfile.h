// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXObjectDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXEnvironmentTickProfile.generated.h"

/**
 * EN: Object DA describing how zones tick. TickHz, dormancy threshold and catch-up
 *     budget are stored as authoring data, but the runtime does not include a tick
 *     scheduler that consumes them.
 *
 *     Authoring invariants 1 + 4 honored: data-driven values
 *     with ClampMin / ClampMax metadata, no hardcoded defaults in the
 *     consuming subsystem.
 *
 * ES: Object DA que describe como actualizan las zonas. TickHz, el umbral de dormancia
 *     y el presupuesto de recuperacion se almacenan como datos de autoria, pero runtime
 *     no incluye un planificador que los consuma.
 *
 *     Invariantes de authoring 1 + 4 honored: valores data-driven con metadata
 *     ClampMin / ClampMax, sin defaults hardcoded en el subsistema
 *     consumer.
 */
UCLASS(BlueprintType)
class PGXENVIRONMENTRUNTIME_API UPGXEnvironmentTickProfile : public UPGXObjectDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: Authored tick rate for active zones (Hz). / ES: Tick rate authoring para zonas activas (Hz). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|TickProfile",
		meta = (ClampMin = "0.1", ClampMax = "120.0"))
	float ActiveTickHz = 5.0f;

	/** EN: Authored tick rate when a zone is dormant (Hz). 0 = no ticks while dormant. / ES: Tick rate authoring cuando una zona esta dormant (Hz). 0 = sin ticks dormant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|TickProfile",
		meta = (ClampMin = "0.0", ClampMax = "120.0"))
	float DormantTickHz = 0.0f;

	/**
	 * EN: Seconds of inactivity before a zone enters dormancy. 0 disables
	 *     the dormancy transition entirely.
	 * ES: Segundos de inactividad antes de que una zona entre dormancy.
	 *     0 deshabilita la transicion de dormancy completamente.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|TickProfile",
		meta = (ClampMin = "0.0", ClampMax = "3600.0"))
	float DormancyAfterSeconds = 0.0f;

	/**
	 * EN: Maximum simulated seconds the catch-up runtime is allowed to
	 *     resolve when a zone wakes from dormancy. Bound prevents huge
	 *     wake-up frame spikes.
	 * ES: Maximos segundos simulados que el catch-up runtime permite
	 *     resolver cuando una zona despierta de dormancy. El bound
	 *     previene picos enormes de frame en wake-up.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|TickProfile",
		meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float MaxCatchUpSeconds = 1.0f;
};
