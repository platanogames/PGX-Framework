// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

struct IConsoleCommand;

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"

/**
 * EN: PGXGameFlow runtime module interface.
 *     Provides 8-channel FSM (GameplayTag-driven), data-driven transition rules,
 *     two-layer validation (Allowed + Disallowed), history tracking, and BP API.
 *
 * ES: Interfaz del modulo runtime de PGXGameFlow.
 *     Proporciona FSM de 8 canales (GameplayTag-driven), reglas de transicion data-driven,
 *     validacion de dos capas (Allowed + Disallowed), tracking de historial y API BP.
 */
class FPGXGameFlowRuntimeModule : public IModuleInterface
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
