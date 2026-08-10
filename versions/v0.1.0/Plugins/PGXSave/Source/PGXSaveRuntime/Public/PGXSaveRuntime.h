// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXSave runtime module interface.
 *     Provides a complete save/load system with slot management,
 *     auto-save, versioning, and serialization utilities.
 *
 * ES: Interfaz del modulo runtime de PGXSave.
 *     Proporciona un sistema completo de guardado/carga con gestion de slots,
 *     auto-save, versionado y utilidades de serializacion.
 */
class FPGXSaveRuntimeModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
