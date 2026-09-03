// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXUI runtime module.
 *     Screen management, notifications, widget pooling, and loading screens.
 *
 * ES: Modulo runtime de PGXUI.
 *     Gestion de pantallas, notificaciones, pooling de widgets y pantallas de carga.
 */
class FPGXUIRuntimeModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
