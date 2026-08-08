// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXFooterBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

void SPGXFooterBar::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)

		// EN: Top border line (1px Glass — replaces SSeparator for integrated look)
		// ES: Linea de borde superior (1px Glass — reemplaza SSeparator para aspecto integrado)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(1.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(PGX::Border::Glass)
				.Padding(0)
			]
		]

		// EN: Footer content with Base background
		// ES: Contenido del footer con fondo Base
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(PGX::Height::Footer)
			[
				SNew(SBorder)
				.BorderBackgroundColor(PGX::Surface::Base)
				.Padding(FMargin(PGX::Spacing::LG, 0.0f))
				[
					SNew(SHorizontalBox)

					// EN: Left content slot
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						InArgs._LeftContent.Widget
					]

					// EN: Flexible spacer
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNullWidget::NullWidget
					]

					// EN: Status text
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(PGX::Spacing::MD, 0.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(InArgs._StatusText)
						.Font(PGX::Font::Caption())
						.ColorAndOpacity_Lambda([InArgs]() -> FSlateColor
						{
							return FSlateColor(InArgs._StatusColor.Get());
						})
					]

					// EN: Right content slot
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(PGX::Spacing::MD, 0.0f, 0.0f, 0.0f)
					[
						InArgs._RightContent.Widget
					]
				]
			]
		]
	];
}
