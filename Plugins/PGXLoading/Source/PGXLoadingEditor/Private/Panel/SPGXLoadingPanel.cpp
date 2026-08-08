// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Panel/SPGXLoadingPanel.h"

#include "Style/PGXVisualTokens.h"
#include "Widgets/SPGXPremiumShell.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "PGXLoadingPanel"

void SPGXLoadingPanel::Construct(const FArguments& /*InArgs*/)
{
	// EN: Present loading and level-flow status in the shared PGX editor shell.
	// ES: Presentar el estado de carga y flujo de niveles en el panel PGX compartido.
	ChildSlot
	[
		SNew(SPGXPremiumShell)
		.SystemColor(PGX::System::Loading)
		.Title(LOCTEXT("PGXLoadingShellTitle", "PGX Loading + LevelFlow"))
		.Subtitle(LOCTEXT(
			"PGXLoadingShellSubtitle",
			"Loading and level-flow configuration overview"))
		.bShowFooter(true)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(PGX::Spacing::XL, PGX::Spacing::LG, PGX::Spacing::XL, PGX::Spacing::LG)
			[
				SNew(STextBlock)
				.Text(LOCTEXT(
					"PGXLoadingShellBody",
					"Inspect loading configuration, runtime status, level transitions, and active profiles from one editor panel."))
				.Font(PGX::Font::Body())
				.ColorAndOpacity(FSlateColor(PGX::Text::Secondary))
				.AutoWrapText(true)
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
