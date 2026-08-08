// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXStatusBadge.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Style/PGXEditorStyle.h"

void SPGXStatusBadge::Construct(const FArguments& InArgs)
{
	if (InArgs._Shape == EPGXBadgeShape::Dot)
	{
		ChildSlot
		[
			SNew(SBox)
			.WidthOverride(PGX::Height::StatusDot)
			.HeightOverride(PGX::Height::StatusDot)
			[
				SNew(SBorder)
				.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Shape.StatusDot"))
				.BorderBackgroundColor(InArgs._Color)
				.Padding(0)
			]
		];
	}
	else
	{
		// EN: Pill shape with label
		ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Shape.Pill"))
			.BorderBackgroundColor(InArgs._Color)
			.Padding(FMargin(PGX::Spacing::MD, PGX::Spacing::XS))
			[
				SNew(STextBlock)
				.Text(InArgs._Label)
				.Font(PGX::Font::Badge())
				.ColorAndOpacity(FSlateColor(PGX::Text::OnColor))
			]
		];
	}
}
