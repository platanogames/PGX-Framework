// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXAccentBar.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"

void SPGXAccentBar::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBox)
		.HeightOverride(InArgs._Height)
		[
			SNew(SBorder)
			.BorderBackgroundColor(InArgs._Color)
			.Padding(0)
		]
	];
}
