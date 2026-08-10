// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXDataTable.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"

void SPGXDataTableHeader::Construct(const FArguments& InArgs)
{
	TSharedRef<SHorizontalBox> HeaderRow = SNew(SHorizontalBox);

	for (const FPGXColumnDef& Col : InArgs._Columns)
	{
		TSharedRef<SWidget> LabelWidget = SNew(STextBlock)
			.Text(Col.Label)
			.Font(PGX::Font::CaptionBold())
			.ColorAndOpacity(FSlateColor(PGX::Text::Muted));

		if (Col.Width > 0.0f)
		{
			HeaderRow->AddSlot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(Col.HAlign)
				.Padding(PGX::Spacing::SM, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(Col.Width)
					[
						LabelWidget
					]
				];
		}
		else
		{
			HeaderRow->AddSlot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.HAlign(Col.HAlign)
				.Padding(PGX::Spacing::SM, 0.0f)
				[
					LabelWidget
				];
		}
	}

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(PGX::Spacing::SM, PGX::Spacing::SM)
		[
			HeaderRow
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
			.ColorAndOpacity(PGX::Border::Default)
		]
	];
}
