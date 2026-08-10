// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXObjectDataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXEnvironmentTypes.h"
#include "PGXEnvironmentVariable.generated.h"

/**
 * EN: Object DA defining a single environmental variable. Authored
 *     identity (VariableTag), runtime kind (Continuous / Discrete /
 *     Boolean), human-readable unit string for inspector display,
 *     default initial value, and clamp bounds the runtime applies to
 *     modifier deltas.
 *
 *     Authoring Guide authoring invariant 1: NO hardcoded values in source —
 *     project content authors derived assets to define the project's
 *     variable taxonomy (e.g. DA_Variable_Temperature, _Oxygen, _Radiation)
 *     under the developer-extensible PGX.Environment.Variable.* tag branch.
 *
 * ES: Object DA que define una variable ambiental individual. Identidad
 *     authoring (VariableTag), kind runtime (Continuous / Discrete /
 *     Boolean), string de unidad legible para display de inspector, valor
 *     inicial default y bounds clamp que el runtime aplica a deltas de
 *     modificador.
 *
 *     Invariante authoring invariant 1: SIN valores hardcoded en source — el contenido
 *     del proyecto authoring assets derivados para definir la taxonomia
 *     de variables del proyecto (ej. DA_Variable_Temperature, _Oxygen,
 *     _Radiation) bajo la rama developer-extensible
 *     PGX.Environment.Variable.* tag.
 */
UCLASS(BlueprintType)
class PGXENVIRONMENTRUNTIME_API UPGXEnvironmentVariable : public UPGXObjectDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: Stable identity tag for this variable. / ES: Tag de identidad estable para esta variable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Variable",
		meta = (Categories = "PGX.Environment.Variable"))
	FGameplayTag VariableTag;

	/** EN: Variable kind — drives interpretation. / ES: Kind de variable — guia interpretacion. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Variable")
	EPGXEnvironmentVariableKind Kind = EPGXEnvironmentVariableKind::Continuous;

	/** EN: Human-readable unit string for inspector display (e.g. "°C", "%", "rad/s"). / ES: String de unidad legible para display de inspector. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Variable")
	FString DisplayUnit;

	/** EN: Initial value when a zone first registers. / ES: Valor inicial cuando una zona se registra. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Variable")
	float InitialValue = 0.0f;

	/** EN: Minimum clamp applied to modifier-driven updates. / ES: Clamp minimo aplicado a updates por modificador. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Variable")
	float ClampMin = 0.0f;

	/** EN: Maximum clamp applied to modifier-driven updates. / ES: Clamp maximo aplicado a updates por modificador. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Variable")
	float ClampMax = 1.0f;

	/**
	 * EN: Threshold bands authored against this variable (in ascending
	 *     LowerBoundInclusive order). Empty = no severity emit; runtime
	 *     evaluates transitions only when bands are present.
	 * ES: Bandas de umbral authoring contra esta variable (en orden
	 *     ascendente de LowerBoundInclusive). Vacio = sin emit de
	 *     severidad; el runtime evalua transiciones solo cuando hay bandas.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Variable|Thresholds")
	TArray<FPGXEnvironmentThresholdBand> ThresholdBands;
};
