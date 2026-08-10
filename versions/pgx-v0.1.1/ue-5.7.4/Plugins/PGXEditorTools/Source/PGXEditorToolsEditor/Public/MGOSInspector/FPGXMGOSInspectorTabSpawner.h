// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Framework/Docking/TabManager.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: NomadTab spawner for the PGX MGOS Inspector Panel.
 *     Registers/unregisters the tab with FGlobalTabmanager.
 *     Accessible from: PGX Toolbar > Tools > MGOS Inspector
 * ES: Spawner de NomadTab para el Panel Inspector MGOS de PGX.
 *     Registra/desregistra el tab con FGlobalTabmanager.
 *     Accesible desde: PGX Toolbar > Tools > MGOS Inspector
 */
class PGXEDITORTOOLSEDITOR_API FPGXMGOSInspectorTabSpawner
{
public:
	/** EN: Unique ID for the tab / ES: ID unico para el tab */
	static const FName TabId;

	/** EN: Register the NomadTab spawner / ES: Registrar el spawner del NomadTab */
	static void Register();

	/** EN: Unregister the NomadTab spawner / ES: Desregistrar el spawner del NomadTab */
	static void Unregister();

private:
	/** EN: Spawn callback / ES: Callback de spawn */
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
