// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers and manages the Config Dashboard tab.
 * ES: Registra y gestiona el tab del Dashboard de Config.
 */
class PGXEDITORTOOLSEDITOR_API FPGXConfigDashboardTabSpawner
{
public:
	static const FName TabId;

	static void Register();
	static void Unregister();

private:
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
