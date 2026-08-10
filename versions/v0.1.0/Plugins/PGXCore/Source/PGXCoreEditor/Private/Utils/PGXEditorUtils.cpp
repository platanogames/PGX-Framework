// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Utils/PGXEditorUtils.h"
#include "Style/PGXVisualTokens.h"
#include "Utils/SPGXTelemetryGraph.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"

FString PGXEditorUtils::TagToLeafName(const FGameplayTag& Tag)
{
	if (!Tag.IsValid())
	{
		return TEXT("(none)");
	}
	const FString FullTag = Tag.ToString();
	int32 LastDot;
	if (FullTag.FindLastChar(TEXT('.'), LastDot))
	{
		return FullTag.Mid(LastDot + 1);
	}
	return FullTag;
}

FText PGXEditorUtils::FormatRelativeTime(double AbsoluteSeconds)
{
	const double Now = FPlatformTime::Seconds();
	const double Delta = Now - AbsoluteSeconds;

	if (Delta < 0.0 || AbsoluteSeconds <= 0.0)
	{
		return NSLOCTEXT("PGXEditor", "TimeNever", "never");
	}
	if (Delta < 60.0)
	{
		return FText::Format(
			NSLOCTEXT("PGXEditor", "TimeSecondsAgo", "{0}s ago"),
			FText::AsNumber(FMath::FloorToInt32(Delta)));
	}
	if (Delta < 3600.0)
	{
		return FText::Format(
			NSLOCTEXT("PGXEditor", "TimeMinutesAgo", "{0}m ago"),
			FText::AsNumber(FMath::FloorToInt32(Delta / 60.0)));
	}
	return FText::Format(
		NSLOCTEXT("PGXEditor", "TimeHoursAgo", "{0}h ago"),
		FText::AsNumber(FMath::FloorToInt32(Delta / 3600.0)));
}

TSharedRef<SWidget> PGXEditorUtils::MakeSystemColorBadge(const FLinearColor& Color)
{
	return SNew(SBox)
		.WidthOverride(16.0f)
		.HeightOverride(16.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(Color)
			.Padding(0)
		];
}

TSharedRef<SWidget> PGXEditorUtils::MakeProgressBarWithText(
	TAttribute<TOptional<float>> Percent,
	TAttribute<FText> Label,
	const FLinearColor& FillColor)
{
	return SNew(SBox)
		.HeightOverride(20.0f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SProgressBar)
				.Percent(Percent)
				.FillColorAndOpacity(FillColor)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Text::OnColor))
			]
		];
}

TSharedRef<SWidget> PGXEditorUtils::MakeProgressBarThresholded(
	TAttribute<TOptional<float>> Percent,
	TAttribute<FText> Label,
	float WarningThreshold,
	float ErrorThreshold)
{
	// EN: Capture thresholds by value for the lambda / ES: Capturar umbrales por valor para la lambda
	TAttribute<FSlateColor> DynamicColor = TAttribute<FSlateColor>::CreateLambda(
		[Percent, WarningThreshold, ErrorThreshold]() -> FSlateColor
		{
			const auto& Val = Percent.Get();
			if (!Val.IsSet())
			{
				return FSlateColor(PGX::Semantic::Neutral);
			}
			const float P = Val.GetValue();
			if (P >= ErrorThreshold)
			{
				return FSlateColor(PGX::Semantic::Error);
			}
			if (P >= WarningThreshold)
			{
				return FSlateColor(PGX::Semantic::Warn);
			}
			return FSlateColor(PGX::Semantic::Good);
		});

	return SNew(SBox)
		.HeightOverride(20.0f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SProgressBar)
				.Percent(Percent)
				.FillColorAndOpacity(DynamicColor)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Text::OnColor))
			]
		];
}

TSharedRef<SWidget> PGXEditorUtils::BuildGraphLegend(
	const TSharedPtr<SPGXTelemetryGraph>& Graph,
	const FLinearColor& MainColor)
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, PGX::Spacing::LG, 0.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, PGX::Spacing::XS, 0)
			[
				SNew(SBox).WidthOverride(10.0f).HeightOverride(2.0f)
				[SNew(SBorder).BorderBackgroundColor(MainColor)]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([Graph]() {
					return FText::Format(NSLOCTEXT("PGXEditor", "LegendCurrent", "Current: {0}"),
						FText::AsNumber(FMath::RoundToInt(Graph.IsValid() ? Graph->GetCurrent() : 0.0f)));
				})
				.Font(PGX::Font::Caption())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, PGX::Spacing::LG, 0.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([Graph]() {
				return FText::Format(NSLOCTEXT("PGXEditor", "LegendAvg", "Avg: {0}"),
					FText::AsNumber(FMath::RoundToInt(Graph.IsValid() ? Graph->GetAverage() : 0.0f)));
			})
			.Font(PGX::Font::Caption())
			.ColorAndOpacity(FSlateColor(PGX::Semantic::Warn))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([Graph]() {
				return FText::Format(NSLOCTEXT("PGXEditor", "LegendPeak", "Peak: {0}"),
					FText::AsNumber(FMath::RoundToInt(Graph.IsValid() ? Graph->GetPeak() : 0.0f)));
			})
			.Font(PGX::Font::Caption())
			.ColorAndOpacity(FSlateColor(PGX::Semantic::Error))
		];
}
