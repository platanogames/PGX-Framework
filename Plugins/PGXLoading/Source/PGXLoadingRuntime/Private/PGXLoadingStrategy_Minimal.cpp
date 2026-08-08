// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingStrategy_Minimal.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingRuntime.h"
#include "PGXLoadingProfile.h"
#include "PGXLoadingWidget.h"
#include "HAL/PlatformTime.h"

void UPGXLoadingStrategy_Minimal::InitializeStrategy(UPGXLoadingProfile* Profile)
{
	Super::InitializeStrategy(Profile);

	// EN: Minimal requires no external assets — immediately ready
	// ES: Minimal no requiere assets externos — listo inmediatamente
	bReady = true;
	CurrentTipIndex = 0;
	LastTipTime = 0.0;

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Minimal: Initialized (always ready)."));
}

void UPGXLoadingStrategy_Minimal::Activate(UPGXLoadingWidget* Widget)
{
	Super::Activate(Widget);

	if (!CachedWidget) return;

	// EN: Configure widget for minimal display / ES: Configurar widget para display minimo
	CachedWidget->SetProgressBarVisible(CachedProfile ? CachedProfile->bShowProgressBar : true);
	CachedWidget->SetSkipButtonVisible(CachedProfile ? CachedProfile->bAllowSkip : false);

	// EN: Set initial tip if available / ES: Establecer consejo inicial si disponible
	if (CachedProfile && CachedProfile->Tips.Num() > 0)
	{
		CurrentTipIndex = 0;
		CachedWidget->SetTipText(CachedProfile->Tips[0]);
		CachedWidget->OnTipChanged(CachedProfile->Tips[0]);
		LastTipTime = FPlatformTime::Seconds();
	}

	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Minimal: Activated."));
}

void UPGXLoadingStrategy_Minimal::Deactivate()
{
	PGX_LOG_VERBOSE(LogPGXLoading, TEXT("Strategy_Minimal: Deactivated."));
	Super::Deactivate();
}

bool UPGXLoadingStrategy_Minimal::IsReady() const
{
	return true; // EN: Always ready / ES: Siempre listo
}

FText UPGXLoadingStrategy_Minimal::GetCurrentTip() const
{
	if (!CachedProfile || CachedProfile->Tips.Num() == 0)
	{
		return FText::GetEmpty();
	}
	return CachedProfile->Tips[CurrentTipIndex % CachedProfile->Tips.Num()];
}
