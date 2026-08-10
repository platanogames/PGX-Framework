// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class SWidget;
class SPGXTelemetryGraph;

/**
 * EN: Editor utility functions for PGX panel UX standardization.
 *     Shared across all PGX editor panels to provide a consistent UX contract.
 * ES: Funciones utilitarias de editor para estandarizacion UX de paneles PGX.
 *     Compartidas entre todos los paneles de editor PGX para una UX consistente.
 */
namespace PGXEditorUtils
{
	/**
	 * EN: Extract the leaf segment from a GameplayTag (e.g., "PGX.Save.Context.PlayerData" -> "PlayerData").
	 *     Returns "(none)" for invalid tags.
	 * ES: Extraer el segmento hoja de un GameplayTag (ej: "PGX.Save.Context.PlayerData" -> "PlayerData").
	 *     Devuelve "(none)" para tags invalidos.
	 */
	PGXCOREEDITOR_API FString TagToLeafName(const FGameplayTag& Tag);

	/**
	 * EN: Format an absolute timestamp (FPlatformTime::Seconds()) as relative text ("3s ago", "2m ago").
	 *     Use .ToolTipText() on the widget to show the absolute time on hover.
	 * ES: Formatear un timestamp absoluto como texto relativo ("3s ago", "2m ago").
	 */
	PGXCOREEDITOR_API FText FormatRelativeTime(double AbsoluteSeconds);

	/**
	 * EN: Create a 16x16 colored square badge for system identity in panel headers.
	 *     Uses the canonical system color palette.
	 * ES: Crear un badge cuadrado coloreado 16x16 para identidad del sistema en headers de paneles.
	 */
	PGXCOREEDITOR_API TSharedRef<SWidget> MakeSystemColorBadge(const FLinearColor& Color);

	/**
	 * EN: Create an SProgressBar with overlaid percentage text.
	 *     Pattern: SOverlay[ SProgressBar + STextBlock ]. Canonical pattern from Test Dashboard.
	 * ES: Crear un SProgressBar con texto de porcentaje superpuesto.
	 */
	PGXCOREEDITOR_API TSharedRef<SWidget> MakeProgressBarWithText(
		TAttribute<TOptional<float>> Percent,
		TAttribute<FText> Label,
		const FLinearColor& FillColor = FLinearColor(0.2f, 0.6f, 0.2f));

	/**
	 * EN: Create an SProgressBar with automatic green/yellow/red color thresholds.
	 *     Green < WarningThreshold, Yellow < ErrorThreshold, Red >= ErrorThreshold.
	 *     For budget/progress displays across Profile, Platform Health, PSO.
	 * ES: Crear un SProgressBar con umbrales de color verde/amarillo/rojo automaticos.
	 */
	PGXCOREEDITOR_API TSharedRef<SWidget> MakeProgressBarThresholded(
		TAttribute<TOptional<float>> Percent,
		TAttribute<FText> Label,
		float WarningThreshold = 0.7f,
		float ErrorThreshold = 0.9f);

	/**
	 * EN: Build a compact graph legend row showing Current / Avg / Peak values.
	 *     Values update live via lambda bindings to the graph's GetCurrent/GetAverage/GetPeak.
	 * ES: Construir una fila de leyenda compacta mostrando valores Current / Avg / Peak.
	 *     Valores se actualizan en vivo via lambda bindings a GetCurrent/GetAverage/GetPeak del grafo.
	 */
	PGXCOREEDITOR_API TSharedRef<SWidget> BuildGraphLegend(
		const TSharedPtr<SPGXTelemetryGraph>& Graph,
		const FLinearColor& MainColor);
}
