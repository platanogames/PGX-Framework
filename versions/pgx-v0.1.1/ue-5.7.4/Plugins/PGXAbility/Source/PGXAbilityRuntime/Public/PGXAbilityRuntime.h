// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

struct IConsoleCommand;

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Logging/PGXLogMacros.h"

/** EN: Shared log category for the entire PGXAbilityRuntime module (Subsystem, Component, Facades, Bridge). / ES: Categoria de log compartida para todo el modulo PGXAbilityRuntime. */
PGX_DECLARE_LOG_CATEGORY(PGXABILITYRUNTIME_API, LogPGXAbility);

/**
 * EN: PGX Ability runtime module.
 *     Provides a clean facade layer over Unreal's Gameplay Ability System (GAS).
 *     Encapsulates attributes, abilities, and effects behind the PGX API so the rest
 *     of the framework never depends directly on GAS types.
 *
 * ES: Modulo runtime de PGX Ability.
 *     Proporciona una capa facade limpia sobre el Gameplay Ability System (GAS) de Unreal.
 *     Encapsula atributos, abilities y efectos detras de la API PGX para que el resto
 *     del framework nunca dependa directamente de tipos GAS.
 */
class FPGXAbilityRuntimeModule : public IModuleInterface
{
public:

	/**
	 * EN: Called when the module is loaded into memory.
	 * ES: Se llama cuando el modulo se carga en memoria.
	 */
	void StartupModule() override;

	/**
	 * EN: Called when the module is unloaded from memory.
	 * ES: Se llama cuando el modulo se descarga de memoria.
	 */
	void ShutdownModule() override;

private:
	template <typename TSubsystem>
	void RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags);
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();
	TArray<IConsoleCommand*> RegisteredConsoleCommands;
};
