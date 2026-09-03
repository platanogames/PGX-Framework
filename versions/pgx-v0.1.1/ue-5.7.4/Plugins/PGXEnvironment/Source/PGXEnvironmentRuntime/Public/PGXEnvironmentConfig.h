// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "PGXEnvironmentConfig.generated.h"

class UPGXEnvironmentVariable;
class UPGXEnvironmentZoneDefinition;
class UPGXEnvironmentTickProfile;

/**
 * Environment configuration asset containing the authored variable taxonomy,
 * zone definitions and an optional default tick profile.
 *
 * Variable and zone entries are resolved on demand. DefaultTickProfile is
 * authoring-only in this preview because propagation scheduling is not included.
 */
UCLASS(BlueprintType)
class PGXENVIRONMENTRUNTIME_API UPGXEnvironmentConfig : public UPGXConfigDataAsset, public IPGXObservable
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
	 * EN: Authored variable taxonomy for the project. Each entry is a
	 *     soft-pointer to a UPGXEnvironmentVariable Object DA so the
	 *     Config DA does not eager-load all variables.
	 * ES: Taxonomia de variables authoring para el proyecto. Cada entrada
	 *     es un soft-pointer a un Object DA UPGXEnvironmentVariable para
	 *     que el Config DA no eager-loadee todas las variables.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Config|Variables")
	TArray<TSoftObjectPtr<UPGXEnvironmentVariable>> Variables;

	/**
	 * EN: Authored zone catalog for the project. Each entry is a
	 *     soft-pointer to a UPGXEnvironmentZoneDefinition Object DA.
	 * ES: Catalogo authoring de zonas para el proyecto. Cada entrada es un
	 *     soft-pointer a un Object DA UPGXEnvironmentZoneDefinition.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Config|Zones")
	TArray<TSoftObjectPtr<UPGXEnvironmentZoneDefinition>> ZoneDefinitions;

	/**
	 * EN: Default tick profile applied to zones that do not specify one.
	 *     Soft-pointer to keep the Config decoupled from the profile DA.
	 *     The current runtime does not consume this value.
	 * ES: Tick profile default aplicado a zonas que no especifican uno.
	 *     Soft-pointer para mantener el Config desacoplado del profile DA.
	 *     El runtime actual no consume este valor.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Config|Tick")
	TSoftObjectPtr<UPGXEnvironmentTickProfile> DefaultTickProfile;
};
