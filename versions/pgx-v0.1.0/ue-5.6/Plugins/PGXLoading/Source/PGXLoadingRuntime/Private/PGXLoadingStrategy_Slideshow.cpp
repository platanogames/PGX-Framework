// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingStrategy_Slideshow.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingRuntime.h"
#include "PGXLoadingProfile.h"
#include "PGXLoadingWidget.h"
#include "Engine/Texture2D.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UPGXLoadingStrategy_Slideshow::InitializeStrategy(UPGXLoadingProfile* Profile)
{
	Super::InitializeStrategy(Profile);

	CurrentTexture = nullptr;
	NextTexture = nullptr;
	CurrentImageIndex = 0;
	CurrentTipIndex = 0;
	SlideTimer = 0.0f;
	TipTimer = 0.0f;

	if (!Profile || Profile->BackgroundImages.Num() == 0)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("Strategy_Slideshow: No background images — ready without images."));
		bReady = true;
		return;
	}

	// EN: Load first image / ES: Cargar primera imagen
	LoadImageAtIndex(0);
}

void UPGXLoadingStrategy_Slideshow::LoadImageAtIndex(int32 Index)
{
	if (!CachedProfile || Index < 0 || Index >= CachedProfile->BackgroundImages.Num())
	{
		return;
	}

	const TSoftObjectPtr<UTexture2D>& SoftRef = CachedProfile->BackgroundImages[Index];
	if (SoftRef.IsNull()) return;

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

	if (Index == 0)
	{
		// EN: First image — use main handle / ES: Primera imagen — usar handle principal
		AssetLoadHandle = StreamableManager.RequestAsyncLoad(
			SoftRef.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnFirstImageLoaded),
			FStreamableManager::AsyncLoadHighPriority
		);
	}
	else
	{
		// EN: Next images — use secondary handle / ES: Siguientes — usar handle secundario
		NextLoadHandle = StreamableManager.RequestAsyncLoad(
			SoftRef.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnNextImageLoaded)
		);
	}
}

void UPGXLoadingStrategy_Slideshow::OnFirstImageLoaded()
{
	if (CachedProfile && CachedProfile->BackgroundImages.Num() > 0)
	{
		CurrentTexture = CachedProfile->BackgroundImages[0].Get();
	}

	bReady = true;

	if (bActivated && CachedWidget && CurrentTexture)
	{
		CachedWidget->SetBackgroundImage(CurrentTexture);
	}

	// EN: Pre-load next image / ES: Pre-cargar siguiente imagen
	if (CachedProfile && CachedProfile->BackgroundImages.Num() > 1)
	{
		LoadImageAtIndex(1);
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy_Slideshow: First image loaded."));
}

void UPGXLoadingStrategy_Slideshow::OnNextImageLoaded()
{
	int32 NextIndex = (CurrentImageIndex + 1) % CachedProfile->BackgroundImages.Num();
	NextTexture = CachedProfile->BackgroundImages[NextIndex].Get();

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Slideshow: Next image [%d] pre-loaded."), NextIndex);
}

void UPGXLoadingStrategy_Slideshow::Activate(UPGXLoadingWidget* Widget)
{
	Super::Activate(Widget);

	if (!CachedWidget) return;

	if (CurrentTexture)
	{
		CachedWidget->SetBackgroundImage(CurrentTexture);
	}

	CachedWidget->SetProgressBarVisible(CachedProfile ? CachedProfile->bShowProgressBar : true);
	CachedWidget->SetSkipButtonVisible(CachedProfile ? CachedProfile->bAllowSkip : false);

	if (CachedProfile && CachedProfile->Tips.Num() > 0)
	{
		CachedWidget->SetTipText(CachedProfile->Tips[0]);
		CachedWidget->OnTipChanged(CachedProfile->Tips[0]);
	}

	SlideTimer = 0.0f;
	TipTimer = 0.0f;

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Slideshow: Activated with %d images."),
		CachedProfile ? CachedProfile->BackgroundImages.Num() : 0);
}

void UPGXLoadingStrategy_Slideshow::UpdateVisual(float DeltaTime)
{
	if (!bActivated || !CachedProfile) return;

	// EN: Slide rotation / ES: Rotacion de slides
	if (CachedProfile->BackgroundImages.Num() > 1)
	{
		SlideTimer += DeltaTime;
		if (SlideTimer >= CachedProfile->SlideshowInterval)
		{
			AdvanceSlide();
			SlideTimer = 0.0f;
		}
	}

	// EN: Tip rotation / ES: Rotacion de consejos
	if (CachedProfile->Tips.Num() > 1)
	{
		TipTimer += DeltaTime;
		if (TipTimer >= CachedProfile->TipRotationInterval)
		{
			CurrentTipIndex = (CurrentTipIndex + 1) % CachedProfile->Tips.Num();
			if (CachedWidget)
			{
				CachedWidget->SetTipText(CachedProfile->Tips[CurrentTipIndex]);
				CachedWidget->OnTipChanged(CachedProfile->Tips[CurrentTipIndex]);
			}
			TipTimer = 0.0f;
		}
	}
}

void UPGXLoadingStrategy_Slideshow::AdvanceSlide()
{
	// EN: Release current, promote next / ES: Liberar actual, promover siguiente
	CurrentTexture = NextTexture;
	NextTexture = nullptr;
	CurrentImageIndex = (CurrentImageIndex + 1) % CachedProfile->BackgroundImages.Num();

	if (CachedWidget && CurrentTexture)
	{
		CachedWidget->SetBackgroundImage(CurrentTexture);
	}

	// EN: Pre-load the next image / ES: Pre-cargar siguiente imagen
	int32 NextIndex = (CurrentImageIndex + 1) % CachedProfile->BackgroundImages.Num();
	LoadImageAtIndex(NextIndex);

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Slideshow: Advanced to slide %d."), CurrentImageIndex);
}

void UPGXLoadingStrategy_Slideshow::Deactivate()
{
	if (NextLoadHandle.IsValid())
	{
		NextLoadHandle->CancelHandle();
		NextLoadHandle.Reset();
	}

	CurrentTexture = nullptr;
	NextTexture = nullptr;

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Slideshow: Deactivated."));
	Super::Deactivate();
}

FText UPGXLoadingStrategy_Slideshow::GetCurrentTip() const
{
	if (!CachedProfile || CachedProfile->Tips.Num() == 0)
	{
		return FText::GetEmpty();
	}
	return CachedProfile->Tips[CurrentTipIndex % CachedProfile->Tips.Num()];
}
