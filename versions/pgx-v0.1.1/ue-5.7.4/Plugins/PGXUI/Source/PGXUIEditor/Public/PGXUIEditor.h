// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXUI editor module. Registers the read-only UI Inspector NomadTab.
 * ES: Modulo editor de PGXUI. Registra el NomadTab de Inspector UI de solo lectura.
 */
class FPGXUIEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
