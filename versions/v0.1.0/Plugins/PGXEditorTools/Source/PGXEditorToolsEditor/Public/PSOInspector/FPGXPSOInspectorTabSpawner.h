// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Framework/Docking/TabManager.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: NomadTab spawner for the PGX PSO Inspector Panel.
 *     Registers/unregisters the tab with FGlobalTabmanager.
 *     Accessible from: PGX Toolbar > Tools > PSO Inspector
 * ES: Spawner de NomadTab para el Panel Inspector PSO de PGX.
 *     Registra/desregistra el tab con FGlobalTabmanager.
 *     Accesible desde: PGX Toolbar > Tools > PSO Inspector
 */
class PGXEDITORTOOLSEDITOR_API FPGXPSOInspectorTabSpawner
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
