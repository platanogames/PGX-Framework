// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Premium title bar for PGX panels — accent stripe, icon, title, subtitle, right content slot.
// ES: Title bar premium para paneles PGX — stripe de acento, icono, titulo, subtitulo, slot de contenido derecho.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

class PGXCOREEDITOR_API SPGXPremiumTitleBar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXPremiumTitleBar)
		: _SystemColor(PGX::Semantic::Info)
		, _Icon(nullptr)
	{}
		SLATE_ARGUMENT(FLinearColor, SystemColor)
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FText, Subtitle)
		SLATE_ARGUMENT(const FSlateBrush*, Icon)
		SLATE_NAMED_SLOT(FArguments, RightContent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
