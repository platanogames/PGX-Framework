// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingWidget.h"
#include "PGXLoadingRuntime.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"

void UPGXLoadingWidget::SetBackgroundImage(UTexture2D* Texture)
{
	if (IMG_Background && Texture)
	{
		IMG_Background->SetBrushFromTexture(Texture, true);
	}
}

void UPGXLoadingWidget::SetBackgroundMaterial(UMaterialInterface* Material)
{
	if (IMG_Background && Material)
	{
		IMG_Background->SetBrushFromMaterial(Material);
	}
}

void UPGXLoadingWidget::SetProgressValue(float Progress)
{
	if (PB_Progress)
	{
		PB_Progress->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
	}
}

void UPGXLoadingWidget::SetTipText(const FText& TipText)
{
	if (TXT_Tip)
	{
		TXT_Tip->SetText(TipText);
	}
}

void UPGXLoadingWidget::SetStatusText(const FText& StatusText)
{
	if (TXT_Status)
	{
		TXT_Status->SetText(StatusText);
	}
}

void UPGXLoadingWidget::SetSkipButtonVisible(bool bVisible)
{
	if (BTN_Skip)
	{
		BTN_Skip->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPGXLoadingWidget::SetProgressBarVisible(bool bVisible)
{
	if (PB_Progress)
	{
		PB_Progress->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPGXLoadingWidget::BindSkipButton(const FSimpleDelegate& Callback)
{
	SkipCallback = Callback;
	if (BTN_Skip)
	{
		BTN_Skip->OnClicked.Clear();
		BTN_Skip->OnClicked.AddDynamic(this, &ThisClass::HandleSkipClicked);
	}
}

void UPGXLoadingWidget::SetContinueButtonVisible(bool bVisible)
{
	if (BTN_Continue)
	{
		BTN_Continue->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPGXLoadingWidget::BindContinueButton(const FSimpleDelegate& Callback)
{
	ContinueCallback = Callback;
	if (BTN_Continue)
	{
		BTN_Continue->OnClicked.Clear();
		BTN_Continue->OnClicked.AddDynamic(this, &ThisClass::HandleContinueClicked);
	}
}

void UPGXLoadingWidget::HandleSkipClicked()
{
	SkipCallback.ExecuteIfBound();
}

void UPGXLoadingWidget::HandleContinueClicked()
{
	ContinueCallback.ExecuteIfBound();
}
