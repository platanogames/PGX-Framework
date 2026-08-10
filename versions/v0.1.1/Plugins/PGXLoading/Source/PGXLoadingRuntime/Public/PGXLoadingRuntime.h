// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

struct IConsoleCommand;

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// EN: Log category for PGX Loading systems
// ES: Categoria de log para los sistemas de PGX Loading
PGXLOADINGRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogPGXLoading, Log, All);

/**
 * EN: PGXLoadingRuntime module. Loading management with streaming control and async asset loading.
 * ES: Modulo PGXLoadingRuntime. Gestion de carga con control de streaming y carga async de assets.
 */
class FPGXLoadingRuntimeModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

private:
	template <typename TSubsystem>
	void RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags);
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();
	TArray<IConsoleCommand*> RegisteredConsoleCommands;
};
