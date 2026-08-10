// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXLoadingStrategyBase.h"
#include "PGXLoadingStrategy_Material.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * EN: Animated material loading strategy — loads BackgroundMaterials[0] and creates a
 *     dynamic material instance for animation. Sets the material on the widget's IMG_Background.
 *     Drives a "Time" scalar parameter for material animation.
 *
 * ES: Estrategia de material animado — carga BackgroundMaterials[0] y crea una instancia
 *     de material dinamico para animacion. Establece el material en IMG_Background del widget.
 *     Controla un parametro escalar "Time" para animacion del material.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXLoadingStrategy_Material : public UPGXLoadingStrategyBase
{
	GENERATED_BODY()

public:
	void InitializeStrategy(UPGXLoadingProfile* Profile) override;
	void Activate(UPGXLoadingWidget* Widget) override;
	void UpdateVisual(float DeltaTime) override;
	void Deactivate() override;
	FText GetCurrentTip() const override;

private:
	void OnMaterialLoaded();

	UPROPERTY()
	TObjectPtr<UMaterialInterface> LoadedMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	float AnimationTime = 0.0f;
	int32 CurrentTipIndex = 0;
	float TipTimer = 0.0f;
};
