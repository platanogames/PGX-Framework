// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "PGXLoadingTypes.h"

class UPGXLoadingSubsystem;
class SPGXTelemetryGraph;

// ============================================================================
// EN: Local display types for inspector panels.
// ES: Tipos locales de display para paneles del inspector.
// ============================================================================

/**
 * EN: Summary of a loading profile for the Profile Catalog panel.
 * ES: Resumen de un perfil de carga para el panel Profile Catalog.
 */
struct FPGXLoadingProfileEntry
{
	FString ProfileName;
	TArray<FGameplayTag> ContextTags;
	EPGXLoadingVisualType VisualType;
	bool bWaitForPSO;
	float MinDisplayTime;
	bool bAllowSkip;
};

// ============================================================================
// EN: SPGXLoadingInspectorTab — 4-panel inspector for the Loading Screen system.
//     Panels: Current Status | Profile Catalog | Loading History | Debug Controls
//
// ES: SPGXLoadingInspectorTab — inspector de 4 paneles para el sistema Loading Screen.
//     Paneles: Current Status | Profile Catalog | Loading History | Debug Controls
// ============================================================================

class PGXEDITORTOOLSEDITOR_API SPGXLoadingInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXLoadingInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXLoadingInspectorTab() override;

private:
	// ── UI Builders ──────────────────────────────────────────────────────

	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildCurrentStatusPanel();
	TSharedRef<SWidget> BuildProfileCatalogPanel();
	TSharedRef<SWidget> BuildLoadingHistoryPanel();
	TSharedRef<SWidget> BuildDebugControlsPanel();
	TSharedRef<SWidget> BuildTelemetrySection();

	// ── Row Generators ───────────────────────────────────────────────────

	TSharedRef<ITableRow> OnGenerateProfileRow(
		TSharedPtr<FPGXLoadingProfileEntry> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> OnGenerateHistoryRow(
		TSharedPtr<FPGXLoadingRecord> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// ── Data Refresh ─────────────────────────────────────────────────────

	void RefreshAll();
	void RefreshCurrentStatus();
	void RefreshProfileCatalog();
	void RefreshLoadingHistory();

	// ── PIE Lifecycle ────────────────────────────────────────────────────

	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);
	void BindToSubsystem();
	void UnbindFromSubsystem();

	// ── Delegate Callbacks ───────────────────────────────────────────────

	void OnLoadingStateChanged(EPGXLoadingScreenState NewState, EPGXLoadingScreenState OldState);
	void OnLoadingStarted(FGameplayTag ContextTag);
	void OnLoadingProgress(float Progress, const FText& Status);
	void OnLoadingCompleted(const FPGXLoadingRecord& Record);

	// ── KPI ──────────────────────────────────────────────────────────────

	void RefreshKPIChips();

	// ── Helpers ──────────────────────────────────────────────────────────

	static FLinearColor GetStateColor(EPGXLoadingScreenState State);
	static FText GetStateText(EPGXLoadingScreenState State);
	static FText GetVisualTypeText(EPGXLoadingVisualType Type);
	static FText GetResultCodeText(EPGXLoadingResultCode Code);

	// ── Data ─────────────────────────────────────────────────────────────

	// EN: KPI chips in toolbar / ES: Chips KPI en toolbar
	TSharedPtr<STextBlock> KPIStateChip;
	TSharedPtr<STextBlock> KPIProfilesChip;
	TSharedPtr<STextBlock> KPIHistoryChip;

	// Current status widgets
	TSharedPtr<STextBlock> StateTextWidget;
	TSharedPtr<STextBlock> ContextWidget;
	TSharedPtr<STextBlock> ElapsedWidget;
	TSharedPtr<STextBlock> ProgressWidget;
	TSharedPtr<STextBlock> PSOProgressWidget;
	TSharedPtr<STextBlock> VisualTypeWidget;
	TSharedPtr<STextBlock> CloseConditionsWidget;

	// Profile catalog
	TArray<TSharedPtr<FPGXLoadingProfileEntry>> ProfileEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXLoadingProfileEntry>>> ProfileListView;

	// Loading history
	TArray<TSharedPtr<FPGXLoadingRecord>> HistoryEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXLoadingRecord>>> HistoryListView;

	// EN: Telemetry graphs / ES: Graficos de telemetria
	TSharedPtr<SPGXTelemetryGraph> ProgressGraph;
	TSharedPtr<SPGXTelemetryGraph> PSOProgressGraph;

	// Subsystem binding
	TWeakObjectPtr<UPGXLoadingSubsystem> BoundSubsystem;
	FDelegateHandle StateChangedHandle;
	FDelegateHandle LoadingStartedHandle;
	FDelegateHandle ProgressHandle;
	FDelegateHandle CompletedHandle;

	// PIE tracking
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	// Status bar
	TSharedPtr<STextBlock> StatusBarWidget;
};
