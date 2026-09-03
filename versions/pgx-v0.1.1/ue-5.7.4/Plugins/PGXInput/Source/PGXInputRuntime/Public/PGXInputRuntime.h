// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXInput runtime module interface.
 *     Provides input abstraction over Enhanced Input with context management,
 *     device detection, and input buffering.
 *
 * ES: Interfaz del modulo runtime de PGXInput.
 *     Proporciona abstraccion de input sobre Enhanced Input con gestion de contextos,
 *     deteccion de dispositivos y buffering de input.
 */
class FPGXInputRuntimeModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
