// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXLoadingStrategyBase.h"
#include "PGXLoadingStrategy_Image.generated.h"

class UTexture2D;

/**
 * EN: Static image loading strategy — loads BackgroundImages[0] via StreamableManager
 *     and sets it on the widget's IMG_Background. Falls back to Minimal if no images configured
 *     or if asset load fails.
 *
 * ES: Estrategia de imagen estatica — carga BackgroundImages[0] via StreamableManager
 *     y la establece en IMG_Background del widget. Fallback a Minimal si no hay imagenes
 *     o si la carga falla.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXLoadingStrategy_Image : public UPGXLoadingStrategyBase
{
	GENERATED_BODY()

public:
	void InitializeStrategy(UPGXLoadingProfile* Profile) override;
	void Activate(UPGXLoadingWidget* Widget) override;
	void Deactivate() override;
	FText GetCurrentTip() const override;

private:
	void OnImageLoaded();

	UPROPERTY()
	TObjectPtr<UTexture2D> LoadedTexture;

	int32 CurrentTipIndex = 0;
};
