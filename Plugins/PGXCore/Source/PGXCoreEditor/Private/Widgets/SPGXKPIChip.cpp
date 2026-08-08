// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXKPIChip.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Style/PGXEditorStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"

void SPGXKPIChip::Construct(const FArguments& InArgs)
{
	// EN: Decide value widget: custom ValueWidget slot or default FText
	// ES: Decidir widget de valor: slot custom ValueWidget o FText por defecto
	TSharedRef<SWidget> ValueDisplay = InArgs._ValueWidget.Widget;

	// EN: If no custom widget provided, use default text display
	// ES: Si no se proporciono widget custom, usar texto por defecto
	if (ValueDisplay == SNullWidget::NullWidget)
	{
		SAssignNew(ValueDisplay, STextBlock)
			.Text(InArgs._Value)
			.Font(PGX::Font::KPIValue())
			.ColorAndOpacity(FSlateColor(PGX::Text::Primary));
	}

	// EN: Check if icon was provided / ES: Verificar si se proporciono icono
	TSharedRef<SWidget> IconWidget = InArgs._Icon.Widget;
	const bool bHasIcon = IconWidget != SNullWidget::NullWidget;

	// EN: Card brush — elevated background with Glass border, rounded (Premium v2)
	// ES: Brush de tarjeta — fondo elevado con borde Glass, redondeado (Premium v2)
	CardBrush = MakeShareable(new FSlateRoundedBoxBrush(
		PGX::Surface::Elevated, PGX::Radius::Medium,
		PGX::Border::Glass, 1.0f));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(CardBrush.Get())
		.Padding(FMargin(0.0f))
		[
			SNew(SVerticalBox)

			// EN: Top accent bar (integrated as border-top of the rounded card)
			// ES: Barra de acento superior (integrada como border-top de la card redondeada)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(PGX::Height::AccentBar)
				[
					SNew(SBorder)
					.BorderBackgroundColor(InArgs._AccentColor)
					.Padding(0)
				]
			]

			// EN: Card content with Premium padding (12px H, 8px V)
			// ES: Contenido de la tarjeta con padding Premium (12px H, 8px V)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(PGX::Spacing::LG, PGX::Spacing::MD, PGX::Spacing::LG, PGX::Spacing::MD)
			[
				SNew(SHorizontalBox)

				// EN: Optional icon / ES: Icono opcional
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, bHasIcon ? PGX::Spacing::MD : 0.0f, 0.0f)
				[
					SNew(SBox)
					.Visibility(bHasIcon ? EVisibility::Visible : EVisibility::Collapsed)
					[
						IconWidget
					]
				]

				// EN: Label + Value stack / ES: Stack de Label + Valor
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)

					// EN: Label (muted, small)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(InArgs._Label)
						.Font(PGX::Font::KPILabel())
						.ColorAndOpacity(FSlateColor(PGX::Text::Muted))
					]

					// EN: Value (custom widget or default text)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, PGX::Spacing::XS, 0.0f, 0.0f)
					[
						ValueDisplay
					]
				]
			]
		]
	];
}
