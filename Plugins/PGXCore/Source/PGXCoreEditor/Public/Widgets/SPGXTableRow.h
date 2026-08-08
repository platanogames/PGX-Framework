// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Standard table row with left accent stripe, zebra striping, hover/selected states.
// ES: Fila de tabla estandar con stripe de acento izquierdo, zebra striping, estados hover/selected.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Style/PGXVisualTokens.h"

class PGXCOREEDITOR_API SPGXTableRow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXTableRow)
		: _SystemColor(PGX::Semantic::Info)
		, _bIsEvenRow(false)
		, _bIsSelected(false)
		, _bCompact(false)
	{}
		SLATE_ARGUMENT(FLinearColor, SystemColor)
		SLATE_ARGUMENT(bool, bIsEvenRow)
		SLATE_ARGUMENT(bool, bIsSelected)
		SLATE_ARGUMENT(bool, bCompact)
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	void OnMouseLeave(const FPointerEvent& MouseEvent) override;

private:
	FLinearColor SystemColor;
	bool bIsSelected = false;
	bool bIsHovered = false;
};
