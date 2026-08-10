// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Construction/PGXClassConstruction.h"
#include "Construction/PGXConstructionTypes.h"
#include "PGXHUDConstruction.generated.h"

/**
 * EN: Construction DA for HUD. Defines the main HUD widget class and
 *     additional HUD layers to create during construction.
 * ES: DA de construccion para HUD. Define la clase de widget principal del HUD
 *     y capas adicionales de HUD a crear durante la construccion.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXHUDConstruction : public UPGXClassConstruction
{
	GENERATED_BODY()

public:
	UPGXHUDConstruction();

	/** EN: Main HUD widget class / ES: Clase de widget principal del HUD */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|HUD")
	TSoftClassPtr<UUserWidget> MainHUDWidgetClass;

	/** EN: Additional HUD layers to create / ES: Capas adicionales de HUD a crear */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|HUD")
	TArray<FPGXHUDLayerEntry> HUDLayers;

	// ─── GameFlow Visibility ───

	/** EN: Map of GameFlow state -> visible HUD layer tags. Default: all visible / ES: Mapa estado GameFlow -> tags de capas visibles */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|HUD|Visibility")
	TMap<FGameplayTag, FGameplayTagContainer> GameFlowVisibilityMap;
};
