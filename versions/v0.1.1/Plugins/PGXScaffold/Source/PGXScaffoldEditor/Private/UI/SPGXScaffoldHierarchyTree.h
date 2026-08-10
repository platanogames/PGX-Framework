// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Core/PGXScaffoldTypes.h"

/**
 * EN: Tree view widget for scaffold template items with checkboxes.
 *     Supports parent-child toggling propagation.
 *
 * ES: Widget de vista de arbol para items de template de scaffold con checkboxes.
 *     Soporta propagacion de toggling padre-hijo.
 */
class SPGXScaffoldHierarchyTree : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXScaffoldHierarchyTree) {}
		SLATE_ARGUMENT(TArray<TSharedPtr<FPGXScaffoldTreeNode>>, RootNodes)
		SLATE_EVENT(FSimpleDelegate, OnNodeToggled)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FPGXScaffoldTreeNode>> RootNodes;
	FSimpleDelegate OnNodeToggled;

	/** EN: Build tree rows recursively / ES: Construir filas del arbol recursivamente */
	void BuildTreeRows(TSharedRef<SVerticalBox> Container, const TArray<TSharedPtr<FPGXScaffoldTreeNode>>& Nodes, int32 IndentLevel);
};
