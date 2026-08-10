// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "PGXMGOSTypes.h"

class UPGXGCObserverSubsystem;
class SPGXTelemetryGraph;

/**
 * EN: Slate widget for the PGX MGOS Inspector Panel.
 *     5 panels: Status, History, Class Report, Incidents, Baseline & Config.
 *     Connects to UPGXGCObserverSubsystem (Engine subsystem — always available).
 *
 * ES: Widget Slate para el Panel Inspector MGOS de PGX.
 *     5 paneles: Estado, Historial, Class Report, Incidentes, Baseline y Config.
 *     Se conecta a UPGXGCObserverSubsystem (subsistema de Engine — siempre disponible).
 */
class PGXEDITORTOOLSEDITOR_API SPGXMGOSInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXMGOSInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXMGOSInspectorTab() override;

private:
	// ── UI Builders ──────────────────────────────────────────────────────

	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildStatusPanel();
	TSharedRef<SWidget> BuildTelemetryPanel();
	TSharedRef<SWidget> BuildHistoryPanel();
	TSharedRef<SWidget> BuildClassReportPanel();
	TSharedRef<SWidget> BuildIncidentsPanel();
	TSharedRef<SWidget> BuildBaselinePanel();

	// ── Row Generators ───────────────────────────────────────────────────

	TSharedRef<ITableRow> OnGenerateHistoryRow(
		TSharedPtr<FPGXGCSnapshotDiff> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> OnGenerateClassReportRow(
		TSharedPtr<FPGXGCClassHealthReport> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> OnGenerateIncidentRow(
		TSharedPtr<FPGXGCIncident> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// ── Data Refresh ─────────────────────────────────────────────────────

	void RefreshAll();
	void RefreshKPIChips();

	// ── Button Handlers ──────────────────────────────────────────────────

	FReply OnRefreshClicked();
	FReply OnCaptureBaselineClicked();
	FReply OnResetBaselineClicked();
	FReply OnToggleSuppressClicked();

	// ── Delegate Lifecycle ───────────────────────────────────────────────

	void BindSubsystemDelegates();
	void UnbindSubsystemDelegates();

	// ── Delegate Callbacks ───────────────────────────────────────────────

	void OnGCCycleCompleted(uint64 CycleID, const FPGXGCSnapshot& Snapshot, const FPGXGCSnapshotDiff& Diff);
	void OnIncidentRaised(const FPGXGCIncident& Incident);
	void OnProfileChanged(EPGXGCProfileState NewState, EPGXGCProfileState OldState);

	// ── Helpers ──────────────────────────────────────────────────────────

	UPGXGCObserverSubsystem* GetSubsystem() const;
	static FLinearColor GetSeverityColor(EPGXGCSeverity Sev);
	static FLinearColor GetClassHealthColor(EPGXGCClassHealth Health);
	static FLinearColor GetProfileStateColor(EPGXGCProfileState State);

	// ── Data ─────────────────────────────────────────────────────────────

	// EN: KPI chips in toolbar / ES: Chips KPI en toolbar
	TSharedPtr<STextBlock> KPICyclesChip;
	TSharedPtr<STextBlock> KPIIncidentsChip;
	TSharedPtr<STextBlock> KPIMemoryChip;

	// Status widgets
	TSharedPtr<STextBlock> ModeText;
	TSharedPtr<STextBlock> BaselineStateText;
	TSharedPtr<STextBlock> ProfileStateText;
	TSharedPtr<STextBlock> CycleCountText;
	TSharedPtr<STextBlock> IncidentCountText;
	TSharedPtr<STextBlock> ProcessMemoryText;
	TSharedPtr<STextBlock> SuppressedText;
	TSharedPtr<STextBlock> BaselineInfoText;

	// History (SListView)
	TArray<TSharedPtr<FPGXGCSnapshotDiff>> HistoryItems;
	TSharedPtr<SListView<TSharedPtr<FPGXGCSnapshotDiff>>> HistoryListView;

	// Class Report (SListView)
	TArray<TSharedPtr<FPGXGCClassHealthReport>> ClassReportItems;
	TSharedPtr<SListView<TSharedPtr<FPGXGCClassHealthReport>>> ClassReportListView;
	TSharedPtr<STextBlock> ClassReportEmptyText;

	// Incidents (SListView)
	TArray<TSharedPtr<FPGXGCIncident>> IncidentItems;
	TSharedPtr<SListView<TSharedPtr<FPGXGCIncident>>> IncidentListView;

	// EN: Telemetry graphs / ES: Graficos de telemetria
	TSharedPtr<SPGXTelemetryGraph> GCDurationGraph;
	TSharedPtr<SPGXTelemetryGraph> MemoryDeltaGraph;

	// Delegate handles
	FDelegateHandle GCCompletedHandle;
	FDelegateHandle IncidentRaisedHandle;
	FDelegateHandle ProfileChangedHandle;

	// Status bar
	TSharedPtr<STextBlock> StatusBarWidget;
};
