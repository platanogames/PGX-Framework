// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX PSO Auto-Populator tab with the editor's tab system.
 *     Follows the same pattern as FPGXSaveInspectorTabSpawner for consistency.
 * ES: Registra y gestiona el tab PGX PSO Auto-Populator con el sistema de tabs del editor.
 *     Sigue el mismo patron que FPGXSaveInspectorTabSpawner para consistencia.
 */
class PGXEDITORTOOLSEDITOR_API FPGXPSOAutoPopulatorTabSpawner
{
public:
	// EN: Unique ID for the PGX PSO Auto-Populator tab / ES: ID unico para el tab PGX PSO Auto-Populator
	static const FName TabId;

	// EN: Register the tab spawner with FGlobalTabmanager / ES: Registrar el tab spawner con FGlobalTabmanager
	static void Register();

	// EN: Unregister the tab spawner / ES: Desregistrar el tab spawner
	static void Unregister();

private:
	// EN: Spawn callback for creating the PSO Auto-Populator tab / ES: Callback de spawn para crear el tab PSO Auto-Populator
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
