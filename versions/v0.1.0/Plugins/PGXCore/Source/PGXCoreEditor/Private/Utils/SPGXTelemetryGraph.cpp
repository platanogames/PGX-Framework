// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Utils/SPGXTelemetryGraph.h"
#include "Rendering/DrawElements.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"

void SPGXTelemetryGraph::Construct(const FArguments& InArgs)
{
	BufferCapacity = FMath::Max(InArgs._BufferSize, 8);
	LineColor = InArgs._LineColor;
	LineThickness = InArgs._LineThickness;
	MinValue = InArgs._MinValue;
	MaxValue = InArgs._MaxValue;
	BackgroundColor = InArgs._BackgroundColor;
	bShowGrid = InArgs._bShowGrid;
	bShowAvgLine = InArgs._bShowAvgLine;
	bShowPeakMarkers = InArgs._bShowPeakMarkers;
	bShowLabels = InArgs._bShowLabels;
	GridColor = InArgs._GridColor;
	AvgLineColor = InArgs._AvgLineColor;

	RingBuffer.SetNumZeroed(BufferCapacity);
	CachedPoints.SetNumUninitialized(BufferCapacity);
	AvgLinePoints.SetNumUninitialized(2);
	GridLinePoints.SetNumUninitialized(2);
	WriteIndex = 0;
	SampleCount = 0;
}

void SPGXTelemetryGraph::PushValue(float Value)
{
	RingBuffer[WriteIndex] = Value;
	WriteIndex = (WriteIndex + 1) % BufferCapacity;
	SampleCount = FMath::Min(SampleCount + 1, BufferCapacity);
	bStatsDirty = true;
}

void SPGXTelemetryGraph::Clear()
{
	FMemory::Memzero(RingBuffer.GetData(), BufferCapacity * sizeof(float));
	WriteIndex = 0;
	SampleCount = 0;
	bStatsDirty = true;
}

float SPGXTelemetryGraph::GetAverage() const
{
	if (bStatsDirty) { UpdateCachedStats(); }
	return CachedAvg;
}

float SPGXTelemetryGraph::GetPeak() const
{
	if (bStatsDirty) { UpdateCachedStats(); }
	return CachedPeak;
}

float SPGXTelemetryGraph::GetCurrent() const
{
	if (SampleCount == 0) { return 0.0f; }
	const int32 LastIdx = (WriteIndex - 1 + BufferCapacity) % BufferCapacity;
	return RingBuffer[LastIdx];
}

void SPGXTelemetryGraph::UpdateCachedStats() const
{
	if (SampleCount == 0)
	{
		CachedAvg = 0.0f;
		CachedPeak = 0.0f;
		bStatsDirty = false;
		return;
	}

	const int32 ReadStart = (WriteIndex - SampleCount + BufferCapacity) % BufferCapacity;
	float Sum = 0.0f;
	float Peak = -FLT_MAX;
	for (int32 i = 0; i < SampleCount; ++i)
	{
		const float Val = RingBuffer[(ReadStart + i) % BufferCapacity];
		Sum += Val;
		if (Val > Peak) { Peak = Val; }
	}
	CachedAvg = Sum / static_cast<float>(SampleCount);
	CachedPeak = Peak;
	bStatsDirty = false;
}

FVector2D SPGXTelemetryGraph::ComputeDesiredSize(float) const
{
	return FVector2D(200.0f, 60.0f);
}

