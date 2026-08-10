// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX Message Inspector NomadTab with the editor's
 *     tab system. Mirrors the established FPGXGameFlowInspectorTabSpawner wiring.
 *
 *     Lifecycle: Register() called from PGXCoreEditor::StartupModule(),
 *     Unregister() called from ShutdownModule(). Symmetric.
 *
 * ES: Registra y gestiona el NomadTab PGX Message Inspector con el sistema de
 *     tabs del editor. Replica el wiring establecido de
 *     FPGXGameFlowInspectorTabSpawner.
 *
 *     Ciclo de vida: Register() llamado desde PGXCoreEditor::StartupModule(),
 *     Unregister() llamado desde ShutdownModule(). Simetrico.
 */
class PGXCOREEDITOR_API FPGXMessageInspectorTabSpawner
{
public:
	// EN: Unique tab id ("PGXMessageInspector") / ES: Id unico del tab ("PGXMessageInspector")
	static const FName TabId;

	// EN: Register tab spawner with FGlobalTabmanager / ES: Registrar tab spawner con FGlobalTabmanager
	static void Register();

	// EN: Unregister tab spawner / ES: Desregistrar tab spawner
	static void Unregister();

private:
	// EN: Spawn callback for the tab / ES: Callback de spawn para el tab
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
