// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXDataAsset.h"
#include "PGXConfigDataAsset.generated.h"

/**
 * EN: Base class for all PGX Config DataAssets.
 *     Config DAs define framework/plugin configuration and are loaded at startup.
 * ES: Clase base para todos los Config DataAssets de PGX.
 *     Los Config DAs definen configuracion del framework/plugin y se cargan al inicio.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXConfigDataAsset : public UPGXDataAsset
{
	GENERATED_BODY()

public:
	/** EN: Display name for this config / ES: Nombre para mostrar de esta configuracion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Config")
	FText ConfigDisplayName;

	/** EN: Description of what this config controls / ES: Descripcion de lo que controla esta config */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Config", meta = (MultiLine = true, AdvancedDisplay))
	FText ConfigDescription;
};
