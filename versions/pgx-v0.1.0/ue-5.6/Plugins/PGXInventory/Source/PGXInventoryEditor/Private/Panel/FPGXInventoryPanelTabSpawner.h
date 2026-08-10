// Copyright PGX Framework. All Rights Reserved.

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
