// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"

class SDockTab;
class FSpawnTabArgs;

/**
 * EN: Registers the PGX Visual Showcase tab — a fake-data prototype
 *     demonstrating all PGX visual components and tokens.
 * ES: Registra el tab PGX Visual Showcase — un prototipo con datos falsos
 *     demostrando todos los componentes visuales y tokens PGX.
 */
class PGXEDITORTOOLSEDITOR_API FPGXVisualShowcaseTabSpawner
{
public:
	static const FName TabId;
	static void Register();
	static void Unregister();

private:
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
};
