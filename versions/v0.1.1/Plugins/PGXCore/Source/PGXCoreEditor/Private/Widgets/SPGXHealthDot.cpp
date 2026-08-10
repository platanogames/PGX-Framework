// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXHealthDot.h"
#include "Rendering/DrawElements.h"
#include "Style/PGXEditorStyle.h"

void SPGXHealthDot::Construct(const FArguments& InArgs)
{
	HealthState = InArgs._State;
	OverrideColor = InArgs._OverrideColor;

	if (HealthState == EPGXHealthState::Active)
	{
		PulseHandle = PulseSequence.AddCurve(0.0f, PGX::Motion::AlertPulse * 2.5f, ECurveEaseFunction::CubicInOut);
		PulseSequence.Play(AsShared(), true);
	}
}

FVector2D SPGXHealthDot::ComputeDesiredSize(float) const
{
	return FVector2D(PGX::Height::HealthDot, PGX::Height::HealthDot);
}

FLinearColor SPGXHealthDot::ResolveColor() const
{
	if (OverrideColor.A > 0.01f)
	{
		return OverrideColor;
	}

	switch (HealthState)
	{
	case EPGXHealthState::Active:  return PGX::Semantic::Good;
	case EPGXHealthState::Warning: return PGX::Semantic::Warn;
	case EPGXHealthState::Error:   return PGX::Semantic::Error;
	case EPGXHealthState::Offline: return PGX::Semantic::Neutral;
	default:                       return PGX::Semantic::Neutral;
	}
}

int32 SPGXHealthDot::OnPaint(const FPaintArgs& /*Args*/, const FGeometry& AllottedGeometry,
	const FSlateRect& /*MyCullingRect*/, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& /*InWidgetStyle*/, bool /*bParentEnabled*/) const
{
	FLinearColor DotColor = ResolveColor();

	// EN: Offline = 30% opacity / ES: Offline = 30% opacidad
	if (HealthState == EPGXHealthState::Offline)
	{
		DotColor.A = 0.3f;
	}
	// EN: Active = pulse animation / ES: Activo = animacion de pulso
	else if (HealthState == EPGXHealthState::Active && PulseSequence.IsPlaying())
	{
		const float PulseAlpha = FMath::Lerp(0.4f, 1.0f, PulseHandle.GetLerp());
		DotColor.A = PulseAlpha;
	}

	const FSlateBrush* DotBrush = FPGXEditorStyle::Get().GetBrush("PGXEditor.Shape.StatusDot");

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		DotBrush,
		ESlateDrawEffect::None,
		DotColor);

	return LayerId;
}
