// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXInteraction runtime module.
 *     Component-based interaction system for world objects.
 *     Provides detection, contextual prompts, and conditional interactions.
 *
 * ES: Modulo runtime de PGXInteraction.
 *     Sistema de interaccion basado en componentes para objetos del mundo.
 *     Proporciona deteccion, prompts contextuales e interacciones condicionales.
 */
class FPGXInteractionRuntimeModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
