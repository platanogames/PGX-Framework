// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * EN: PGX PSO editor module. Editor tools for PSO recording, inspection,
 *     and warm-up management.
 * ES: Modulo editor de PGX PSO. Herramientas de editor para grabacion,
 *     inspeccion y gestion de warm-up de PSO.
 */
class FPGXPSOEditorModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;
};
