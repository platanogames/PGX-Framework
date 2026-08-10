// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXDataAsset.h"
#include "GameplayTagContainer.h"
#include "PGXObjectDataAsset.generated.h"

/**
 * EN: Base class for all PGX Object DataAssets.
 *     Object DAs define gameplay entities (props, weapons, characters, etc.)
 *     and support deferred loading via the Data Registry.
 * ES: Clase base para todos los Object DataAssets de PGX.
 *     Los Object DAs definen entidades de gameplay (props, armas, personajes, etc.)
 *     y soportan carga diferida via el Data Registry.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXObjectDataAsset : public UPGXDataAsset
{
	GENERATED_BODY()

public:
	/** EN: Gameplay tag identifying the category / ES: Tag de gameplay que identifica la categoria */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|ObjectData")
	FGameplayTag CategoryTag;

	/** EN: Display name for this object / ES: Nombre para mostrar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|ObjectData")
	FText DisplayName;

	/** EN: Description of this object / ES: Descripcion del objeto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|ObjectData", meta = (MultiLine = true))
	FText Description;
};
