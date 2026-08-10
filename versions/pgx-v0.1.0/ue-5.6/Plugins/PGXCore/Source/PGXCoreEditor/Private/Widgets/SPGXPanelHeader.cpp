// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXPanelHeader.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Style/PGXEditorStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"

void SPGXPanelHeader::Construct(const FArguments& InArgs)
{
	const FLinearColor SysColor = InArgs._SystemColor;

	// EN: Badge brush — rounded with system color (Premium v2)
	// ES: Brush del badge — redondeado con color de sistema (Premium v2)
	BadgeBrush = MakeShareable(new FSlateRoundedBoxBrush(SysColor, PGX::Radius::Small));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Card"))
		.Padding(FMargin(0.0f))
		[
			SNew(SVerticalBox)

			// EN: Top accent bar (border-top effect inside rounded container)
			// ES: Barra de acento superior (efecto border-top dentro del contenedor redondeado)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(PGX::Height::AccentBar)
				[
					SNew(SBorder)
					.BorderBackgroundColor(SysColor)
					.Padding(0)
				]
			]

			// EN: Header content row with increased padding (Premium v2)
			// ES: Fila de contenido del header con padding aumentado (Premium v2)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(PGX::Spacing::XL, PGX::Spacing::LG, PGX::Spacing::XL, PGX::Spacing::LG)
			[
				SNew(SHorizontalBox)

				// EN: System color badge (16x16, rounded)
				// ES: Badge de color de sistema (16x16, redondeado)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, PGX::Spacing::MD, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(PGX::Height::Badge)
					.HeightOverride(PGX::Height::Badge)
					[
						SNew(SBorder)
						.BorderImage(BadgeBrush.Get())
						.Padding(0)
						[
							// EN: Optional icon inside badge
							SNew(SBox)
							.Visibility(InArgs._Icon ? EVisibility::Visible : EVisibility::Collapsed)
							[
								SNew(SImage)
								.Image(InArgs._Icon)
								.ColorAndOpacity(FSlateColor(PGX::Text::OnColor))
							]
						]
					]
				]

				// EN: Title + subtitle
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(InArgs._Title)
						.Font(PGX::Font::PanelTitle())
						.ColorAndOpacity(FSlateColor(PGX::Text::Primary))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(InArgs._Subtitle)
						.Font(PGX::Font::BodySmall())
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
						.Visibility(InArgs._Subtitle.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
					]
				]

				// EN: Optional right-side content (status badges, buttons)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					InArgs._RightContent.Widget
				]
			]
		]
	];
}
