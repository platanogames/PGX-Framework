// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * EN: Editor module for the PGX Camera plugin.
 *     Registers SPGXCameraPanel NomadTab (Config DataAsset inspector).
 * ES: Modulo editor para el plugin PGX Camera (inspector de Config DataAsset).
 */
class FPGXCameraEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
