// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Core/PGXScaffoldTypes.h"

/**
 * EN: Builds tree structure from template items and resolves user selections.
 *     Handles parent/child dependency logic for checkbox toggling.
 *
 * ES: Construye estructura de arbol desde items de template y resuelve selecciones del usuario.
 *     Maneja logica de dependencia padre/hijo para toggling de checkboxes.
 */
class PGXSCAFFOLDEDITOR_API FPGXHierarchyResolver
{
public:
	/** EN: Build tree from template items / ES: Construir arbol desde items del template */
	TArray<TSharedPtr<FPGXScaffoldTreeNode>> BuildTree(const FPGXScaffoldTemplate& Template);

	/** EN: Get selected (checked) items with dependencies resolved / ES: Obtener items seleccionados con dependencias resueltas */
	TArray<FPGXScaffoldTemplateItem> GetSelectedItems(const TArray<TSharedPtr<FPGXScaffoldTreeNode>>& RootNodes);

	/** EN: Toggle a node and propagate to children/parents / ES: Alternar un nodo y propagar a hijos/padres */
	static void ToggleNode(TSharedPtr<FPGXScaffoldTreeNode> Node, bool bNewChecked);

private:
	/** EN: Recursively collect checked items / ES: Recolectar items marcados recursivamente */
	void CollectChecked(const TSharedPtr<FPGXScaffoldTreeNode>& Node, TArray<FPGXScaffoldTemplateItem>& OutItems);
};
