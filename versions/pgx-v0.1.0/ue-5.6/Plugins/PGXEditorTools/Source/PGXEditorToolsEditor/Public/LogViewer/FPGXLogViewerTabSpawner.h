// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX Log Viewer tab with the editor's tab system.
 *     Follows the same pattern as FPGXHubTabSpawner for consistency.
 * ES: Registra y gestiona el tab PGX Log Viewer con el sistema de tabs del editor.
 *     Sigue el mismo patron que FPGXHubTabSpawner para consistencia.
 */
class PGXEDITORTOOLSEDITOR_API FPGXLogViewerTabSpawner
{
public:
	// EN: Unique ID for the PGX Log Viewer tab / ES: ID unico para el tab PGX Log Viewer
	static const FName TabId;

	// EN: Register the tab spawner with FGlobalTabmanager / ES: Registrar el tab spawner con FGlobalTabmanager
	static void Register();

	// EN: Unregister the tab spawner / ES: Desregistrar el tab spawner
	static void Unregister();

private:
	// EN: Spawn callback for creating the Log Viewer tab / ES: Callback de spawn para crear el tab Log Viewer
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
