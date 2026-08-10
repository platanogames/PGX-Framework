// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Premium plugin card for Hub dashboard — rounded container, hover states, accent stripe.
// ES: Card premium de plugin para Hub dashboard — contenedor redondeado, estados hover, stripe de acento.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Brushes/SlateRoundedBoxBrush.h"

class PGXEDITORTOOLSEDITOR_API SPGXPluginCard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXPluginCard)
		: _SystemColor(FLinearColor::White)
	{}
		SLATE_ARGUMENT(FString, PluginName)
		SLATE_ARGUMENT(FString, Description)
		SLATE_ARGUMENT(FLinearColor, SystemColor)
		SLATE_ARGUMENT(FString, ButtonLabel)
		SLATE_EVENT(FOnClicked, OnButtonClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	void OnMouseLeave(const FPointerEvent& MouseEvent) override;

private:
	// EN: Pre-allocated brushes for rest/hover states (avoid allocation on hover)
	// ES: Brushes pre-allocados para estados rest/hover (evitar allocation en hover)
	TSharedPtr<FSlateRoundedBoxBrush> RestBrush;
	TSharedPtr<FSlateRoundedBoxBrush> HoverBrush;
	TSharedPtr<SBorder> CardBorder;
};
