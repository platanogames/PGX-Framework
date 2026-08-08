// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXTableRow.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Rendering/DrawElements.h"

void SPGXTableRow::Construct(const FArguments& InArgs)
{
	SystemColor = InArgs._SystemColor;
	bIsSelected = InArgs._bIsSelected;

	const float RowHeight = InArgs._bCompact ? PGX::Height::TableRowCompact : PGX::Height::TableRow;
	// EN: Zebra — alternate between Void and Base for subtle differentiation (Premium v2)
	// ES: Zebra — alternar entre Void y Base para diferenciacion sutil (Premium v2)
	const FLinearColor BgColor = InArgs._bIsEvenRow ? PGX::Surface::Void : PGX::Surface::Base;

	ChildSlot
	[
		SNew(SBox)
		.HeightOverride(RowHeight)
		[
			SNew(SBorder)
			.BorderBackgroundColor(BgColor)
			.Padding(FMargin(PGX::Width::AccentStripe + PGX::Spacing::SM, 0.0f, PGX::Spacing::SM, 0.0f))
			[
				InArgs._Content.Widget
			]
		]
	];
}

void SPGXTableRow::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
	bIsHovered = true;
}

void SPGXTableRow::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseLeave(MouseEvent);
	bIsHovered = false;
}

int32 SPGXTableRow::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();

	// EN: Draw hover/selected background overlay (Premium v2 — Raised for hover, Elevated+accent for selected)
	// ES: Overlay de fondo hover/seleccion (Premium v2 — Raised para hover, Elevated+acento para selected)
	if (bIsSelected)
	{
		FLinearColor SelBg = SystemColor;
		SelBg.A = 0.10f;
		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(),
			FAppStyle::GetBrush(TEXT("WhiteBrush")),
			ESlateDrawEffect::None, SelBg);
	}
	else if (bIsHovered)
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerId,
			AllottedGeometry.ToPaintGeometry(),
			FAppStyle::GetBrush(TEXT("WhiteBrush")),
			ESlateDrawEffect::None, PGX::Surface::Raised);
	}

	// EN: Left accent stripe
	if (bIsSelected || bIsHovered)
	{
		const float StripeWidth = bIsSelected ? PGX::Width::AccentStripeSelected : PGX::Width::AccentStripeHover;

		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerId + 1,
			AllottedGeometry.ToPaintGeometry(FVector2f(StripeWidth, static_cast<float>(LocalSize.Y)), FSlateLayoutTransform()),
			FAppStyle::GetBrush(TEXT("WhiteBrush")),
			ESlateDrawEffect::None, SystemColor);
	}

	// EN: Paint children
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
		LayerId + 2, InWidgetStyle, bParentEnabled);
}
