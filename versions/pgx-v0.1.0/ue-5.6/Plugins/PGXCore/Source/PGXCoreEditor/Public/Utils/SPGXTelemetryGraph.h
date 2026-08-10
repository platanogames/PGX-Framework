// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Ring-buffer telemetry graph widget — O(1) push, single MakeLines draw call.
// ES: Widget de grafico de telemetria con ring buffer — O(1) push, un solo MakeLines draw call.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "Style/PGXVisualTokens.h"

/**
 * EN: Lightweight real-time line graph using a circular buffer.
 *     Renders via FSlateDrawElement::MakeLines() for GPU-direct performance.
 *     Pre-allocates all memory — zero allocations in OnPaint/Tick.
 *     Supports grid lines, AVG line, and peak markers.
 *
 * ES: Grafico de linea en tiempo real liviano usando buffer circular.
 *     Renderiza via FSlateDrawElement::MakeLines() para performance GPU-directa.
 *     Pre-aloca toda la memoria — cero allocations en OnPaint/Tick.
 *     Soporta lineas de grid, linea AVG, y marcadores de pico.
 */
class PGXCOREEDITOR_API SPGXTelemetryGraph : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXTelemetryGraph)
		: _BufferSize(256)
		, _LineColor(PGX::Semantic::Info)
		, _LineThickness(1.5f)
		, _MinValue(0.0f)
		, _MaxValue(100.0f)
		, _BackgroundColor(PGX::Surface::Void)
		, _bShowGrid(false)
		, _bShowAvgLine(false)
		, _bShowPeakMarkers(false)
		, _bShowLabels(false)
		, _GridColor(PGX::Border::Subtle)
		, _AvgLineColor(PGX::Semantic::Warn)
	{}
		SLATE_ARGUMENT(int32, BufferSize)
		SLATE_ARGUMENT(FLinearColor, LineColor)
		SLATE_ARGUMENT(float, LineThickness)
		SLATE_ARGUMENT(float, MinValue)
		SLATE_ARGUMENT(float, MaxValue)
		SLATE_ARGUMENT(FLinearColor, BackgroundColor)
		SLATE_ARGUMENT(bool, bShowGrid)
		SLATE_ARGUMENT(bool, bShowAvgLine)
		SLATE_ARGUMENT(bool, bShowPeakMarkers)
		SLATE_ARGUMENT(bool, bShowLabels)
		SLATE_ARGUMENT(FLinearColor, GridColor)
		SLATE_ARGUMENT(FLinearColor, AvgLineColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** EN: Push a new value into the ring buffer / ES: Insertar un nuevo valor en el ring buffer */
	void PushValue(float Value);

	/** EN: Clear all values / ES: Limpiar todos los valores */
	void Clear();

	/** EN: Get current average value / ES: Obtener valor promedio actual */
	float GetAverage() const;

	/** EN: Get peak value / ES: Obtener valor pico */
	float GetPeak() const;

	/** EN: Get most recent value / ES: Obtener valor mas reciente */
	float GetCurrent() const;

	// SWidget interface
	FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TArray<float> RingBuffer;
	int32 WriteIndex = 0;
	int32 SampleCount = 0;
	int32 BufferCapacity = 256;

	FLinearColor LineColor;
	float LineThickness;
	float MinValue;
	float MaxValue;
	FLinearColor BackgroundColor;

	// EN: Overlay features / ES: Caracteristicas de overlay
	bool bShowGrid = false;
	bool bShowAvgLine = false;
	bool bShowPeakMarkers = false;
	bool bShowLabels = false;
	FLinearColor GridColor;
	FLinearColor AvgLineColor;

	// EN: Cached stats (updated on push) / ES: Stats cacheadas (actualizadas en push)
	mutable float CachedAvg = 0.0f;
	mutable float CachedPeak = 0.0f;
	mutable bool bStatsDirty = true;
	void UpdateCachedStats() const;

	// EN: Pre-allocated point arrays for OnPaint (mutable for const OnPaint)
	mutable TArray<FVector2f> CachedPoints;
	mutable TArray<FVector2f> AvgLinePoints;
	mutable TArray<FVector2f> GridLinePoints;
};
