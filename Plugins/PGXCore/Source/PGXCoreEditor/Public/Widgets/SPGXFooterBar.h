// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Panel footer bar with status text and optional content slots.
// ES: Barra de footer de panel con texto de estado y slots de contenido opcionales.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

class PGXCOREEDITOR_API SPGXFooterBar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXFooterBar)
		: _StatusColor(PGX::Text::Muted)
	{}
		SLATE_ATTRIBUTE(FText, StatusText)
		SLATE_ATTRIBUTE(FLinearColor, StatusColor)
		SLATE_NAMED_SLOT(FArguments, LeftContent)
		SLATE_NAMED_SLOT(FArguments, RightContent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
