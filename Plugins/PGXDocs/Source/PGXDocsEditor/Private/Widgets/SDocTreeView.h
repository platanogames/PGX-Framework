// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "PGXDocTypes.h"

DECLARE_DELEGATE_OneParam(FOnDocSelected, const FString& /* DocId */);

/**
 * EN: Left sidebar tree view showing the documentation hierarchy.
 *     Supports folders and document leaves with icons.
 *     Lazy children loading for performance.
 *
 * ES: Vista de arbol en sidebar izquierdo mostrando la jerarquia de documentacion.
 *     Soporta carpetas y hojas de documentos con iconos.
 *     Carga lazy de hijos para rendimiento.
 */
class SDocTreeView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDocTreeView) {}
		SLATE_EVENT(FOnDocSelected, OnDocSelected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** EN: Refresh the tree from the registry / ES: Refrescar el arbol desde el registry */
	void RefreshTree();

	/** EN: Programmatically select a document / ES: Seleccionar programaticamente un documento */
	void SelectDocument(const FString& DocId);

private:
	// EN: STreeView callbacks / ES: Callbacks de STreeView
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FDocNode> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TSharedPtr<FDocNode> Item, TArray<TSharedPtr<FDocNode>>& OutChildren);
	void OnSelectionChanged(TSharedPtr<FDocNode> Item, ESelectInfo::Type SelectInfo);
	void OnExpansionChanged(TSharedPtr<FDocNode> Item, bool bIsExpanded);

	// EN: Find a node by DocId recursively / ES: Encontrar un nodo por DocId recursivamente
	TSharedPtr<FDocNode> FindNode(const FString& DocId, const TArray<TSharedPtr<FDocNode>>& Nodes) const;

	// EN: Find a node by DocId and collect ancestor path for expansion
	// ES: Encontrar un nodo por DocId y recopilar ruta de ancestros para expansion
	TSharedPtr<FDocNode> FindNodeWithPath(const FString& DocId, const TArray<TSharedPtr<FDocNode>>& Nodes, TArray<TSharedPtr<FDocNode>>& OutPath) const;

	FOnDocSelected OnDocSelected;
	TArray<TSharedPtr<FDocNode>> RootItems;
	TSharedPtr<STreeView<TSharedPtr<FDocNode>>> TreeWidget;
};
