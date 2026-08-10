// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX GameFlow Inspector tab with the editor's tab system.
 *     Follows the same pattern as FPGXSaveInspectorTabSpawner for consistency.
 * ES: Registra y gestiona el tab PGX GameFlow Inspector con el sistema de tabs del editor.
 *     Sigue el mismo patron que FPGXSaveInspectorTabSpawner para consistencia.
 */
class PGXEDITORTOOLSEDITOR_API FPGXGameFlowInspectorTabSpawner
{
public:
	// EN: Unique ID for the PGX GameFlow Inspector tab / ES: ID unico para el tab PGX GameFlow Inspector
	static const FName TabId;

	// EN: Register the tab spawner with FGlobalTabmanager / ES: Registrar el tab spawner con FGlobalTabmanager
	static void Register();

	// EN: Unregister the tab spawner / ES: Desregistrar el tab spawner
	static void Unregister();

private:
	// EN: Spawn callback for creating the GameFlow Inspector tab / ES: Callback de spawn para crear el tab GameFlow Inspector
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
