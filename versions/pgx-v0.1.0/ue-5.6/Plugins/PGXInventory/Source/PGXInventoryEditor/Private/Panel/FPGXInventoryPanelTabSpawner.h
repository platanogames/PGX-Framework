// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

class FPGXInventoryPanelTabSpawner
{
public:
	static void Register();
	static void Unregister();
	static const FName TabId;

private:
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
