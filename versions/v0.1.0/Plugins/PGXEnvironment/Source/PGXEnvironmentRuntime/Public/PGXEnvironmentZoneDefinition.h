// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXObjectDataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXEnvironmentTypes.h"
#include "PGXEnvironmentZoneDefinition.generated.h"

class UPGXEnvironmentVariable;

/**
 * EN: Authored seed value for one variable inside a zone definition. The
 *     subsystem applies these seeds when the zone first registers (public API
 *     the public API zone registration pipeline step "apply authored initial variable
 *     profile"). InitialValue is clamped at install time against the
 *     variable's authored ClampMin / ClampMax.
 * ES: Valor seed authoring para una variable dentro de una definicion de
 *     zona. El subsistema aplica estos seeds cuando la zona se registra
 *     primero (the current product scope pipeline de registro de zona paso "apply
 *     authored initial variable profile"). InitialValue se clampea al
 *     install contra ClampMin / ClampMax authoring de la variable.
 */
USTRUCT(BlueprintType)
struct PGXENVIRONMENTRUNTIME_API FPGXEnvironmentVariableSeed
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|ZoneSeed")
	TSoftObjectPtr<UPGXEnvironmentVariable> Variable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|ZoneSeed")
	float InitialValueOverride = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|ZoneSeed")
	bool bUseInitialValueOverride = false;
};

/**
 * EN: Object DA defining one zone's authoring profile — its identity tag,
 *     a list of variable seeds applied at registration time, and a default
 *     severity assumed before any threshold transition fires. Project
 *     content authors derived assets per scenario (e.g. DA_Zone_Cave_Damp,
 *     DA_Zone_Bunker_Sealed) under the developer-extensible
 *     PGX.Environment.Zone.* tag branch.
 *
 *     Baseline scope keeps the definition flat (no linked-zone graph here);
 *     propagation references are outside the current product boundary
 *     the current product scope.
 *
 * ES: Object DA que define el perfil authoring de una zona — su tag de
 *     identidad, una lista de seeds de variable aplicados al registro y
 *     una severidad default asumida antes de que dispare cualquier
 *     transicion de umbral. El contenido del proyecto authoring assets
 *     derivados por escenario (ej. DA_Zone_Cave_Damp, DA_Zone_Bunker_Sealed)
 *     bajo la rama developer-extensible PGX.Environment.Zone.* tag.
 *
 *     El scope baseline mantiene la definicion plana (sin graph de
 *     linked-zone aqui); las referencias the public API public API de propagacion
 *     diferidas per current product boundary.
 */
UCLASS(BlueprintType)
class PGXENVIRONMENTRUNTIME_API UPGXEnvironmentZoneDefinition : public UPGXObjectDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: Stable identity tag for this zone definition. / ES: Tag de identidad estable para esta definicion de zona. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Zone",
		meta = (Categories = "PGX.Environment.Zone"))
	FGameplayTag ZoneTag;

	/** EN: Authoring seeds applied at zone registration. / ES: Seeds authoring aplicados al registro de zona. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Zone")
	TArray<FPGXEnvironmentVariableSeed> VariableSeeds;

	/** EN: Default severity before any threshold transition fires. / ES: Severidad default antes de que dispare cualquier transicion de umbral. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Zone")
	EPGXEnvironmentSeverity DefaultSeverity = EPGXEnvironmentSeverity::None;

	// EN: A free-form authoring description (FText Description) is inherited
	//     from UPGXObjectDataAsset (PGXCoreRuntime/Public/Data/PGXObjectDataAsset.h:34),
	//     which already declares it as EditDefaultsOnly + BlueprintReadOnly +
	//     Category = "PGX|ObjectData" + meta=(MultiLine = true). UHT rejects
	//     shadowing parent UPROPERTY members (`shadowing is not allowed`),
	//     so this baseline does NOT redeclare it locally — Zone authors edit
	//     the inherited Description field. runtime-safety
	//     (known engine constraint).
	// ES: Una descripcion authoring libre (FText Description) se hereda de
	//     UPGXObjectDataAsset (PGXCoreRuntime/Public/Data/PGXObjectDataAsset.h:34),
	//     que ya la declara como EditDefaultsOnly + BlueprintReadOnly +
	//     Category = "PGX|ObjectData" + meta=(MultiLine = true). UHT rechaza
	//     shadowing de miembros UPROPERTY del padre (`shadowing is not
	//     allowed`), asi que este baseline NO la re-declara localmente —
	//     los autores de Zone editan el campo Description heredado. runtime-safety
	//     (known engine constraint).
};
