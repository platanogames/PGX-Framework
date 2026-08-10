// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXPremiumShell.h"
#include "Widgets/SPGXPremiumTitleBar.h"
#include "Widgets/SPGXFooterBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Style/PGXEditorStyle.h"

void SPGXPremiumShell::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		// EN: Full-bleed background — Surface::Void covers UE's default chrome in Docked mode
		// ES: Fondo full-bleed — Surface::Void cubre el chrome default de UE en modo Docked
		SNew(SBorder)
		.BorderImage(FPGXEditorStyle::Get().GetBrush("PGXEditor.Premium.Shell"))
		.Padding(0.0f)
		[
			SNew(SVerticalBox)

			// EN: Premium title bar (accent stripe + icon + title + right content)
			// ES: Title bar premium (stripe de acento + icono + titulo + contenido derecho)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SPGXPremiumTitleBar)
				.SystemColor(InArgs._SystemColor)
				.Title(InArgs._Title)
				.Subtitle(InArgs._Subtitle)
				.Icon(InArgs._Icon)
				.RightContent()
				[
					InArgs._TitleRightContent.Widget
				]
			]

			// EN: Content area with standard padding
			// ES: Area de contenido con padding estandar
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(PGX::Spacing::LG)
			[
				InArgs._Content.Widget
			]

			// EN: Footer bar (optional)
			// ES: Barra de footer (opcional)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.Visibility(InArgs._bShowFooter ? EVisibility::Visible : EVisibility::Collapsed)
				[
					SNew(SPGXFooterBar)
					.StatusText(InArgs._StatusText)
					.StatusColor(InArgs._StatusColor)
					.LeftContent()
					[
						InArgs._FooterLeftContent.Widget
					]
					.RightContent()
					[
						InArgs._FooterRightContent.Widget
					]
				]
			]
		]
	];
}
