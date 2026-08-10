// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Full panel header: accent bar + badge + title + subtitle + optional icon/status.
// ES: Header completo de panel: barra de acento + badge + titulo + subtitulo + icono/estado opcional.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"
#include "Brushes/SlateRoundedBoxBrush.h"

class PGXCOREEDITOR_API SPGXPanelHeader : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXPanelHeader)
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

private:
	// EN: Owned brush for rounded badge (must outlive the widget)
	// ES: Brush propio para badge redondeado (debe sobrevivir al widget)
	TSharedPtr<FSlateRoundedBoxBrush> BadgeBrush;
};
