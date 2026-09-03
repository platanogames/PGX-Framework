// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * EN: PGXSpawn runtime module. Centralized spawn management with pooling support.
 * ES: Modulo runtime de PGXSpawn. Gestion centralizada de spawn con soporte de pooling.
 */
class FPGXSpawnRuntimeModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
