// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Compact 8px health indicator dot with optional pulse animation.
// ES: Indicador de salud compacto 8px dot con animacion de pulso opcional.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Style/PGXVisualTokens.h"

enum class EPGXHealthState : uint8
{
	Active,
	Warning,
	Error,
	Offline
};

class PGXCOREEDITOR_API SPGXHealthDot : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXHealthDot)
		: _State(EPGXHealthState::Active)
		, _OverrideColor(FLinearColor::Transparent)
	{}
		SLATE_ARGUMENT(EPGXHealthState, State)
		SLATE_ARGUMENT(FLinearColor, OverrideColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	FLinearColor ResolveColor() const;

	EPGXHealthState HealthState = EPGXHealthState::Active;
	FLinearColor OverrideColor;
	FCurveSequence PulseSequence;
	FCurveHandle PulseHandle;
};