int32 SPGXTelemetryGraph::OnPaint(const FPaintArgs& /*Args*/, const FGeometry& AllottedGeometry,
	const FSlateRect& /*MyCullingRect*/, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& /*InWidgetStyle*/, bool /*bParentEnabled*/) const
{
	const FVector2f LocalSize(
		static_cast<float>(AllottedGeometry.GetLocalSize().X),
		static_cast<float>(AllottedGeometry.GetLocalSize().Y));

	const float LabelMargin = bShowLabels ? 32.0f : 0.0f;
	const float GraphLeft = LabelMargin;
	const float GraphWidth = LocalSize.X - LabelMargin;

	// EN: Draw background / ES: Dibujar fondo
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		FAppStyle::GetBrush(TEXT("WhiteBrush")),
		ESlateDrawEffect::None,
		BackgroundColor);

	const float Range = FMath::Max(MaxValue - MinValue, SMALL_NUMBER);

	// EN: Draw horizontal grid lines (4 divisions) / ES: Dibujar lineas de grid horizontales (4 divisiones)
	if (bShowGrid)
	{
		for (int32 g = 0; g <= 4; ++g)
		{
			const float YFrac = static_cast<float>(g) / 4.0f;
			const float Y = LocalSize.Y * YFrac;

			GridLinePoints[0] = FVector2f(GraphLeft, Y);
			GridLinePoints[1] = FVector2f(LocalSize.X, Y);

			// EN: Resize to exactly 2 points / ES: Redimensionar a exactamente 2 puntos
			GridLinePoints.SetNum(2, EAllowShrinking::No);
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 1,
				AllottedGeometry.ToPaintGeometry(),
				GridLinePoints,
				ESlateDrawEffect::None,
				GridColor,
				false,
				1.0f);
		}
	}

	// EN: Draw Y-axis labels / ES: Dibujar etiquetas del eje Y
	if (bShowLabels)
	{
		const FSlateFontInfo LabelFont = PGX::Font::Caption();
		const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

		for (int32 g = 0; g <= 4; ++g)
		{
			const float YFrac = static_cast<float>(g) / 4.0f;
			const float LabelValue = MaxValue - (YFrac * Range);
			const FString LabelStr = FString::Printf(TEXT("%.0f"), LabelValue);
			const FVector2D TextSize = FontMeasure->Measure(LabelStr, LabelFont);
			const float Y = LocalSize.Y * YFrac - static_cast<float>(TextSize.Y) * 0.5f;

			FSlateDrawElement::MakeText(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(
					FVector2f(static_cast<float>(TextSize.X), static_cast<float>(TextSize.Y)),
					FSlateLayoutTransform(FVector2f(0.0f, Y))),
				LabelStr,
				LabelFont,
				ESlateDrawEffect::None,
				PGX::Text::Muted);
		}
	}

	if (SampleCount < 2)
	{
		return LayerId + 2;
	}

	// EN: Update stats if dirty / ES: Actualizar stats si dirty
	if (bStatsDirty) { UpdateCachedStats(); }

	// EN: Build point array from ring buffer / ES: Construir array de puntos desde ring buffer
	const int32 PointCount = SampleCount;
	const float XStep = GraphWidth / static_cast<float>(FMath::Max(PointCount - 1, 1));
	const int32 ReadStart = (WriteIndex - SampleCount + BufferCapacity) % BufferCapacity;

	// EN: Resize before writing (prevents OOB if previous frame shrunk the array)
	// ES: Redimensionar antes de escribir (previene OOB si el frame anterior achico el array)
	CachedPoints.SetNum(PointCount, EAllowShrinking::No);

	for (int32 i = 0; i < PointCount; ++i)
	{
		const int32 Idx = (ReadStart + i) % BufferCapacity;
		const float Normalized = FMath::Clamp((RingBuffer[Idx] - MinValue) / Range, 0.0f, 1.0f);
		CachedPoints[i] = FVector2f(
			GraphLeft + XStep * static_cast<float>(i),
			LocalSize.Y * (1.0f - Normalized));
	}

	// EN: Draw AVG line / ES: Dibujar linea AVG
	if (bShowAvgLine && SampleCount > 0)
	{
		const float AvgNorm = FMath::Clamp((CachedAvg - MinValue) / Range, 0.0f, 1.0f);
		const float AvgY = LocalSize.Y * (1.0f - AvgNorm);

		AvgLinePoints[0] = FVector2f(GraphLeft, AvgY);
		AvgLinePoints[1] = FVector2f(LocalSize.X, AvgY);

		AvgLinePoints.SetNum(2, EAllowShrinking::No);
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 3,
			AllottedGeometry.ToPaintGeometry(),
			AvgLinePoints,
			ESlateDrawEffect::None,
			AvgLineColor,
			false,
			1.0f);
	}

	// EN: Draw main line (CachedPoints already sized to PointCount above)
	// ES: Dibujar linea principal (CachedPoints ya dimensionado a PointCount arriba)
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId + 4,
		AllottedGeometry.ToPaintGeometry(),
		CachedPoints,
		ESlateDrawEffect::None,
		LineColor,
		true,
		LineThickness);

	// EN: Draw peak markers (small dots at peaks > 80% range) / ES: Marcadores de pico (puntos en picos > 80% rango)
	if (bShowPeakMarkers)
	{
		const float PeakThreshold = MinValue + Range * 0.8f;
		const FSlateBrush* DotBrush = FAppStyle::GetBrush(TEXT("WhiteBrush"));

		for (int32 i = 0; i < PointCount; ++i)
		{
			const int32 Idx = (ReadStart + i) % BufferCapacity;
			if (RingBuffer[Idx] >= PeakThreshold)
			{
				const float DotSize = 4.0f;
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId + 5,
					AllottedGeometry.ToPaintGeometry(
						FVector2f(DotSize, DotSize),
						FSlateLayoutTransform(FVector2f(
							CachedPoints[i].X - DotSize * 0.5f,
							CachedPoints[i].Y - DotSize * 0.5f))),
					DotBrush,
					ESlateDrawEffect::None,
					PGX::Semantic::Error);
			}
		}
	}

	return LayerId + 5;
}
