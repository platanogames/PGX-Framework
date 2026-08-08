// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Widgets/SDocTreeView.h"
#include "PGXDocSystem.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "PGXDocs"

void SDocTreeView::Construct(const FArguments& InArgs)
{
	OnDocSelected = InArgs._OnDocSelected;

	// EN: Get tree data from the doc system / ES: Obtener datos del arbol del sistema de docs
	RefreshTree();

	ChildSlot
	[
		SNew(SBox)
		.Padding(FMargin(2.0f))
		[
			SAssignNew(TreeWidget, STreeView<TSharedPtr<FDocNode>>)
			.TreeItemsSource(&RootItems)
			.OnGenerateRow(this, &SDocTreeView::OnGenerateRow)
			.OnGetChildren(this, &SDocTreeView::OnGetChildren)
			.OnSelectionChanged(this, &SDocTreeView::OnSelectionChanged)
			.OnExpansionChanged(this, &SDocTreeView::OnExpansionChanged)
			.SelectionMode(ESelectionMode::Single)
		]
	];

	// EN: Expand root nodes by default / ES: Expandir nodos raiz por defecto
	for (const TSharedPtr<FDocNode>& RootNode : RootItems)
	{
		if (TreeWidget.IsValid())
		{
			TreeWidget->SetItemExpansion(RootNode, true);
		}
	}
}

void SDocTreeView::RefreshTree()
{
	const FDocSystem& DocSystem = FDocSystem::Get();
	RootItems = DocSystem.GetRootTree();

	if (TreeWidget.IsValid())
	{
		TreeWidget->RequestTreeRefresh();
	}
}

void SDocTreeView::SelectDocument(const FString& DocId)
{
	if (!TreeWidget.IsValid())
	{
		return;
	}

	// EN: Find the node and expand all ancestor folders so the item is visible
	// ES: Encontrar el nodo y expandir todas las carpetas ancestro para que el item sea visible
	TArray<TSharedPtr<FDocNode>> AncestorPath;
	TSharedPtr<FDocNode> TargetNode = FindNodeWithPath(DocId, RootItems, AncestorPath);

	if (TargetNode.IsValid())
	{
		for (const TSharedPtr<FDocNode>& Ancestor : AncestorPath)
		{
			TreeWidget->SetItemExpansion(Ancestor, true);
		}
		TreeWidget->SetSelection(TargetNode);
		TreeWidget->RequestScrollIntoView(TargetNode);
	}
}

TSharedRef<ITableRow> SDocTreeView::OnGenerateRow(TSharedPtr<FDocNode> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const bool bIsFolder = Item->DocId.IsEmpty();

	// EN: Choose icon based on node type / ES: Elegir icono basado en tipo de nodo
	const FSlateBrush* IconBrush = bIsFolder
		? FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderClosed")
		: FAppStyle::GetBrush("Icons.Documentation");

	// EN: Use the active editor foreground color for every documentation source.
	// ES: Usar el color de primer plano del editor para toda fuente documental.
	const FSlateColor TextColor = FSlateColor::UseForeground();

	return SNew(STableRow<TSharedPtr<FDocNode>>, OwnerTable)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2.0f, 0.0f)
			[
				SNew(SImage)
				.Image(IconBrush)
				.DesiredSizeOverride(FVector2D(16.0f, 16.0f))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(4.0f, 2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->Title))
				.ColorAndOpacity(TextColor)
				.Font(bIsFolder
					? FCoreStyle::GetDefaultFontStyle("Bold", 10)
					: FCoreStyle::GetDefaultFontStyle("Regular", 10))
			]
		];
}

void SDocTreeView::OnGetChildren(TSharedPtr<FDocNode> Item, TArray<TSharedPtr<FDocNode>>& OutChildren)
{
	if (Item.IsValid())
	{
		OutChildren = Item->Children;
	}
}

void SDocTreeView::OnSelectionChanged(TSharedPtr<FDocNode> Item, ESelectInfo::Type /*SelectInfo*/)
{
	if (Item.IsValid() && !Item->DocId.IsEmpty())
	{
		OnDocSelected.ExecuteIfBound(Item->DocId);
	}
}

void SDocTreeView::OnExpansionChanged(TSharedPtr<FDocNode> Item, bool bIsExpanded)
{
	if (Item.IsValid())
	{
		Item->bIsExpanded = bIsExpanded;
	}
}

TSharedPtr<FDocNode> SDocTreeView::FindNode(const FString& DocId, const TArray<TSharedPtr<FDocNode>>& Nodes) const
{
	for (const TSharedPtr<FDocNode>& Node : Nodes)
	{
		if (Node->DocId == DocId)
		{
			return Node;
		}

		TSharedPtr<FDocNode> Found = FindNode(DocId, Node->Children);
		if (Found.IsValid())
		{
			return Found;
		}
	}

	return nullptr;
}

TSharedPtr<FDocNode> SDocTreeView::FindNodeWithPath(const FString& DocId, const TArray<TSharedPtr<FDocNode>>& Nodes, TArray<TSharedPtr<FDocNode>>& OutPath) const
{
	for (const TSharedPtr<FDocNode>& Node : Nodes)
	{
		if (Node->DocId == DocId)
		{
			return Node;
		}

		if (Node->Children.Num() > 0)
		{
			OutPath.Add(Node);
			TSharedPtr<FDocNode> Found = FindNodeWithPath(DocId, Node->Children, OutPath);
			if (Found.IsValid())
			{
				return Found;
			}
			OutPath.Pop();
		}
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
