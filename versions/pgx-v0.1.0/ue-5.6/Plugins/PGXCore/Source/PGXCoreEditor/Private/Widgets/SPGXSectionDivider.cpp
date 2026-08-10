// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXSectionDivider.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Style/PGXEditorStyle.h"

void SPGXSectionDivider::Construct(const FArguments& InArgs)
{
	const bool bHasAccent = InArgs._AccentColor.A > 0.01f;
	TSharedRef<SWidget> RightWidget = InArgs._RightContent.Widget;
	const bool bHasRight = RightWidget != SNullWidget::NullWidget;

	ChildSlot
	[
		// EN: Section container — rounded rect with Glass border (Premium style)
		// ES: Contenedor de seccion — rect redondeado con borde Glass (estilo Premium)
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, PGX::Spacing::LG, 0.0f, PGX::Spacing::SM)
		[
			SNew(SBorder)
			.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.SectionHeader"))
			.Padding(FMargin(0.0f))
			[
				SNew(SHorizontalBox)

				// EN: Accent stripe (left edge, inside rounded container)
				// ES: Stripe de acento (borde izquierdo, dentro del contenedor redondeado)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Fill)
				[
					SNew(SBox)
					.WidthOverride(PGX::Width::AccentStripe)
					.Visibility(bHasAccent ? EVisibility::Visible : EVisibility::Collapsed)
					[
						SNew(SBorder)
						.BorderBackgroundColor(InArgs._AccentColor)
						.Padding(0)
					]
				]

				// EN: Title text with increased padding
				// ES: Texto de titulo con padding aumentado
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(PGX::Spacing::LG, PGX::Spacing::MD, PGX::Spacing::MD, PGX::Spacing::MD)
				[
					SNew(STextBlock)
					.Text(InArgs._Title)
					.Font(PGX::Font::SectionHeader())
					.ColorAndOpacity(FSlateColor(PGX::Text::Primary))
				]

				// EN: Spacer / ES: Espaciador
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)

				// EN: Optional right content (filters, range buttons, etc.)
				// ES: Contenido derecho opcional (filtros, botones de rango, etc.)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, PGX::Spacing::LG, 0.0f)
				[
					SNew(SBox)
					.Visibility(bHasRight ? EVisibility::Visible : EVisibility::Collapsed)
					[
						RightWidget
					]
				]
			]
		]
	];
}
