// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Widgets/SPGXSparkline.h"
#include "Rendering/DrawElements.h"

void SPGXSparkline::Construct(const FArguments& InArgs)
{
	LineColor = InArgs._LineColor;
	LineThickness = InArgs._LineThickness;

	RingBuffer.SetNumZeroed(BufferCapacity);
	CachedPoints.SetNumUninitialized(BufferCapacity);
	WriteIndex = 0;
	SampleCount = 0;
}

void SPGXSparkline::PushValue(float Value)
{
	RingBuffer[WriteIndex] = Value;
	WriteIndex = (WriteIndex + 1) % BufferCapacity;
	SampleCount = FMath::Min(SampleCount + 1, BufferCapacity);
}

void SPGXSparkline::Clear()
{
	FMemory::Memzero(RingBuffer.GetData(), BufferCapacity * sizeof(float));
	WriteIndex = 0;
	SampleCount = 0;
}

FVector2D SPGXSparkline::ComputeDesiredSize(float) const
{
	return FVector2D(40.0f, PGX::Height::SparklineDefault);
}

int32 SPGXSparkline::OnPaint(const FPaintArgs& /*Args*/, const FGeometry& AllottedGeometry,
	const FSlateRect& /*MyCullingRect*/, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& /*InWidgetStyle*/, bool /*bParentEnabled*/) const
{
	if (SampleCount < 2)
	{
		return LayerId;
	}

	const FVector2f LocalSize(
		static_cast<float>(AllottedGeometry.GetLocalSize().X),
		static_cast<float>(AllottedGeometry.GetLocalSize().Y));

	// EN: Auto-scale Y from ring buffer min/max / ES: Auto-escalar Y desde min/max del ring buffer
	const int32 ReadStart = (WriteIndex - SampleCount + BufferCapacity) % BufferCapacity;
	float MinVal = FLT_MAX;
	float MaxVal = -FLT_MAX;
	for (int32 i = 0; i < SampleCount; ++i)
	{
		const float Val = RingBuffer[(ReadStart + i) % BufferCapacity];
		MinVal = FMath::Min(MinVal, Val);
		MaxVal = FMath::Max(MaxVal, Val);
	}

	const float Range = FMath::Max(MaxVal - MinVal, SMALL_NUMBER);
	const float XStep = LocalSize.X / static_cast<float>(FMath::Max(SampleCount - 1, 1));

	// EN: Resize before writing (prevents OOB if previous frame shrunk the array)
	// ES: Redimensionar antes de escribir (previene OOB si el frame anterior achico el array)
	CachedPoints.SetNum(SampleCount, EAllowShrinking::No);

	// EN: Build point array / ES: Construir array de puntos
	for (int32 i = 0; i < SampleCount; ++i)
	{
		const float Val = RingBuffer[(ReadStart + i) % BufferCapacity];
		const float Normalized = FMath::Clamp((Val - MinVal) / Range, 0.0f, 1.0f);
		CachedPoints[i] = FVector2f(
			XStep * static_cast<float>(i),
			LocalSize.Y * (1.0f - Normalized));
	}

	// EN: Draw line (CachedPoints already sized to SampleCount above)
	// ES: Dibujar linea (CachedPoints ya dimensionado a SampleCount arriba)
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		CachedPoints,
		ESlateDrawEffect::None,
		LineColor,
		true,
		LineThickness);

	return LayerId;
}
