// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXInventory runtime module.
 *     Data-driven item and inventory system with components and containers.
 *     Provides item definitions, instanced items, inventory management,
 *     and container actors.
 *
 * ES: Modulo runtime de PGXInventory.
 *     Sistema de items e inventario data-driven con componentes y contenedores.
 *     Proporciona definiciones de items, items instanciados, gestion de inventario
 *     y actores contenedores.
 */
class FPGXInventoryRuntimeModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
