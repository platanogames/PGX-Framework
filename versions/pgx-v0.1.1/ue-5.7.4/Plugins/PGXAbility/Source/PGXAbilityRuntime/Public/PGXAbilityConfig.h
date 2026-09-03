// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXAbilityConfig.generated.h"

/**
 * EN: Config DataAsset for the PGX Ability system.
 *     Defines cooldown policy, attribute clamp/regen defaults, and effect stacking policy.
 *
 * ES: Config DataAsset para el sistema de Ability PGX.
 *     Define politica de cooldown, defaults de clamp/regen de atributos, y politica de
 *     stacking de efectos.
 */
UCLASS(BlueprintType)
class PGXABILITYRUNTIME_API UPGXAbilityConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: Minimum seconds between cooldown re-checks (avoids per-frame polling cost). / ES: Segundos minimos entre rechequeos de cooldown. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Ability", meta = (ClampMin = "0.0"))
	float CooldownPollIntervalSeconds = 0.1f;

	/** EN: Global floor applied to every cooldown duration, regardless of ability-level reduction. / ES: Suelo global aplicado a cada duracion de cooldown. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Ability", meta = (ClampMin = "0.0"))
	float GlobalCooldownFloorSeconds = 0.0f;

	/** EN: Default minimum clamp applied to attributes that do not author their own bounds. / ES: Clamp minimo default para atributos sin bounds propios. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Ability|Attribute")
	float DefaultAttributeClampMin = 0.0f;

	/** EN: Default maximum clamp applied to attributes that do not author their own bounds. / ES: Clamp maximo default para atributos sin bounds propios. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Ability|Attribute")
	float DefaultAttributeClampMax = 100.0f;

	/** EN: Minimum absolute attribute delta that triggers a PGX.Attribute.Changed broadcast (avoids message-bus spam from tiny regen ticks). / ES: Delta minimo absoluto de atributo que dispara PGX.Attribute.Changed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Ability|Attribute", meta = (ClampMin = "0.0"))
	float AttributeChangeSignificanceThreshold = 0.01f;

	/** EN: Maximum number of stacks a single effect tag may accumulate before further applications are rejected. 0 = unbounded. / ES: Maximo de stacks por tag de efecto. 0 = sin limite. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Ability|Effect", meta = (ClampMin = "0"))
	int32 DefaultMaxEffectStacks = 1;

	/** EN: Bounded history length for the Inspector's recent-effects view. / ES: Longitud acotada de historial para la vista de efectos recientes del Inspector. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Ability|Debug", meta = (ClampMin = "1", ClampMax = "1024"))
	int32 InspectorEffectHistoryLimit = 64;

	/** EN: Enable verbose logging for ability/attribute/effect lifecycle events. / ES: Habilitar logging verboso para eventos de ciclo de vida de ability/atributo/efecto. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Ability|Debug")
	bool bEnableVerboseAbilityLogging = false;
};
