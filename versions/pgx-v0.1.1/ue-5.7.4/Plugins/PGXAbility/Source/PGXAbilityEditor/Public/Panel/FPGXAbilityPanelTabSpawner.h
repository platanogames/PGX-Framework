// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX Ability Panel NomadTab with the editor's tab system.
 *     Pattern mirrors FPGXMessageInspectorTabSpawner per shared editor tab pattern.
 *
 *     Lifecycle: Register() called from PGXAbilityEditor::StartupModule(),
 *     Unregister() called from ShutdownModule(). Symmetric.
 *
 * ES: Registra y gestiona el NomadTab del PGX Ability Panel con el sistema de tabs del
 *     editor. Patron replica FPGXMessageInspectorTabSpawner per shared editor tab pattern.
 *
 *     Ciclo de vida: Register() llamado desde PGXAbilityEditor::StartupModule(),
 *     Unregister() desde ShutdownModule(). Simetrico.
 */
class PGXABILITYEDITOR_API FPGXAbilityPanelTabSpawner
{
public:
	// EN: Unique tab id ("PGXAbilityPanel") / ES: Id unico del tab ("PGXAbilityPanel")
	static const FName TabId;

	// EN: Register tab spawner with FGlobalTabmanager / ES: Registrar tab spawner
	static void Register();

	// EN: Unregister tab spawner / ES: Desregistrar tab spawner
	static void Unregister();

private:
	// EN: Spawn callback / ES: Callback de spawn
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
