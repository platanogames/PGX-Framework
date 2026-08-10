// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Colored accent bar for panel/section headers.
// ES: Barra de acento coloreada para headers de panel/seccion.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

class PGXCOREEDITOR_API SPGXAccentBar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXAccentBar)
		: _Color(PGX::Semantic::Info)
		, _Height(PGX::Height::AccentBar)
	{}
		SLATE_ARGUMENT(FLinearColor, Color)
		SLATE_ARGUMENT(float, Height)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
