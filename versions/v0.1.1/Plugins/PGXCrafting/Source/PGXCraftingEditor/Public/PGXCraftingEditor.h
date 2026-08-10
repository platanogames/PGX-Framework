// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXCrafting editor module. Registers the Development Preview read-only Crafting
 *     Inspector NomadTab. Self-registers — does not modify other plugins.
 * ES: Modulo editor de PGXCrafting. Registra el NomadTab del Inspector
 *     Development Preview read-only de Crafting. Auto-registro — no modifica otros plugins.
 */
class FPGXCraftingEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
