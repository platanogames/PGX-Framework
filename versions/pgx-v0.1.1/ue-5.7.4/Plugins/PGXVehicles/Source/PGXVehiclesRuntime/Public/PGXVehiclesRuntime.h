// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

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