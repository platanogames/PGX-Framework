// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

struct FPGXDemoEntry;
class UToolMenu;

/**
 * EN: Registers a "PGX DEMO" section in the Content Browser context menu.
 *     Creates pre-configured DataAssets with educational example values, organized in 7 numbered categories.
 *
 * ES: Registra una seccion "PGX DEMO" en el menu contextual del Content Browser.
 *     Crea DataAssets pre-configurados con valores ejemplo educativos, organizados en 7 categorias numeradas.
 */
class PGXSIMHARNESSEDITOR_API FPGXSimHarnessCBExtension
{
public:
	static void Register();
	static void Unregister();

	/** EN: Create a demo DA, populate it, and open in editor / ES: Crear un demo DA, poblarlo y abrir en editor */
	static void CreateDemoAsset(const FPGXDemoEntry& Entry);

private:
	/** EN: Build the demo submenu with 7 categories / ES: Construir el submenu demo con 7 categorias */
	static void BuildDemoMenu(UToolMenu* SubMenu);
};
