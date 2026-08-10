// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Premium shell — wraps ALL panel content. Used in both Docked and Floating modes.
//     Provides: background (covers UE chrome), title bar, content area, footer.
// ES: Contenedor comun para paneles acoplados y flotantes.
//     Provee: fondo (cubre chrome de UE), title bar, area de contenido, footer.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

class PGXCOREEDITOR_API SPGXPremiumShell : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXPremiumShell)
		: _SystemColor(PGX::Semantic::Info)
		, _Icon(nullptr)
		, _StatusColor(PGX::Text::Muted)
		, _bShowFooter(true)
	{}
		SLATE_ARGUMENT(FLinearColor, SystemColor)
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FText, Subtitle)
		SLATE_ARGUMENT(const FSlateBrush*, Icon)
		SLATE_NAMED_SLOT(FArguments, TitleRightContent)
		SLATE_ATTRIBUTE(FText, StatusText)
		SLATE_ATTRIBUTE(FLinearColor, StatusColor)
		SLATE_NAMED_SLOT(FArguments, FooterLeftContent)
		SLATE_NAMED_SLOT(FArguments, FooterRightContent)
		SLATE_ARGUMENT(bool, bShowFooter)
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
