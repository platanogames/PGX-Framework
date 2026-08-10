// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Logging/PGXLogTypes.h"
#include "GameplayTagContainer.h"

class UPGXLogSubsystem;
class UPGXLogDomainRendererBase;
class SWindow;

/**
 * EN: View item wrapping a log entry with its cached domain renderer.
 * ES: Item de vista que envuelve una entrada de log con su renderer de dominio cacheado.
 */
struct FPGXLogViewItem
{
	TSharedPtr<FPGXLogEntry> Entry;
	TWeakObjectPtr<UPGXLogDomainRendererBase> Renderer;
};

/**
 * EN: PGX Log Viewer v3.0 — Polymorphic Observability Viewer.
 *     - Domain-aware rendering: each system has its own columns and colors
 *     - Domain filter bar with toggle buttons and entry counts
 *     - Live / Manual refresh modes with delta badge
 *     - Detail windows (double-click) that survive PIE with STALE/LIVE indicator
 *     - Backward compatible: entries without DomainTag use auto-infer + generic renderer
 *
 * ES: PGX Log Viewer v3.0 — Viewer de Observabilidad Polimorfico.
 */
class PGXEDITORTOOLSEDITOR_API SPGXLogViewerTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXLogViewerTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXLogViewerTab() override;

private:
	// ═══════════════════════════════════════
	// UI Build / Construccion de UI
	// ═══════════════════════════════════════

	TSharedRef<SWidget> BuildToolbar();
	TSharedRef<SWidget> BuildVerbosityFilters();
	TSharedRef<SWidget> BuildDomainFilterBar();
	TSharedRef<SWidget> BuildSessionInfoBar();
	TSharedRef<SWidget> BuildStatusBar();
	TSharedRef<SWidget> BuildColumnHeaders();

	// ═══════════════════════════════════════
	// SListView / SListView
	// ═══════════════════════════════════════

	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FPGXLogViewItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnRowDoubleClicked(TSharedPtr<FPGXLogViewItem> Item);

	// ═══════════════════════════════════════
	// Search & Filter / Busqueda y Filtrado
	// ═══════════════════════════════════════

	void ParseSearchInput(const FText& NewText);
	bool MatchesTextSearch(const FPGXLogEntry& Entry) const;
	bool PassesFilter(const FPGXLogViewItem& Item) const;
	void RebuildFilteredList();
	void UpdateStatusBar();
	void UpdateSessionInfo();
	void RebuildDomainCounts();

	// ═══════════════════════════════════════
	// Refresh Mode / Modo de Refresco
	// ═══════════════════════════════════════

	void FlushDeltaBuffer();
	void ToggleRefreshMode();

	// ═══════════════════════════════════════
	// Detail Windows / Ventanas de Detalle
	// ═══════════════════════════════════════

	void OpenDetailWindow(TSharedPtr<FPGXLogViewItem> Item);
	void CleanupDetailWindows();
	void MarkDetailWindowsStale();

	// ═══════════════════════════════════════
	// PIE Lifecycle / Ciclo de Vida PIE
	// ═══════════════════════════════════════

	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);
	void BindToSubsystem();
	void UnbindFromSubsystem();

	// ═══════════════════════════════════════
	// Log Entry Callback / Callback de Entrada de Log
	// ═══════════════════════════════════════

	void OnNewLogEntry(const FPGXLogEntry& Entry);

	// ═══════════════════════════════════════
	// Actions / Acciones
	// ═══════════════════════════════════════

	FReply OnExportClicked();
	FReply OnImportClicked();
	FReply OnClearClicked();

	// EN: Category filter dropdown / ES: Dropdown de filtro por categoria
	void RebuildCategoryOptions();
	TSharedRef<SWidget> GenerateCategoryComboItem(TSharedPtr<FName> Item);
	void OnCategorySelected(TSharedPtr<FName> Item, ESelectInfo::Type SelectType);
	FText GetCurrentCategoryText() const;

	// EN: Action feedback / ES: Feedback de accion
	void UpdateActionFeedback(bool bSuccess, const FString& Detail);

	// EN: Get renderer for an entry / ES: Obtener renderer para una entrada
	UPGXLogDomainRendererBase* ResolveRenderer(const FPGXLogEntry& Entry);

	// ═══════════════════════════════════════
	// State / Estado
	// ═══════════════════════════════════════

	// EN: All entries with cached renderers / ES: Todas las entradas con renderers cacheados
	TArray<TSharedPtr<FPGXLogViewItem>> AllItems;

	// EN: Filtered items (what SListView shows) / ES: Items filtrados (lo que muestra SListView)
	TArray<TSharedPtr<FPGXLogViewItem>> FilteredItems;

	// EN: Delta buffer for Manual mode / ES: Buffer delta para modo Manual
	TArray<TSharedPtr<FPGXLogViewItem>> DeltaBuffer;

	TSharedPtr<SListView<TSharedPtr<FPGXLogViewItem>>> ListView;

	// EN: Active filter state / ES: Estado de filtros activos
	FString SearchText;
	FGameplayTag SearchTag;
	FName ActiveCategoryFilter = NAME_None;
	TMap<EPGXLogVerbosity, bool> VerbosityFilters;
	TSet<FName> KnownCategories;
	bool bAutoScroll = true;

	// EN: Domain filter state / ES: Estado de filtro de dominio
	TMap<FGameplayTag, bool> DomainVisibility;
	TMap<FGameplayTag, int32> DomainCounts;

	// EN: Refresh mode / ES: Modo de refresco
	EPGXLogRefreshMode RefreshMode = EPGXLogRefreshMode::Live;

	// EN: Category filter UI / ES: UI de filtro de categoria
	TSharedPtr<SComboBox<TSharedPtr<FName>>> CategoryComboBox;
	TArray<TSharedPtr<FName>> CategoryOptions;
	int32 LastCategoryCount = 0;

	// EN: Imported session metadata / ES: Metadatos de sesion importada
	FPGXLogSession ImportedSession;
	bool bViewingImported = false;

	// EN: PIE state / ES: Estado PIE
	TWeakObjectPtr<UPGXLogSubsystem> BoundSubsystem;
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;
	FDelegateHandle LogEntryHandle;
	bool bPIEActive = false;

	// EN: Detail windows tracking / ES: Tracking de ventanas de detalle
	TArray<TWeakPtr<SWindow>> OpenDetailWindows;

	// EN: UI references / ES: Referencias de UI
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> SessionInfoText;
	TSharedPtr<STextBlock> DeltaBadgeText;
	TSharedPtr<STextBlock> ModeBadgeText;
	TSharedPtr<SHorizontalBox> DomainFilterBox;
	TSharedPtr<SVerticalBox> MainContent;
};
