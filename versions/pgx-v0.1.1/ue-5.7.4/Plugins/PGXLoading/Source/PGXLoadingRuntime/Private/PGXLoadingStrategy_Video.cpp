// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingStrategy_Video.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingRuntime.h"
#include "PGXLoadingProfile.h"
#include "PGXLoadingWidget.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "MediaTexture.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UPGXLoadingStrategy_Video::InitializeStrategy(UPGXLoadingProfile* Profile)
{
	Super::InitializeStrategy(Profile);

	Player = nullptr;
	LoadedSource = nullptr;
	MediaTex = nullptr;
	bVideoReady = false;
	CurrentTipIndex = 0;
	TipTimer = 0.0f;

	if (!Profile || Profile->VideoSource.IsNull())
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("Strategy_Video: No video source — ready without video."));
		bReady = true;
		return;
	}

	// EN: Async load the video source / ES: Carga async de la fuente de video
	FSoftObjectPath SourcePath = Profile->VideoSource.ToSoftObjectPath();

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	AssetLoadHandle = StreamableManager.RequestAsyncLoad(
		SourcePath,
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnVideoSourceLoaded),
		FStreamableManager::AsyncLoadHighPriority
	);

	PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy_Video: Async loading video source '%s'..."), *SourcePath.ToString());
}

void UPGXLoadingStrategy_Video::OnVideoSourceLoaded()
{
	if (CachedProfile)
	{
		LoadedSource = CachedProfile->VideoSource.Get();
	}

	if (LoadedSource)
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy_Video: Video source loaded: %s"), *LoadedSource->GetName());
	}
	else
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("Strategy_Video: Video source load returned null — fallback to no-video."));
	}

	bReady = true;
}

void UPGXLoadingStrategy_Video::Activate(UPGXLoadingWidget* Widget)
{
	Super::Activate(Widget);

	if (!CachedWidget) return;

	if (LoadedSource)
	{
		// EN: Create MediaPlayer and MediaTexture on demand / ES: Crear bajo demanda
		Player = NewObject<UMediaPlayer>(this);
		MediaTex = NewObject<UMediaTexture>(this);
		MediaTex->SetMediaPlayer(Player);
		MediaTex->UpdateResource();

		// EN: Try to open the source / ES: Intentar abrir la fuente
		if (Player->OpenSource(LoadedSource))
		{
			bVideoReady = true;
			Player->SetLooping(true);
			Player->Play();
			PGX_LOG_INFO(LogPGXLoading, TEXT("Strategy_Video: Video playing."));
		}
		else
		{
			// EN: Codec failure — fallback to no-video / ES: Fallo de codec — fallback
			PGX_LOG_WARNING(LogPGXLoading, TEXT("Strategy_Video: Failed to open video source — codec issue?"));
			bVideoReady = false;
		}
	}

	CachedWidget->SetProgressBarVisible(CachedProfile ? CachedProfile->bShowProgressBar : true);
	CachedWidget->SetSkipButtonVisible(CachedProfile ? CachedProfile->bAllowSkip : false);

	if (CachedProfile && CachedProfile->Tips.Num() > 0)
	{
		CachedWidget->SetTipText(CachedProfile->Tips[0]);
		CachedWidget->OnTipChanged(CachedProfile->Tips[0]);
	}

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Video: Activated (video=%s)."),
		bVideoReady ? TEXT("playing") : TEXT("unavailable"));
}

void UPGXLoadingStrategy_Video::UpdateVisual(float DeltaTime)
{
	if (!bActivated) return;

	// EN: Tip rotation / ES: Rotacion de consejos
	if (CachedProfile && CachedProfile->Tips.Num() > 1)
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

void UPGXLoadingStrategy_Video::Deactivate()
{
	// EN: Explicitly close MediaPlayer before releasing / ES: Cerrar explicitamente antes de liberar
	if (Player)
	{
		Player->Close();
		Player = nullptr;
	}

	MediaTex = nullptr;
	LoadedSource = nullptr;
	bVideoReady = false;

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Video: Deactivated (MediaPlayer closed)."));
	Super::Deactivate();
}

FText UPGXLoadingStrategy_Video::GetCurrentTip() const
{
	if (!CachedProfile || CachedProfile->Tips.Num() == 0)
	{
		return FText::GetEmpty();
	}
	return CachedProfile->Tips[CurrentTipIndex % CachedProfile->Tips.Num()];
}
