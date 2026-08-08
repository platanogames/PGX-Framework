// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class FToolBarBuilder;
class FExtender;

/**
 * EN: Builds and registers the PGX toolbar dropdown in the Level Editor.
 *     Provides the main entry point to all PGX editor features.
 * ES: Construye y registra el dropdown PGX en la toolbar del Level Editor.
 *     Proporciona el punto de entrada principal a todas las features de editor PGX.
 */
class PGXCOREEDITOR_API FPGXToolbarBuilder
{
public:
	// EN: Register the PGX toolbar with UToolMenus / ES: Registrar la toolbar PGX con UToolMenus
	static void RegisterToolbar();

	// EN: Register quick-access pinnable buttons in the toolbar / ES: Registrar botones pinneables de acceso rapido en la toolbar
	static void RegisterQuickAccessButtons();

	// EN: Unregister the PGX toolbar / ES: Desregistrar la toolbar PGX
	static void UnregisterToolbar();

private:
	// EN: Build the PGX dropdown menu content / ES: Construir el contenido del menu dropdown PGX
	static void BuildMenu(class UToolMenu* Menu);

	// EN: Build the Create PGX Asset submenu / ES: Construir el submenu de crear asset PGX
	static void BuildCreateAssetSubmenu(class UToolMenu* Menu);

	// EN: Build the Tools submenu / ES: Construir el submenu de herramientas
	static void BuildToolsSubmenu(class UToolMenu* Menu);

	// EN: Build the Documentation submenu / ES: Construir el submenu de documentacion
	static void BuildDocsSubmenu(class UToolMenu* Menu);

	// EN: Execute safe restart with dirty check / ES: Ejecutar reinicio seguro con verificacion de cambios
	static void ExecuteSafeRestart();

	// EN: Combo dropdown builders for Quick Access groups / ES: Builders de combo dropdown para grupos Quick Access
	static void BuildInspectorsDropdown(class UToolMenu* Menu);
	static void BuildPipelineDropdown(class UToolMenu* Menu);
	static void BuildDevToolsDropdown(class UToolMenu* Menu);
};
