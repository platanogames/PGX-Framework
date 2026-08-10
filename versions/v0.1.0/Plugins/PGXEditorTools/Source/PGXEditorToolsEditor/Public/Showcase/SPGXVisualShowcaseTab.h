// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Visual Showcase Tab — AAA standard validation of PGX visual system.
// ES: Tab de Visual Showcase — validacion estandar AAA del sistema visual PGX.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SPGXTelemetryGraph;

// EN: Density mode for layout validation / ES: Modo densidad para validacion de layout
enum class EPGXShowcaseDensity : uint8
{
	Default,
	Compact
};

// EN: Theme mode for token validation / ES: Modo theme para validacion de tokens
enum class EPGXShowcaseTheme : uint8
{
	Obsidian,
	ValidationLight
};

// EN: Health filter for System Health section / ES: Filtro de salud para seccion System Health
enum class EPGXHealthFilter : uint8
{
	All,
	Active,
	Warning,
	Error
};

// EN: Event filter for Recent Events section / ES: Filtro de eventos para seccion Recent Events
enum class EPGXEventFilter : uint8
{
	All,
	OK,
	Warning,
	Error
};

// EN: Telemetry time range / ES: Rango temporal de telemetria
enum class EPGXTelemetryRange : uint8
{
	Seconds30,
	Minutes2,
	Minutes5,
	Live
};

/**
 * EN: AAA Showcase panel — validates PGX visual system architecture.
 *     Exercises all atomic components, density mode, theme switching,
 *     and governance compliance with 100% fake data.
 *
 * ES: Panel Showcase AAA — valida arquitectura del sistema visual PGX.
 *     Ejercita todos los componentes atomicos, modo densidad, cambio de theme,
 *     y cumplimiento de gobernanza con 100% datos falsos.
 */
class PGXEDITORTOOLSEDITOR_API SPGXVisualShowcaseTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXVisualShowcaseTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// ─── DATA STRUCTS ───
	struct FShowcaseSystemEntry
	{
		FText Name;
		FLinearColor Color;
		FText Status;
		FLinearColor StatusColor;
		float BudgetValue;
		float BudgetMax;
		bool bUnlimited;
		int32 SeverityOrder; // EN: 0=Error, 1=Warning, 2=Active / ES: 0=Error, 1=Warning, 2=Activo
	};

	struct FShowcaseEventEntry
	{
		int32 Id;
		FText System;
		FText Event;
		FText Time;
		FText Status;
		FLinearColor StatusColor;
		int32 SeverityOrder; // EN: 0=Error, 1=Warning, 2=OK / ES: 0=Error, 1=Warning, 2=OK
	};

	struct FShowcaseIncident
	{
		FText System;
		FText Description;
		FLinearColor Color;
	};

	// ─── STATE ───
	EPGXShowcaseDensity CurrentDensity = EPGXShowcaseDensity::Default;
	EPGXShowcaseTheme CurrentTheme = EPGXShowcaseTheme::Obsidian;
	EPGXHealthFilter CurrentHealthFilter = EPGXHealthFilter::All;
	EPGXEventFilter CurrentEventFilter = EPGXEventFilter::All;
	EPGXTelemetryRange CurrentTelemetryRange = EPGXTelemetryRange::Live;

	// ─── DATA ───
	TArray<TSharedPtr<FShowcaseSystemEntry>> SystemEntries;
	TArray<TSharedPtr<FShowcaseEventEntry>> EventEntries;
	TArray<FShowcaseIncident> Incidents;
	TSharedPtr<SPGXTelemetryGraph> GCGraph;
	TSharedPtr<SPGXTelemetryGraph> FPSGraph;

	// EN: Root container for rebuild on toggle / ES: Container raiz para rebuild en toggle
	TSharedPtr<SVerticalBox> RootContent;

	// ─── BUILD METHODS ───
	void RebuildContent();
	TSharedRef<SWidget> BuildCommandHeader();
	TSharedRef<SWidget> BuildHealthSummaryLine();
	TSharedRef<SWidget> BuildIncidentBlock();
	TSharedRef<SWidget> BuildKPIStrip();
	TSharedRef<SWidget> BuildSystemHealthSection();
	TSharedRef<SWidget> BuildTelemetrySection();
	TSharedRef<SWidget> BuildRecentEventsSection();
	TSharedRef<SWidget> BuildEmptyStateSection();
	TSharedRef<SWidget> BuildFooterBar();

	// ─── HELPERS ───
	void PopulateFakeData();
	void PopulateFakeGraphData();

	// EN: Density-aware helpers / ES: Helpers conscientes de densidad
	float GetRowHeight() const;
	float GetBodyFontSize() const;
	float GetPaddingScale() const;
	FSlateFontInfo GetBodyFont() const;
	FSlateFontInfo GetCaptionFont() const;

	// EN: Theme-aware helpers / ES: Helpers conscientes de theme
	FLinearColor GetSurfaceBase() const;
	FLinearColor GetSurfaceRaised() const;
	FLinearColor GetTextPrimary() const;
	FLinearColor GetBorderDefault() const;

	// EN: Filter helpers / ES: Helpers de filtro
	TSharedRef<SWidget> BuildFilterButton(const FText& Label, bool bActive, FOnClicked OnClick);

	// EN: Health counters / ES: Contadores de salud
	int32 CountByHealth(int32 SeverityOrder) const;
};
