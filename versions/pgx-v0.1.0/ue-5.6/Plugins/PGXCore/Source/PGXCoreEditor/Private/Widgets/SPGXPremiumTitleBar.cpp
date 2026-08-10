// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXPremiumTitleBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Style/PGXEditorStyle.h"

void SPGXPremiumTitleBar::Construct(const FArguments& InArgs)
{
	const FLinearColor SysColor = InArgs._SystemColor;
	const bool bHasIcon = InArgs._Icon != nullptr;
	const bool bHasSubtitle = !InArgs._Subtitle.IsEmpty();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.TitleBar"))
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

			// EN: Title bar content — Icon + Title/Subtitle + RightContent
			// ES: Contenido del title bar — Icono + Titulo/Subtitulo + ContenidoDerecho
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(PGX::Height::TitleBar - 3.0f) // minus accent stripe
				[
					SNew(SHorizontalBox)

					// EN: Icon (20x20, tinted to system color)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(PGX::Spacing::XL, 0.0f, PGX::Spacing::MD, 0.0f)
					[
						SNew(SBox)
						.WidthOverride(20.0f)
						.HeightOverride(20.0f)
						.Visibility(bHasIcon ? EVisibility::Visible : EVisibility::Collapsed)
						[
							SNew(SImage)
							.Image(InArgs._Icon)
							.ColorAndOpacity(FSlateColor(SysColor))
						]
					]

					// EN: Title + Subtitle stack
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					.Padding(bHasIcon ? 0.0f : PGX::Spacing::XL, 0.0f, 0.0f, 0.0f)
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
							.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
							.Visibility(bHasSubtitle ? EVisibility::Visible : EVisibility::Collapsed)
						]
					]

					// EN: Right content slot (action buttons)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, PGX::Spacing::XL, 0.0f)
					[
						InArgs._RightContent.Widget
					]
				]
			]
		]
	];
}
