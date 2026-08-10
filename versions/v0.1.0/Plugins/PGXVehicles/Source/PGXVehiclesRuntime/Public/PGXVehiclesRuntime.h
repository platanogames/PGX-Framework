// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXVehicles runtime module. Owns generic vehicle metadata and baseline state orchestration.
 * ES: Modulo runtime de PGXVehicles. Posee metadata generica de vehiculos y orquestacion base de estado.
 */
class FPGXVehiclesRuntimeModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};