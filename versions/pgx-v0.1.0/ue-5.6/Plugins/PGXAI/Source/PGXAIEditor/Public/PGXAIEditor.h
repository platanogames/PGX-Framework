// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXAI editor module. Registers the Development Preview read-only AI Inspector NomadTab.
 * ES: Modulo editor de PGXAI. Registra el NomadTab de Inspector AI read-only de Development Preview.
 */
class FPGXAIEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
