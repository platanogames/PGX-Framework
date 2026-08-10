// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Sparkline mini-graph — compact ring-buffer visualization for KPI chips and table cells.
// ES: Sparkline mini-grafico — visualizacion compacta con ring buffer para KPI chips y celdas de tabla.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "Style/PGXVisualTokens.h"

/**
 * EN: Compact sparkline widget (40x12px default) using a 32-sample ring buffer.
 *     Auto-scaling Y axis, single MakeLines() draw call, zero allocations per frame.
 *     Designed for embedding in SPGXKPIChip, table cells, and inline metrics.
 *
 * ES: Widget sparkline compacto (40x12px default) usando ring buffer de 32 muestras.
 *     Auto-escalado eje Y, un solo MakeLines() draw call, cero allocations por frame.
 *     Disenado para embeber en SPGXKPIChip, celdas de tabla y metricas inline.
 */
class PGXCOREEDITOR_API SPGXSparkline : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXSparkline)
		: _LineColor(PGX::Semantic::Info)
		, _LineThickness(1.0f)
	{}
		SLATE_ARGUMENT(FLinearColor, LineColor)
		SLATE_ARGUMENT(float, LineThickness)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// EN: Push a value into the ring buffer / ES: Empujar valor al ring buffer
	void PushValue(float Value);

	// EN: Clear all values / ES: Limpiar todos los valores
	void Clear();

protected:
	FVector2D ComputeDesiredSize(float) const override;
	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	FLinearColor LineColor;
	float LineThickness = 1.0f;

	static constexpr int32 BufferCapacity = 32;
	TArray<float> RingBuffer;
	int32 WriteIndex = 0;
	int32 SampleCount = 0;
	mutable TArray<FVector2f> CachedPoints;
};
