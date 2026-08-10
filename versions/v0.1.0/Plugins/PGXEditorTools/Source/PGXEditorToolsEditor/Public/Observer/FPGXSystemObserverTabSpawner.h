// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Framework/Docking/TabManager.h"

/**
 * EN: NomadTab spawner for the PGX System Observer Panel.
 *     Registers/unregisters the tab with FGlobalTabmanager.
 *     Accessible from: PGX Toolbar > Tools > System Observer
 *
 * ES: Spawner de NomadTab para el Panel Observer del Sistema PGX.
 *     Registra/desregistra el tab con FGlobalTabmanager.
 *     Accesible desde: PGX Toolbar > Tools > System Observer
 */
class PGXEDITORTOOLSEDITOR_API FPGXSystemObserverTabSpawner
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
