// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "PGXVersionControlTypes.h"

class UPGXSourceControlSubsystem;
class STextBlock;
class SVerticalBox;
class SWrapBox;
template<typename ItemType> class SListView;  // EN: Template forward-declare / ES: Forward-declare de template

/**
 * EN: Inspector panel for PGX Version Control overlay.
 *     4 sections: Provider Status, Changelists, PGX Workflow, Quick Actions.
 *     Refreshes periodically via Tick (2s interval).
 *     Binds OnChangelistsChanged in Construct, unbinds in destructor (S1).
 *
 * ES: Panel inspector para el overlay PGX Version Control.
 *     4 secciones: Estado del Provider, Changelists, PGX Workflow, Acciones Rapidas.
 *     Refresca periodicamente via Tick (intervalo 2s).
 *     Binds OnChangelistsChanged en Construct, unbinds en destructor (S1).
 */
class PGXVERSIONCONTROLEDITOR_API SPGXVersionControlInspectorTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXVersionControlInspectorTab) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXVersionControlInspectorTab() override;

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// EN: Section builders / ES: Constructores de seccion
	// EN: BuildToolbar() removed — integrated into SPGXPanelHeader
	TSharedRef<SWidget> BuildProviderStatusSection();
	TSharedRef<SWidget> BuildChangelistSection();
	TSharedRef<SWidget> BuildWorkflowSection();
	TSharedRef<SWidget> BuildQuickActionsSection();

	// EN: Actions / ES: Acciones
	void RefreshData();
	void OnChangelistsChanged();
	void OnValidateClicked();
	void OnCreateChangelistClicked();
	void OnDeleteChangelistClicked();
	void OnRenameChangelistClicked();

	// EN: Changelist list view / ES: Vista de lista de changelists
	TSharedRef<ITableRow> GenerateChangelistRow(TSharedPtr<FPGXChangelist> InItem, const TSharedRef<STableViewBase>& InOwnerTable);

	UPGXSourceControlSubsystem* GetSubsystem() const;

	// ─── Cached widgets ───
	TSharedPtr<STextBlock> ProviderNameText;
	TSharedPtr<STextBlock> BranchText;
	TSharedPtr<STextBlock> ConnectionStatusText;
	TSharedPtr<SWrapBox> SystemTagsBox;
	TSharedPtr<STextBlock> SuggestedPrefixText;
	TSharedPtr<STextBlock> FooterText;
	TSharedPtr<STextBlock> TemplateText;
	TSharedPtr<SVerticalBox> ValidationResultsBox;
	TSharedPtr<SListView<TSharedPtr<FPGXChangelist>>> ChangelistListView;

	// ─── Data ───
	TArray<TSharedPtr<FPGXChangelist>> ChangelistItems;
	TSharedPtr<FPGXChangelist> SelectedChangelist;
	TArray<FPGXValidationIssue> CachedValidationIssues;

	// ─── Tick refresh ───
	float RefreshAccumulator = 0.0f;
	static constexpr float RefreshInterval = 2.0f;
};
