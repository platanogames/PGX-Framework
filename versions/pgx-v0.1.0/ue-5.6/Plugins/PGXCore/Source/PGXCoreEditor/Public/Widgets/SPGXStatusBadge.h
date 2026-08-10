// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Status dot or pill badge with semantic color.
// ES: Dot o pill de estado con color semantico.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

enum class EPGXBadgeShape : uint8
{
	Dot,
	Pill
};

class PGXCOREEDITOR_API SPGXStatusBadge : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXStatusBadge)
		: _Shape(EPGXBadgeShape::Dot)
		, _Color(PGX::Semantic::Neutral)
	{}
		SLATE_ARGUMENT(EPGXBadgeShape, Shape)
		SLATE_ARGUMENT(FLinearColor, Color)
		SLATE_ARGUMENT(FText, Label)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
