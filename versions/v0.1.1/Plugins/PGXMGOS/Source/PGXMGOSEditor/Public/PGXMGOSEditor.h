// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXMGOS editor module. Registers the read-only MGOS Inspector
 *     NomadTab. Self-registers — does not modify other plugins.
 * ES: Modulo editor de PGXMGOS. Registra el NomadTab del Inspector
 *     read-only de MGOS. Auto-registro — no modifica otros plugins.
 */
class FPGXMGOSEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
