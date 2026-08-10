// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EN: PGXSpawn editor module. Registers the Development Preview read-only Spawn Inspector
 *     NomadTab. Self-registers — does not modify other plugins.
 * ES: Modulo editor de PGXSpawn. Registra el NomadTab del Inspector Development Preview
 *     read-only de Spawn. Auto-registro — no modifica otros plugins.
 */
class FPGXSpawnEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
