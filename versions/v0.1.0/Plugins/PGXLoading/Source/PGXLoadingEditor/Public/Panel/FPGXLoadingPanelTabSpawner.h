// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the PGX Loading Panel NomadTab.
 * ES: Registra y gestiona el NomadTab del PGX Loading Panel.
 */
class PGXLOADINGEDITOR_API FPGXLoadingPanelTabSpawner
{
public:
	static const FName TabId;
	static void Register();
	static void Unregister();

private:
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
