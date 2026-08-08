// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXBudgetBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

void SPGXBudgetBar::Construct(const FArguments& InArgs)
{
	ValueAttr = InArgs._Value;
	MaxValue = FMath::Max(InArgs._MaxValue, 1.0f);

	if (InArgs._bUnlimited)
	{
		// EN: Unlimited mode — show infinity symbol
		ChildSlot
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, PGX::Spacing::SM, 0.0f)
			[
				SNew(STextBlock)
				.Text(InArgs._Label)
				.Font(PGX::Font::BodySmall())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("\u221E")))
				.Font(PGX::Font::SubHeader())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
			]
		];
		return;
	}

	// EN: Bar mode — label + SProgressBar with dynamic color + percentage
	ChildSlot
	[
		SNew(SHorizontalBox)

		// EN: Label
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 0.0f, PGX::Spacing::SM, 0.0f)
		[
			SNew(STextBlock)
			.Text(InArgs._Label)
			.Font(PGX::Font::BodySmall())
			.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
		]

		// EN: Progress bar with semantic gradient color
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(PGX::Height::BudgetBar)
			[
				SNew(SProgressBar)
				.Percent_Lambda([this]() -> TOptional<float>
				{
					return FMath::Clamp(ValueAttr.Get() / MaxValue, 0.0f, 1.0f);
				})
				.FillColorAndOpacity_Lambda([this]() -> FSlateColor
				{
					const float Ratio = FMath::Clamp(ValueAttr.Get() / MaxValue, 0.0f, 1.0f);
					return FSlateColor(GetBarColor(Ratio));
				})
				.BackgroundImage(FAppStyle::GetBrush("WhiteBrush"))
				.BorderPadding(FVector2D::ZeroVector)
			]
		]

		// EN: Percentage text
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(PGX::Spacing::SM, 0.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				const float Ratio = FMath::Clamp(ValueAttr.Get() / MaxValue, 0.0f, 1.0f);
				return FText::Format(NSLOCTEXT("PGXBudget", "Pct", "{0}%"),
					FText::AsNumber(FMath::RoundToInt32(Ratio * 100.0f)));
			})
			.Font(PGX::Font::Caption())
			.ColorAndOpacity_Lambda([this]() -> FSlateColor
			{
				const float Ratio = FMath::Clamp(ValueAttr.Get() / MaxValue, 0.0f, 1.0f);
				return FSlateColor(GetBarColor(Ratio));
			})
		]
	];
}

FLinearColor SPGXBudgetBar::GetBarColor(float Ratio)
{
	if (Ratio < 0.6f)
	{
		return PGX::Semantic::Good;
	}
	if (Ratio < 0.8f)
	{
		const float T = (Ratio - 0.6f) / 0.2f;
		return FMath::Lerp(PGX::Semantic::Good, PGX::Semantic::Warn, T);
	}
	if (Ratio < 0.95f)
	{
		const float T = (Ratio - 0.8f) / 0.15f;
		return FMath::Lerp(PGX::Semantic::Warn, PGX::Semantic::Error, T);
	}
	return PGX::Semantic::Error;
}
