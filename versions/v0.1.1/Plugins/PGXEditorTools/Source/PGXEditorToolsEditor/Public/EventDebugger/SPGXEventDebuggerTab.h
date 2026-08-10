// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "EventHandler/PGXEventHandlerTypes.h"

class UPGXEventHandlerSubsystem;
class SWidgetSwitcher;
class SPGXTelemetryGraph;
class SPGXSparkline;

/**
 * EN: Row data for Handler Browser table.
 * ES: Datos de fila para tabla del Browser de Handlers.
 */
struct FPGXHandlerRowData
{
	FString LeafTag;
	FString FullTag;
	FString ClassName;
	bool bCached = false;
	bool bEnabled = true;
	FString CategoryLeaf;
	FString CategoryFull;
	FString LifecycleName;
};

/**
 * EN: Row data for Telemetry table.
 * ES: Datos de fila para tabla de Telemetria.
 */
struct FPGXTelemetryRowData
{
	FString LeafTag;
	FString FullTag;
	int32 ExecutionCount = 0;
	int32 SuccessCount = 0;
	int32 FailCount = 0;
	int32 SkipCount = 0;
	float AvgMs = 0.0f;
	float MaxMs = 0.0f;
};

/**
 * EN: Row data for Blackbox History table (structured replacement of text dump).
 * ES: Datos de fila para tabla de Historial Blackbox (reemplazo estructurado del dump de texto).
 */
struct FPGXBlackboxRowData
{
	double Timestamp = 0.0;
	FString LeafTag;
	FString FullTag;
	FString ResultText;
	FLinearColor ResultColor = FLinearColor::White;
	FString HandlerClassName;
	float ExecutionTimeMs = 0.0f;
	FString InstigatorName;
};

/**
 * EN: PGX Event Debugger — NomadTab for inspecting the Event Handler System.
 *     v1.0 views: Handler Browser (SListView), Telemetry Dashboard (SListView), Blackbox History (SListView).
 *     Universal shell: 2px color bar + KPI chips + footer PIE state.
 *     Delegate-driven: OnHandlerExecutedNative for real-time blackbox updates.
 *     v1.1 planned: Execution Waterfall, Pragmatism Map, Lifecycle Visualizer.
 * ES: PGX Event Debugger — NomadTab para inspeccionar el Sistema de Event Handler.
 *     Vistas v1.0: Browser de Handlers, Dashboard de Telemetria, Historial Blackbox (todas SListView).
 *     Shell universal: barra de color 2px + chips KPI + footer estado PIE.
 *     Delegate-driven: OnHandlerExecutedNative para updates en tiempo real del blackbox.
 */
class SPGXEventDebuggerTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXEventDebuggerTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXEventDebuggerTab() override;
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TSharedRef<SWidget> BuildHandlerBrowserSection();
	TSharedRef<SWidget> BuildGraphSection();
	TSharedRef<SWidget> BuildTelemetrySection();
	TSharedRef<SWidget> BuildBlackboxSection();
	TSharedRef<SWidget> BuildActionsSection();

	UPGXEventHandlerSubsystem* GetEventHandlerSubsystem() const;
	void RefreshData();

	// EN: PIE lifecycle / ES: Ciclo de vida PIE
	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);

	// EN: Subsystem delegate binding / ES: Binding de delegates del subsistema
	void BindToSubsystem();
	void UnbindFromSubsystem();

	// EN: Delegate callbacks / ES: Callbacks de delegates
	void OnHandlerExecuted(FGameplayTag EventTag, EPGXEventResult Result, double ExecutionTimeMs);

	// EN: Row generation for tables / ES: Generacion de filas para tablas
	TSharedRef<ITableRow> OnGenerateHandlerRow(TSharedPtr<FPGXHandlerRowData> Item, const TSharedRef<STableViewBase>& Owner);
	TSharedRef<ITableRow> OnGenerateTelemetryRow(TSharedPtr<FPGXTelemetryRowData> Item, const TSharedRef<STableViewBase>& Owner);
	TSharedRef<ITableRow> OnGenerateBlackboxRow(TSharedPtr<FPGXBlackboxRowData> Item, const TSharedRef<STableViewBase>& Owner);

	// EN: Content switcher (empty state vs live) / ES: Switcher de contenido (estado vacio vs live)
	TSharedPtr<SWidgetSwitcher> ContentSwitcher;
	TSharedPtr<STextBlock> FooterStatusText;

	// EN: PIE delegate handles / ES: Handles de delegates PIE
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	// EN: Subsystem delegate handles / ES: Handles de delegates del subsistema
	FDelegateHandle HandlerExecutedHandle;

	// EN: Overview metrics / ES: Metricas del resumen
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> RegisteredText;
	TSharedPtr<STextBlock> CachedText;
	TSharedPtr<STextBlock> HitRatioText;
	TSharedPtr<STextBlock> EvictionsText;

	// EN: Handler Browser table / ES: Tabla del Browser de Handlers
	TArray<TSharedPtr<FPGXHandlerRowData>> HandlerItems;
	TSharedPtr<SListView<TSharedPtr<FPGXHandlerRowData>>> HandlerListView;

	// EN: Telemetry table / ES: Tabla de Telemetria
	TArray<TSharedPtr<FPGXTelemetryRowData>> TelemetryItems;
	TSharedPtr<SListView<TSharedPtr<FPGXTelemetryRowData>>> TelemetryListView;

	// EN: Blackbox table / ES: Tabla de Blackbox
	TArray<TSharedPtr<FPGXBlackboxRowData>> BlackboxItems;
	TSharedPtr<SListView<TSharedPtr<FPGXBlackboxRowData>>> BlackboxListView;

	// EN: Telemetry graphs / ES: Graficos de telemetria
	TSharedPtr<SPGXTelemetryGraph> ExecTimeGraph;
	TSharedPtr<SPGXSparkline> CacheHitSparkline;

	float RefreshAccumulator = 0.0f;
	static constexpr float RefreshInterval = 0.5f;
};
