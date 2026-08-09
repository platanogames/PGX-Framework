// Copyright PGX Framework. All Rights Reserved.

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
