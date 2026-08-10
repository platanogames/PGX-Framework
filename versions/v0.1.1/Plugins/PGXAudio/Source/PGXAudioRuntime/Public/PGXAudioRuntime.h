// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

struct IConsoleCommand;

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"

/**
 * EN: PGXAudio runtime module.
 *     Dual-backend audio management: tag-based channels, sound resolution,
 *     music manager, dialogue queue, 5-layer mix, pooling, observability.
 *
 * ES: Modulo runtime de PGXAudio.
 *     Gestion de audio dual-backend: canales basados en tags, resolucion de sonido,
 *     music manager, cola de dialogo, mezcla de 5 capas, pooling, observabilidad.
 */
class FPGXAudioRuntimeModule : public IModuleInterface
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
