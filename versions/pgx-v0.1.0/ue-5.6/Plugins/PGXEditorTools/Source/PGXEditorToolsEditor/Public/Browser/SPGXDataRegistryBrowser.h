// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "GameplayTagContainer.h"
#include "Registry/PGXRegistryTypes.h"

class UPGXDataRegistrySubsystem;

/**
 * EN: Browser for Object DataAssets registered in the Data Registry.
 *     Left panel: database list with stats. Right panel: entries with search/filter + SHeaderRow.
 *     Status bar with aggregate stats. Auto-refresh via subsystem delegates during PIE.
 * ES: Browser para Object DataAssets registrados en el Data Registry.
 *     Panel izquierdo: lista de databases con stats. Panel derecho: entries con busqueda/filtro + SHeaderRow.
 *     Barra de estado con stats agregadas. Auto-refresh via delegados del subsistema durante PIE.
 */
class PGXEDITORTOOLSEDITOR_API SPGXDataRegistryBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXDataRegistryBrowser) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXDataRegistryBrowser() override;

private:
	// ── UI Build ─────────────────────────────────────────────────────────
	TSharedRef<SWidget> BuildDatabaseListPanel();
	TSharedRef<SWidget> BuildEntriesPanel();
	TSharedRef<SWidget> BuildStatusBar();

	// ── Row generators ───────────────────────────────────────────────────
	TSharedRef<ITableRow> OnGenerateDatabaseRow(TSharedPtr<FGameplayTag> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> OnGenerateEntryRow(TSharedPtr<FPGXRegistryEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);

	// ── Actions ──────────────────────────────────────────────────────────
	void OnDatabaseSelected(TSharedPtr<FGameplayTag> Item, ESelectInfo::Type SelectInfo);
	void OnSearchTextChanged(const FText& NewText);
	FReply OnRefreshClicked();

	// ── PIE Lifecycle ────────────────────────────────────────────────────
	void BindPIEDelegates();
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);

	// ── Subsystem Delegates ──────────────────────────────────────────────
	void BindToSubsystem();
	void UnbindFromSubsystem();
	void OnAssetRegistered(const FGameplayTag& DbTag, const FGameplayTag& ItemTag);
	void OnAssetUnregistered(const FGameplayTag& DbTag, const FGameplayTag& ItemTag);
	void OnDatabaseCreated(const FGameplayTag& DbTag);
	void OnCacheInvalidated(const FGameplayTag& DbTag);
	void OnTablesIngested(const FGameplayTag& DbTag, int32 EntryCount);

	// ── Data ─────────────────────────────────────────────────────────────
	void RefreshDatabases();
	void RefreshEntries();
	void ApplySearchFilter();

	// ── State ────────────────────────────────────────────────────────────
	TArray<TSharedPtr<FGameplayTag>> DatabaseTags;
	TSharedPtr<SListView<TSharedPtr<FGameplayTag>>> DatabaseListView;

	TArray<TSharedPtr<FPGXRegistryEntry>> AllEntries;
	TArray<TSharedPtr<FPGXRegistryEntry>> FilteredEntries;
	TSharedPtr<SListView<TSharedPtr<FPGXRegistryEntry>>> EntryListView;

	FGameplayTag SelectedDatabase;
	FString SearchText;

	TSharedPtr<STextBlock> StatusText;
	bool bIsPIEActive = false;

	// ── Delegate Handles ─────────────────────────────────────────────────
	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	FDelegateHandle OnAssetRegisteredHandle;
	FDelegateHandle OnAssetUnregisteredHandle;
	FDelegateHandle OnDatabaseCreatedHandle;
	FDelegateHandle OnCacheInvalidatedHandle;
	FDelegateHandle OnTablesIngestedHandle;

	TWeakObjectPtr<UPGXDataRegistrySubsystem> BoundSubsystem;
};
