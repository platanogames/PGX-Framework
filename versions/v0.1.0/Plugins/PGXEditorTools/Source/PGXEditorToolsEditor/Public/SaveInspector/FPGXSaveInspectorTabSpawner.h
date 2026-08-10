// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX Save Inspector tab with the editor's tab system.
 *     Follows the same pattern as FPGXLogViewerTabSpawner for consistency.
 * ES: Registra y gestiona el tab PGX Save Inspector con el sistema de tabs del editor.
 *     Sigue el mismo patron que FPGXLogViewerTabSpawner para consistencia.
 */
class PGXEDITORTOOLSEDITOR_API FPGXSaveInspectorTabSpawner
{
public:
	// EN: Unique ID for the PGX Save Inspector tab / ES: ID unico para el tab PGX Save Inspector
	static const FName TabId;

	// EN: Register the tab spawner with FGlobalTabmanager / ES: Registrar el tab spawner con FGlobalTabmanager
	static void Register();

	// EN: Unregister the tab spawner / ES: Desregistrar el tab spawner
	static void Unregister();

private:
	// EN: Spawn callback for creating the Save Inspector tab / ES: Callback de spawn para crear el tab Save Inspector
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
