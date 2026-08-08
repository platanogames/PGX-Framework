// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX Hub tab with the editor's tab system.
 *     Handles tab spawning, singleton enforcement, and layout persistence.
 * ES: Registra y gestiona el tab PGX Hub con el sistema de tabs del editor.
 *     Maneja spawning de tabs, singleton enforcement y persistencia de layout.
 */
class PGXEDITORTOOLSEDITOR_API FPGXHubTabSpawner
{
public:
	// EN: Unique ID for the PGX Hub tab / ES: ID unico para el tab PGX Hub
	static const FName TabId;

	// EN: Register the tab spawner with FGlobalTabmanager / ES: Registrar el tab spawner con FGlobalTabmanager
	static void Register();

	// EN: Unregister the tab spawner / ES: Desregistrar el tab spawner
	static void Unregister();

private:
	// EN: Spawn callback for creating the Hub tab / ES: Callback de spawn para crear el tab Hub
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
