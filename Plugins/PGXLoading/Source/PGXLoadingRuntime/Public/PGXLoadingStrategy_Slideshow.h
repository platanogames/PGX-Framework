// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXLoadingStrategyBase.h"
#include "PGXLoadingStrategy_Slideshow.generated.h"

class UTexture2D;

/**
 * EN: Slideshow loading strategy — rotates through BackgroundImages with crossfade.
 *     Loads current + next image only (never all at once). Releases previous after crossfade.
 *     Falls back to Image strategy behavior if only one image configured.
 *
 * ES: Estrategia de slideshow — rota entre BackgroundImages con crossfade.
 *     Carga solo imagen actual + siguiente (nunca todas). Libera la anterior tras crossfade.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXLoadingStrategy_Slideshow : public UPGXLoadingStrategyBase
{
	GENERATED_BODY()

public:
	void InitializeStrategy(UPGXLoadingProfile* Profile) override;
	void Activate(UPGXLoadingWidget* Widget) override;
	void UpdateVisual(float DeltaTime) override;
	void Deactivate() override;
	FText GetCurrentTip() const override;

private:
	void LoadImageAtIndex(int32 Index);
	void OnFirstImageLoaded();
	void OnNextImageLoaded();
	void AdvanceSlide();

	UPROPERTY()
	TObjectPtr<UTexture2D> CurrentTexture;

	UPROPERTY()
	TObjectPtr<UTexture2D> NextTexture;

	TSharedPtr<FStreamableHandle> NextLoadHandle;

	int32 CurrentImageIndex = 0;
	int32 CurrentTipIndex = 0;
	float SlideTimer = 0.0f;
	float TipTimer = 0.0f;
};
