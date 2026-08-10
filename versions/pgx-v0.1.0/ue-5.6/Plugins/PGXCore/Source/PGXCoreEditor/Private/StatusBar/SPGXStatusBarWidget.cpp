// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "StatusBar/SPGXStatusBarWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "PGXCoreRuntime.h"
#include "Style/PGXVisualTokens.h"

#define LOCTEXT_NAMESPACE "PGXStatusBar"

void SPGXStatusBarWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("StatusBarText", "PGX v{0} | {1} Plugins"),
				FText::FromString(PGXVersion::String), PGXVersion::PluginCount))
		]
	];
}

void SPGXStatusBarWidget::Register()
{
	// EN: Status bar widget registration deferred until Hub provides primary dashboard.
	//     The toolbar dropdown serves as the main entry point for now.
	// ES: Registro del widget de status bar diferido hasta que el Hub provea el dashboard principal.
	//     El dropdown del toolbar sirve como punto de entrada principal por ahora.
}

void SPGXStatusBarWidget::Unregister()
{
}

#undef LOCTEXT_NAMESPACE
