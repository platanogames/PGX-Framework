// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Section header with title + separator line + optional accent.
// ES: Header de seccion con titulo + linea separadora + acento opcional.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

class PGXCOREEDITOR_API SPGXSectionDivider : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXSectionDivider)
		: _AccentColor(FLinearColor::Transparent)
	{}
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FLinearColor, AccentColor)
		// EN: Optional right-side content (filters, range buttons, etc.) / ES: Contenido derecho opcional (filtros, botones de rango, etc.)
		SLATE_NAMED_SLOT(FArguments, RightContent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
