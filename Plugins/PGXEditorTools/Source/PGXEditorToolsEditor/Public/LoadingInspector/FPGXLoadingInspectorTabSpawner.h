// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX Loading Inspector tab with the editor's tab system.
 *     Follows the same pattern as FPGXLevelFlowInspectorTabSpawner for consistency.
 * ES: Registra y gestiona el tab PGX Loading Inspector con el sistema de tabs del editor.
 *     Sigue el mismo patron que FPGXLevelFlowInspectorTabSpawner para consistencia.
 */
class PGXEDITORTOOLSEDITOR_API FPGXLoadingInspectorTabSpawner
{
public:
	// EN: Unique ID for the PGX Loading Inspector tab / ES: ID unico para el tab PGX Loading Inspector
	static const FName TabId;

	// EN: Register the tab spawner with FGlobalTabmanager / ES: Registrar el tab spawner con FGlobalTabmanager
	static void Register();

	// EN: Unregister the tab spawner / ES: Desregistrar el tab spawner
	static void Unregister();

private:
	// EN: Spawn callback for creating the Loading Inspector tab / ES: Callback de spawn para crear el tab Loading Inspector
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
