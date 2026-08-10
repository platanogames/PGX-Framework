// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXCamera runtime module.
 *     Camera mode management, transitions, and modifiers.
 *
 * ES: Modulo runtime de PGXCamera.
 *     Gestion de modos de camara, transiciones y modifiers.
 */
class FPGXCameraRuntimeModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
