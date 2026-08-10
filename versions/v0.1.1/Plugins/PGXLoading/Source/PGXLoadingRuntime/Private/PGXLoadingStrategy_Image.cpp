// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingStrategy_Image.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingRuntime.h"
#include "PGXLoadingProfile.h"
#include "PGXLoadingWidget.h"
#include "Engine/Texture2D.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UPGXLoadingStrategy_Image::InitializeStrategy(UPGXLoadingProfile* Profile)
{
	Super::InitializeStrategy(Profile);

	LoadedTexture = nullptr;
	CurrentTipIndex = 0;

	if (!Profile || Profile->BackgroundImages.Num() == 0 || Profile->BackgroundImages[0].IsNull())
	{
		// EN: No images — become ready immediately (subsystem will see Minimal-like behavior)
		// ES: Sin imagenes — listo inmediatamente (el subsistema vera comportamiento tipo Minimal)
		PGX_LOG_WARNING(LogPGXLoading, TEXT("Strategy_Image: No background images — ready without image."));
		bReady = true;
		return;
	}

	// EN: Async load the first background image / ES: Carga async de la primera imagen de fondo
	FSoftObjectPath ImagePath = Profile->BackgroundImages[0].ToSoftObjectPath();

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	AssetLoadHandle = StreamableManager.RequestAsyncLoad(
		ImagePath,
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnImageLoaded),
		FStreamableManager::AsyncLoadHighPriority
	);

	PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy_Image: Async loading '%s'..."), *ImagePath.ToString());
}

void UPGXLoadingStrategy_Image::OnImageLoaded()
{
	if (CachedProfile && CachedProfile->BackgroundImages.Num() > 0)
	{
		LoadedTexture = CachedProfile->BackgroundImages[0].Get();
	}

	bReady = true;

	if (LoadedTexture)
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy_Image: Image loaded: %s"), *LoadedTexture->GetName());

		// EN: If already activated, apply immediately / ES: Si ya esta activado, aplicar inmediatamente
		if (bActivated && CachedWidget)
		{
			CachedWidget->SetBackgroundImage(LoadedTexture);
		}
	}
	else
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("Strategy_Image: Image load returned null."));
	}
}

void UPGXLoadingStrategy_Image::Activate(UPGXLoadingWidget* Widget)
{
	Super::Activate(Widget);

	if (!CachedWidget) return;

	// EN: Apply loaded image / ES: Aplicar imagen cargada
	if (LoadedTexture)
	{
		CachedWidget->SetBackgroundImage(LoadedTexture);
	}

	CachedWidget->SetProgressBarVisible(CachedProfile ? CachedProfile->bShowProgressBar : true);
	CachedWidget->SetSkipButtonVisible(CachedProfile ? CachedProfile->bAllowSkip : false);

	// EN: Set initial tip / ES: Establecer consejo inicial
	if (CachedProfile && CachedProfile->Tips.Num() > 0)
	{
		CachedWidget->SetTipText(CachedProfile->Tips[0]);
		CachedWidget->OnTipChanged(CachedProfile->Tips[0]);
	}

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Image: Activated."));
}

void UPGXLoadingStrategy_Image::Deactivate()
{
	LoadedTexture = nullptr;

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Image: Deactivated."));
	Super::Deactivate();
}

FText UPGXLoadingStrategy_Image::GetCurrentTip() const
{
	if (!CachedProfile || CachedProfile->Tips.Num() == 0)
	{
		return FText::GetEmpty();
	}
	return CachedProfile->Tips[CurrentTipIndex % CachedProfile->Tips.Num()];
}
