// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXTrade editor module. Registers the Development Preview read-only Trade Inspector
 *     NomadTab. Self-registers — does not modify other plugins.
 * ES: Modulo editor de PGXTrade. Registra el NomadTab del Inspector Development Preview
 *     read-only de Trade. Auto-registro — no modifica otros plugins.
 */
class FPGXTradeEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
