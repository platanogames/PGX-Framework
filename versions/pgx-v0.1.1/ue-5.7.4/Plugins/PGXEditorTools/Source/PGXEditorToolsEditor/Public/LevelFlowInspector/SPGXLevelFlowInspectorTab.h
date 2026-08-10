// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "PGXLevelFlowTypes.h"

class UPGXLevelFlowSubsystem;
class FEditorModeTools;

// ============================================================================
// EN: Local display types for inspector panels.
// ES: Tipos locales de display para paneles del inspector.
// ============================================================================

/**
 * EN: Summary of a catalog entry for the Level Catalog panel.
 * ES: Resumen de una entrada del catalogo para el panel Level Catalog.
 */
struct FPGXLevelCatalogEntry
{
	FGameplayTag LevelTag;
	FText DisplayName;
	EPGXLoadStrategy LoadStrategy;
	EPGXTransitionMode TransitionMode;
	int32 SubLevelCount;
};

/**
 * EN: Sub-level status entry for the Sub-Levels panel.
 * ES: Entrada de estado de sub-nivel para el panel Sub-Levels.
 */
struct FPGXSubLevelStatusRow
{
	FGameplayTag SubLevelTag;
	bool bIsLoaded;
};

// ============================================================================
// EN: SPGXLevelFlowInspectorTab — 4-panel inspector for the LevelFlow system.
//     Panels: Transition Status | Level Catalog | Transition History | Sub-Levels
//
// ES: SPGXLevelFlowInspectorTab — inspector de 4 paneles para el sistema LevelFlow.
//     Paneles: Transition Status | Level Catalog | Transition History | Sub-Levels
// ============================================================================

class PGXEDITORTOOLSEDITOR_API SPGXLevelFlowInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXLevelFlowInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXLevelFlowInspectorTab() override;

private:
	// ── UI Builders ──────────────────────────────────────────────────────

	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildTransitionStatusPanel();
	TSharedRef<SWidget> BuildLevelCatalogPanel();
	TSharedRef<SWidget> BuildTransitionHistoryPanel();
	TSharedRef<SWidget> BuildSubLevelsPanel();

	// ── Row Generators ───────────────────────────────────────────────────

	TSharedRef<ITableRow> OnGenerateCatalogRow(
		TSharedPtr<FPGXLevelCatalogEntry> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> OnGenerateHistoryRow(
		TSharedPtr<FPGXLevelTransitionRecord> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> OnGenerateSubLevelRow(
		TSharedPtr<FPGXSubLevelStatusRow> Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	// ── Data Refresh ─────────────────────────────────────────────────────

	void RefreshAll();
	void RefreshTransitionStatus();
	void RefreshLevelCatalog();
	void RefreshTransitionHistory();
	void RefreshSubLevels();

	// ── PIE Lifecycle ────────────────────────────────────────────────────

	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);
	void BindToSubsystem();
	void UnbindFromSubsystem();

	// ── Delegate Callbacks ───────────────────────────────────────────────

	void OnLevelFlowStateChanged(EPGXLevelFlowState NewState, FGameplayTag LevelTag);
	void OnTransitionStarted(FGameplayTag LevelTag);
	void OnTransitionCompleted(FGameplayTag LevelTag);

	// ── Actions ─────────────────────────────────────────────────────────

	void RefreshKPIChips();

	// ── Helpers ──────────────────────────────────────────────────────────

	static FLinearColor GetStateColor(EPGXLevelFlowState State);
	static FText GetStateText(EPGXLevelFlowState State);
	static FText GetLoadStrategyText(EPGXLoadStrategy Strategy);
	static FText GetTransitionModeText(EPGXTransitionMode Mode);

	// ── Data ─────────────────────────────────────────────────────────────

	// EN: KPI chips in toolbar / ES: Chips KPI en toolbar
	TSharedPtr<STextBlock> KPIStateChip;
	TSharedPtr<STextBlock> KPILevelsChip;
	TSharedPtr<STextBlock> KPIProfilesChip;

	// EN: Cancel button / ES: Boton de cancelar
	TSharedPtr<SButton> CancelButton;

	// Transition status widgets
	TSharedPtr<STextBlock> StateTextWidget;
	TSharedPtr<STextBlock> CurrentLevelWidget;
	TSharedPtr<STextBlock> PreviousLevelWidget;
	TSharedPtr<STextBlock> ProgressWidget;
	TSharedPtr<STextBlock> TimingInfoWidget;

	// Level catalog
	TArray<TSharedPtr<FPGXLevelCatalogEntry>> CatalogEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXLevelCatalogEntry>>> CatalogListView;

	// Transition history
	TArray<TSharedPtr<FPGXLevelTransitionRecord>> HistoryEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXLevelTransitionRecord>>> HistoryListView;

	// Sub-levels
	TArray<TSharedPtr<FPGXSubLevelStatusRow>> SubLevelEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXSubLevelStatusRow>>> SubLevelListView;

	// Subsystem binding
	TWeakObjectPtr<UPGXLevelFlowSubsystem> BoundSubsystem;
	FDelegateHandle StateChangedHandle;
	FDelegateHandle TransitionStartedHandle;
	FDelegateHandle TransitionCompletedHandle;

	// PIE tracking
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	// Status bar
	TSharedPtr<STextBlock> StatusBarWidget;
};
