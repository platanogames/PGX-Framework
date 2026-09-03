// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * EN: Module bootstrap for PGXColonyRuntime. The L2 colony-state framework lives here:
 *     survivor / settlement / role / task / need primitives + subsystem registry. Detailed
 *     surface owned by `UPGXColonySubsystem` and the data-driven `UPGXColonyConfig` /
 *     `UPGXColonySettings` resolution chain. See current product scope for the canonical scope
 *     boundary (PGXColony decides intent; AI / Inventory / Trade / Crafting / Save execute).
 * ES: Bootstrap del modulo PGXColonyRuntime. El framework L2 de estado de colonia vive aqui.
 */
class FPGXColonyRuntimeModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
