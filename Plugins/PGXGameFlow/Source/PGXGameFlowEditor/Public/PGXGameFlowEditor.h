// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXGameFlow editor module. Registers the read-only GameFlow
 *     Inspector NomadTab. Self-registers — does not modify other plugins.
 * ES: Modulo editor de PGXGameFlow. Registra el NomadTab del Inspector
 *     read-only de GameFlow. Auto-registro — no modifica otros plugins.
 */
class FPGXGameFlowEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
