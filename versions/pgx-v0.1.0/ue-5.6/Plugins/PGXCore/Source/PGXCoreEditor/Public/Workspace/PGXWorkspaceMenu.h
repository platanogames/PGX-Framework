// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class FWorkspaceItem;

/**
 * EN: Registers PGX Framework workspace menu groups in the Window menu.
 *     Creates a "PGX Framework" root group with sub-groups for Inspectors,
 *     Dashboards, and Developer Tools.
 *
 * ES: Registra los grupos del menu workspace de PGX Framework en el menu Window.
 *     Crea un grupo raiz "PGX Framework" con sub-grupos para Inspectors,
 *     Dashboards, y Developer Tools.
 */
struct PGXCOREEDITOR_API FPGXWorkspaceMenu
{
	/** EN: Initialize the PGX workspace menu groups. Call once during StartupModule. */
	/** ES: Inicializar los grupos del menu workspace PGX. Llamar una vez en StartupModule. */
	static void Initialize();

	/** EN: Root "PGX Framework" group / ES: Grupo raiz "PGX Framework" */
	static TSharedRef<FWorkspaceItem> GetRoot();

	/** EN: "System Inspectors" sub-group / ES: Sub-grupo "System Inspectors" */
	static TSharedRef<FWorkspaceItem> GetInspectorsGroup();

	/** EN: "Dashboards" sub-group / ES: Sub-grupo "Dashboards" */
	static TSharedRef<FWorkspaceItem> GetDashboardsGroup();

	/** EN: "Developer Tools" sub-group / ES: Sub-grupo "Developer Tools" */
	static TSharedRef<FWorkspaceItem> GetToolsGroup();

	/** EN: "System Panels" sub-group / ES: Sub-grupo "System Panels"
	 *      Feature-specific editor panels (Ability, Animation, Camera, Cinematic,
	 *      VFX, Inventory, Materials, Interaction, Multiplayer, Online).
	 *      NOT inspectors (runtime debugging) — these are content-authoring panels. */
	static TSharedRef<FWorkspaceItem> GetSystemPanelsGroup();
};
