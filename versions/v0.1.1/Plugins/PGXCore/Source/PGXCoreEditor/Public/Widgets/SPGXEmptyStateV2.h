// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Enhanced empty state with icon + message + hint + optional action button.
// ES: Estado vacio mejorado con icono + mensaje + hint + boton de accion opcional.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

class PGXCOREEDITOR_API SPGXEmptyStateV2 : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXEmptyStateV2)
		: _Icon(nullptr)
	{}
		SLATE_ARGUMENT(FText, Message)
		SLATE_ARGUMENT(FText, Hint)
		SLATE_ARGUMENT(const FSlateBrush*, Icon)
		SLATE_EVENT(FOnClicked, OnActionClicked)
		SLATE_ARGUMENT(FText, ActionLabel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
