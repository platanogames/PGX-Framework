// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXVehicles editor module. Registers the read-only Vehicles Inspector NomadTab. Self-registers — does not modify other plugins.
 * ES: Modulo editor de PGXVehicles. Registra el NomadTab del Inspector de solo lectura de Vehicles. Auto-registro — no modifica otros plugins.
 */
class FPGXVehiclesEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
