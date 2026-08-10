// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Dashboard/SPGXPluginCard.h"
#include "Style/PGXVisualTokens.h"
#include "Style/PGXEditorStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SPGXPluginCard::Construct(const FArguments& InArgs)
{
	const FLinearColor SysColor = InArgs._SystemColor;

	// EN: Pre-allocate rest/hover brushes — avoid allocation during paint
	// ES: Pre-allocar brushes rest/hover — evitar allocation durante paint
	RestBrush = MakeShareable(new FSlateRoundedBoxBrush(
		PGX::Surface::Raised, PGX::Radius::Large,
		PGX::Border::Glass, 1.0f));

	FLinearColor HoverBorderColor = SysColor;
	HoverBorderColor.A = 0.40f;
	HoverBrush = MakeShareable(new FSlateRoundedBoxBrush(
		PGX::Surface::Elevated, PGX::Radius::Large,
		HoverBorderColor, 1.0f));

	ChildSlot
	[
		SAssignNew(CardBorder, SBorder)
		.BorderImage(RestBrush.Get())
		.Padding(0.0f)
		[
			SNew(SVerticalBox)

			// EN: Top accent stripe (3px, system color)
			// ES: Stripe de acento superior (3px, color de sistema)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(3.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(SysColor)
					.Padding(0)
				]
			]

			// EN: Card content
			// ES: Contenido de la card
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(PGX::Spacing::XL, PGX::Spacing::LG, PGX::Spacing::XL, PGX::Spacing::LG)
			[
				SNew(SVerticalBox)

				// EN: Plugin name (system color)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(InArgs._PluginName))
					.Font(PGX::Font::SubHeader())
					.ColorAndOpacity(FSlateColor(SysColor))
				]

				// EN: Description (secondary text, wrapping)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, PGX::Spacing::XS, 0.0f, PGX::Spacing::MD)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InArgs._Description))
					.Font(PGX::Font::BodySmall())
					.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
					.AutoWrapText(true)
				]

				// EN: Action button (Secondary tier style)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.ButtonStyle(&FPGXEditorStyle::Get().GetWidgetStyle<FButtonStyle>(
						FName(TEXT("PGXEditor.Button.Secondary"))))
					.OnClicked(InArgs._OnButtonClicked)
					.ContentPadding(FMargin(PGX::Spacing::LG, PGX::Spacing::SM))
					[
						SNew(STextBlock)
						.Text(FText::FromString(InArgs._ButtonLabel))
						.Font(PGX::Font::Body())
						.ColorAndOpacity(FSlateColor(PGX::Text::Primary))
					]
				]
			]
		]
	];
}

void SPGXPluginCard::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
	if (CardBorder.IsValid() && HoverBrush.IsValid())
	{
		CardBorder->SetBorderImage(HoverBrush.Get());
	}
}

void SPGXPluginCard::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseLeave(MouseEvent);
	if (CardBorder.IsValid() && RestBrush.IsValid())
	{
		CardBorder->SetBorderImage(RestBrush.Get());
	}
}
