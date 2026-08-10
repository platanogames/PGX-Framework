// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXEmptyStateV2.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SPGXEmptyStateV2::Construct(const FArguments& InArgs)
{
	const bool bHasIcon = InArgs._Icon != nullptr;
	const bool bHasAction = !InArgs._ActionLabel.IsEmpty();
	const bool bHasHint = !InArgs._Hint.IsEmpty();

	ChildSlot
	[
		SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(PGX::Spacing::XXL)
		[
			SNew(SVerticalBox)

			// EN: Optional large icon
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 0.0f, 0.0f, PGX::Spacing::LG)
			[
				SNew(SBox)
				.WidthOverride(48.0f)
				.HeightOverride(48.0f)
				.Visibility(bHasIcon ? EVisibility::Visible : EVisibility::Collapsed)
				[
					SNew(SImage)
					.Image(InArgs._Icon)
					.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				]
			]

			// EN: Main message
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(InArgs._Message)
				.Font(PGX::Font::Body())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
			]

			// EN: Optional hint text
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, PGX::Spacing::SM, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(InArgs._Hint)
				.Font(PGX::Font::Hint())
				.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
				.Visibility(bHasHint ? EVisibility::Visible : EVisibility::Collapsed)
			]

			// EN: Optional action button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, PGX::Spacing::LG, 0.0f, 0.0f)
			[
				SNew(SButton)
				.OnClicked(InArgs._OnActionClicked)
				.Visibility(bHasAction ? EVisibility::Visible : EVisibility::Collapsed)
				[
					SNew(STextBlock)
					.Text(InArgs._ActionLabel)
					.Font(PGX::Font::Body())
				]
			]
		]
	];
}
