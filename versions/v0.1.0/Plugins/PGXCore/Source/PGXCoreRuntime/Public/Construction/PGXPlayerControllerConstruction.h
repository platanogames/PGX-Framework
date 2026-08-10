// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Construction/PGXClassConstruction.h"
#include "PGXPlayerControllerConstruction.generated.h"

/**
 * EN: Construction DA for PlayerController. Defines input config, camera config,
 *     and mouse cursor settings.
 * ES: DA de construccion para PlayerController. Define config de input, config de
 *     camara y ajustes de cursor del mouse.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXPlayerControllerConstruction : public UPGXClassConstruction
{
	GENERATED_BODY()

public:
	/** EN: Input configuration asset / ES: Asset de configuracion de input */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|PlayerController")
	TSoftObjectPtr<UPGXConfigDataAsset> InputConfig;

	/** EN: Camera configuration asset / ES: Asset de configuracion de camara */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|PlayerController")
	TSoftObjectPtr<UPGXConfigDataAsset> CameraConfig;

	/** EN: Whether to show the mouse cursor by default / ES: Si mostrar el cursor del mouse por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|PlayerController")
	bool bShowMouseCursor = false;
};
