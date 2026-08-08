// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Testing/PGXTestResultCollector.h"
#include "TestDashboard/FPGXTestHarness.h"

/**
 * EN: PGX Test Dashboard — Slate panel for running and viewing system validation tests.
 *     Shows results in real-time, supports Run All / Clear / Export JSON / Export Markdown (EN/ES).
 *     Harness mode injects transient test data into 6 subsystems before test execution.
 *     Detects PIE state to enable/disable test execution.
 *
 * ES: PGX Test Dashboard — Panel Slate para ejecutar y visualizar tests de validacion de sistemas.
 *     Muestra resultados en tiempo real, soporta Run All / Clear / Export JSON / Export Markdown (EN/ES).
 *     Modo Harness inyecta datos transient en 6 subsistemas antes de la ejecucion de tests.
 *     Detecta estado PIE para habilitar/deshabilitar ejecucion de tests.
 */
class SPGXTestDashboardTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXTestDashboardTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXTestDashboardTab() override;

private:
	// EN: Build UI sections / ES: Construir secciones de UI
	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildStatusBar();
	TSharedRef<SWidget> BuildSummaryBar();
	TSharedRef<SWidget> BuildResultsArea();
	TSharedRef<SWidget> BuildSystemSection(const FPGXSystemTestSummary& Summary);
	TSharedRef<SWidget> BuildResultRow(const FPGXTestResult& Result);

	// EN: Actions / ES: Acciones
	FReply OnRunAllClicked();
	FReply OnClearClicked();
	FReply OnExportJsonClicked();
	FReply OnExportMarkdownClicked();
	FReply OnToggleLanguageClicked();
	void OnRunSystemClicked(const FString& SystemName);

	// EN: Callbacks / ES: Callbacks
	void OnResultsChanged();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);

	// EN: Refresh the results display / ES: Refrescar la visualizacion de resultados
	void RefreshResults();

	// EN: Get PIE world if available / ES: Obtener PIE world si esta disponible
	UWorld* GetPIEWorld() const;

	// EN: System color lookup / ES: Busqueda de color por sistema
	static FLinearColor GetSystemColor(const FString& SystemName);

	// EN: Cached state / ES: Estado cacheado
	TSharedPtr<SVerticalBox> ResultsBox;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<SWidget> PassedChip;
	TSharedPtr<SWidget> FailedChip;
	TSharedPtr<SProgressBar> SummaryProgress;
	TSharedPtr<STextBlock> LastRunText;
	FText CachedPassedValue;
	FText CachedFailedValue;
	FText CachedFooterText;

	// EN: Delegate handles / ES: Handles de delegados
	FDelegateHandle CollectorChangedHandle;
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	bool bIsPIEActive = false;

	// EN: Filter state / ES: Estado de filtro
	FString FilterText;
	bool bShowFailedOnly = false;

	// EN: Last run timestamp / ES: Timestamp de ultima ejecucion
	FDateTime LastRunTimestamp;

	// EN: Test harness for automatic data injection / ES: Harness de test para inyeccion automatica de datos
	FPGXTestHarness TestHarness;
	bool bHarnessMode = false;

	// EN: Report language for Markdown export / ES: Idioma de reporte para exportacion Markdown
	EPGXReportLanguage ReportLanguage = EPGXReportLanguage::English;
};
