// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: PGXTutorials module — registers Tutorial Hub, console commands, and manages runner.
// ES: Modulo PGXTutorials — registra Tutorial Hub, comandos de consola y gestiona el runner.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct IConsoleCommand;
class FPGXTutorialRunner;

class FPGXTutorialsEditorModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

	/** EN: Get the tutorial runner (used by Hub to launch tutorials) / ES: Obtener el runner */
	FPGXTutorialRunner* GetRunner() const { return TutorialRunner.Get(); }

private:
	/** EN: Register all console commands / ES: Registrar todos los comandos de consola */
	void RegisterConsoleCommands();

	/** EN: Unregister all console commands / ES: Desregistrar todos los comandos de consola */
	void UnregisterConsoleCommands();

	/** EN: Register the Tutorial Hub NomadTab / ES: Registrar el NomadTab del Tutorial Hub */
	void RegisterHubTab();

	TUniquePtr<FPGXTutorialRunner> TutorialRunner;

	/** EN: All registered console commands / ES: Todos los comandos de consola registrados */
	TArray<IConsoleCommand*> ConsoleCommands;
};
