// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Core/PGXHierarchyResolver.h"
#include "PGXScaffoldEditor.h"
#include "Logging/PGXLogMacros.h"

TArray<TSharedPtr<FPGXScaffoldTreeNode>> FPGXHierarchyResolver::BuildTree(const FPGXScaffoldTemplate& Template)
{
	// EN: Map ItemId -> Node for fast parent lookup
	// ES: Mapa ItemId -> Node para busqueda rapida de padres
	TMap<FName, TSharedPtr<FPGXScaffoldTreeNode>> NodeMap;
	TArray<TSharedPtr<FPGXScaffoldTreeNode>> AllNodes;

	// EN: Create all nodes first
	// ES: Crear todos los nodos primero
	for (const auto& Item : Template.Items)
	{
		TSharedPtr<FPGXScaffoldTreeNode> Node = MakeShared<FPGXScaffoldTreeNode>();
		Node->Item = Item;
		Node->bChecked = true;
		NodeMap.Add(Item.ItemId, Node);
		AllNodes.Add(Node);
	}

	// EN: Build parent-child relationships
	// ES: Construir relaciones padre-hijo
	TArray<TSharedPtr<FPGXScaffoldTreeNode>> RootNodes;

	for (const auto& Node : AllNodes)
	{
		if (Node->Item.ParentItemId.IsNone())
		{
			RootNodes.Add(Node);
		}
		else
		{
			TSharedPtr<FPGXScaffoldTreeNode>* ParentPtr = NodeMap.Find(Node->Item.ParentItemId);
			if (ParentPtr && ParentPtr->IsValid())
			{
				(*ParentPtr)->Children.Add(Node);
				Node->Parent = *ParentPtr;
			}
			else
			{
				// EN: Orphaned node — make it a root
				// ES: Nodo huerfano — hacerlo raiz
				PGX_LOG_WARNING(LogPGXScaffold, TEXT("FPGXHierarchyResolver: Item '%s' has invalid parent '%s', making root"),
					*Node->Item.ItemId.ToString(), *Node->Item.ParentItemId.ToString());
				RootNodes.Add(Node);
			}
		}
	}

	// EN: Sort children by ExecutionOrder
	// ES: Ordenar hijos por ExecutionOrder
	for (const auto& Node : AllNodes)
	{
		Node->Children.Sort([](const TSharedPtr<FPGXScaffoldTreeNode>& A, const TSharedPtr<FPGXScaffoldTreeNode>& B)
		{
			return A->Item.ExecutionOrder < B->Item.ExecutionOrder;
		});
	}

	RootNodes.Sort([](const TSharedPtr<FPGXScaffoldTreeNode>& A, const TSharedPtr<FPGXScaffoldTreeNode>& B)
	{
		return A->Item.ExecutionOrder < B->Item.ExecutionOrder;
	});

	return RootNodes;
}

TArray<FPGXScaffoldTemplateItem> FPGXHierarchyResolver::GetSelectedItems(const TArray<TSharedPtr<FPGXScaffoldTreeNode>>& RootNodes)
{
	TArray<FPGXScaffoldTemplateItem> Selected;
	for (const auto& Root : RootNodes)
	{
		CollectChecked(Root, Selected);
	}
	return Selected;
}

void FPGXHierarchyResolver::ToggleNode(TSharedPtr<FPGXScaffoldTreeNode> Node, bool bNewChecked)
{
	if (!Node.IsValid()) { return; }

	Node->bChecked = bNewChecked;

	// EN: Propagate DOWN — unchecking parent unchecks all children
	// ES: Propagar ABAJO — desmarcar padre desmarca todos los hijos
	if (!bNewChecked)
	{
		for (auto& Child : Node->Children)
		{
			ToggleNode(Child, false);
		}
	}

	// EN: Propagate UP — checking child checks all ancestors
	// ES: Propagar ARRIBA — marcar hijo marca todos los ancestros
	if (bNewChecked)
	{
		TSharedPtr<FPGXScaffoldTreeNode> ParentNode = Node->Parent.Pin();
		while (ParentNode.IsValid())
		{
			ParentNode->bChecked = true;
			ParentNode = ParentNode->Parent.Pin();
		}
	}
}

void FPGXHierarchyResolver::CollectChecked(const TSharedPtr<FPGXScaffoldTreeNode>& Node, TArray<FPGXScaffoldTemplateItem>& OutItems)
{
	if (!Node.IsValid()) { return; }

	if (Node->bChecked)
	{
		OutItems.Add(Node->Item);
	}

	for (const auto& Child : Node->Children)
	{
		CollectChecked(Child, OutItems);
	}
}
