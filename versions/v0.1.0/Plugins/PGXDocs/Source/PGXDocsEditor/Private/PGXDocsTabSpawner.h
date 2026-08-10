// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX Documentation Viewer tab with the editor's tab system.
 *     Follows the NomadTab pattern used by FPGXHubTabSpawner and other PGX tabs.
 * ES: Registra y gestiona el tab PGX Documentation Viewer con el sistema de tabs del editor.
 *     Sigue el patron NomadTab usado por FPGXHubTabSpawner y otros tabs PGX.
 */
class FPGXDocsTabSpawner
{
public:
	// EN: Unique ID for the PGX Documentation Viewer tab / ES: ID unico para el tab PGX Documentation Viewer
	static const FName TabId;

	// EN: Register the tab spawner with FGlobalTabmanager / ES: Registrar el tab spawner con FGlobalTabmanager
	static void Register();

	// EN: Unregister the tab spawner / ES: Desregistrar el tab spawner
	static void Unregister();

private:
	// EN: Spawn callback for creating the Documentation Viewer tab / ES: Callback de spawn para crear el tab Documentation Viewer
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
