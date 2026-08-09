// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXInteraction editor module. Registers the read-only
 *     Interaction Panel NomadTab. Self-registers — does not modify other plugins.
 *     PGXCoreEditor owns the central style set; this module consumes it.
 * ES: Modulo editor de PGXInteraction. Registra el NomadTab de solo lectura
 *     del Panel Interaction. Auto-registro — no modifica otros plugins.
 *     PGXCoreEditor es propietario del style set central; este modulo lo consume.
 */
class FPGXInteractionEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
