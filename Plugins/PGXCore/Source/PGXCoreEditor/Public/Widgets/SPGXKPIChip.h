// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Compact metric display: label + value + optional accent color.
// ES: Display compacto de metrica: label + valor + color de acento opcional.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"
#include "Brushes/SlateRoundedBoxBrush.h"

class PGXCOREEDITOR_API SPGXKPIChip : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXKPIChip)
		: _AccentColor(PGX::Semantic::Info)
	{}
		SLATE_ARGUMENT(FText, Label)
		SLATE_ATTRIBUTE(FText, Value)
		SLATE_ARGUMENT(FLinearColor, AccentColor)
		// EN: Optional custom widget to replace default Value text / ES: Widget custom opcional para reemplazar texto Value
		SLATE_NAMED_SLOT(FArguments, ValueWidget)
		// EN: Optional icon widget displayed left of label/value / ES: Widget de icono opcional a la izquierda de label/valor
		SLATE_NAMED_SLOT(FArguments, Icon)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// EN: Owned brush for rounded card (must outlive the widget)
	// ES: Brush propio para card redondeada (debe sobrevivir al widget)
	TSharedPtr<FSlateRoundedBoxBrush> CardBrush;
};
