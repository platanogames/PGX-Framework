// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class UToolMenu;

/**
 * EN: Extends the Content Browser "Add New" menu with a "PGX FRAMEWORK" section.
 *     Creates its own section with header (like GET/CREATE) positioned after "New Folder".
 *     Contains a "PGX Framework" submenu that expands into system categories,
 *     each with Blueprints/DataAssets groups and icons on all entries.
 *
 * ES: Extiende el menu "Add New" del Content Browser con una seccion "PGX FRAMEWORK".
 *     Crea su propia seccion con header (como GET/CREATE) posicionada despues de "New Folder".
 *     Contiene un submenu "PGX Framework" que se expande en categorias de sistema,
 *     cada una con grupos Blueprints/DataAssets e iconos en todas las entradas.
 */
class PGXCOREEDITOR_API FPGXContentBrowserExtension
{
public:
	static void Register();
	static void Unregister();

private:
	// EN: Builds the full PGX Framework submenu tree (systems > Blueprints/DataAssets)
	// ES: Construye el arbol de submenu PGX Framework completo (sistemas > Blueprints/DataAssets)
	static void BuildPGXFrameworkMenu(UToolMenu* SubMenu);

	// EN: Get the FSlateIcon for a system category / ES: Obtener FSlateIcon para una categoria de sistema
	static FSlateIcon GetSystemIcon(const FString& System);
};
