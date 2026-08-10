// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

struct IConsoleCommand;

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"

/**
 * EN: PGX PSO runtime module. Provides data-driven PSO warm-up,
 *     batched material precaching, and recording for Active Audit workflows.
 * ES: Modulo runtime de PGX PSO. Proporciona warm-up de PSO data-driven,
 *     precaching de materiales por lotes y grabacion para flujos de Active Audit.
 */
class FPGXPSORuntimeModule : public IModuleInterface
{
public:
	/** EN: Called when the module is loaded into memory / ES: Se llama cuando el modulo se carga en memoria */
	void StartupModule() override;

	/** EN: Called when the module is unloaded from memory / ES: Se llama cuando el modulo se descarga de memoria */
	void ShutdownModule() override;

private:
	template <typename TSubsystem>
	void RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags);
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();
	TArray<IConsoleCommand*> RegisteredConsoleCommands;
};
