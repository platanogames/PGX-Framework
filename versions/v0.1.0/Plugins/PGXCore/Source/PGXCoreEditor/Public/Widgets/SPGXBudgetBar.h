// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Budget bar with semantic gradient coloring and unlimited mode.
// ES: Barra de presupuesto con gradiente semantico y modo ilimitado.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

class PGXCOREEDITOR_API SPGXBudgetBar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXBudgetBar)
		: _Value(0.0f)
		, _MaxValue(100.0f)
		, _bUnlimited(false)
	{}
		SLATE_ARGUMENT(FText, Label)
		SLATE_ATTRIBUTE(float, Value)
		SLATE_ARGUMENT(float, MaxValue)
		SLATE_ARGUMENT(bool, bUnlimited)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static FLinearColor GetBarColor(float Ratio);

private:
	TAttribute<float> ValueAttr;
	float MaxValue = 100.0f;
};
