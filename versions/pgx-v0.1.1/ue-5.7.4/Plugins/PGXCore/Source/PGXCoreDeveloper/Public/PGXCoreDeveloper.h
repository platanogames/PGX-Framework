// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXCoreDeveloper module interface.
 *     Provides debug tools, testing utilities, profiling helpers,
 *     and developer-only features. This module is automatically
 *     stripped from Shipping builds by Unreal Engine.
 *
 * ES: Interfaz del modulo PGXCoreDeveloper.
 *     Proporciona herramientas de depuracion, utilidades de testing,
 *     ayudas de profiling y funcionalidades solo para desarrolladores.
 *     Este modulo es automaticamente eliminado de los builds de
 *     Shipping por Unreal Engine.
 */
class FPGXCoreDeveloperModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
