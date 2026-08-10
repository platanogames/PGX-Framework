// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXInput editor module. Registers the read-only Input Inspector NomadTab.
 * ES: Modulo editor de PGXInput. Registra el NomadTab de Inspector Input de solo lectura.
 */
class FPGXInputEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
