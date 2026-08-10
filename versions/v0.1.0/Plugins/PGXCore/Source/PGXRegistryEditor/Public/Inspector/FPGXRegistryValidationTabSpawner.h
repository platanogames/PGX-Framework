// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX Registry Validation tab with the editor's tab system.
 *     Follows the NomadTab spawner pattern used across all PGX tabs.
 *
 * ES: Registra y gestiona el tab PGX Registry Validation con el sistema de tabs del editor.
 *     Sigue el patron de spawner NomadTab usado en todos los tabs PGX.
 */
class FPGXRegistryValidationTabSpawner
{
public:
	/** EN: Unique ID for the validation tab / ES: ID unico para el tab de validacion */
	static const FName TabId;

	/** EN: Register the tab spawner with FGlobalTabmanager / ES: Registrar el tab spawner con FGlobalTabmanager */
	static void Register();

	/** EN: Unregister the tab spawner / ES: Desregistrar el tab spawner */
	static void Unregister();

private:
	/** EN: Spawn callback / ES: Callback de spawn */
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
