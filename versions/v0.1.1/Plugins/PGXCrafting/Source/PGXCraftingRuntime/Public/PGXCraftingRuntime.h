// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXCrafting runtime module. Owns generic crafting rules and baseline craft job orchestration.
 * ES: Modulo runtime de PGXCrafting. Posee reglas genericas de crafting y orquestacion base de trabajos.
 */
class FPGXCraftingRuntimeModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};