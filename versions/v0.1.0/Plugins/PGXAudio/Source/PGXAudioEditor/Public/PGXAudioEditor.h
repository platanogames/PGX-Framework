// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXAudio editor module.
 *     Provides Inspector NomadTab, asset factories, and debug tools.
 *
 * ES: Modulo editor de PGXAudio.
 *     Provee Inspector NomadTab, fabricas de assets y herramientas de debug.
 */
class FPGXAudioEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
