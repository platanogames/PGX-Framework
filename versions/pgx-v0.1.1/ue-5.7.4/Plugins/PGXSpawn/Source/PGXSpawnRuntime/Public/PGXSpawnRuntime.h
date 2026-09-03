// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct IConsoleCommand;
class UPGXSpawnSubsystem;
class UWorld;

/**
 * EN: PGXSpawn runtime module. Centralized spawn management with pooling support.
 * ES: Modulo runtime de PGXSpawn. Gestion centralizada de spawn con soporte de pooling.
 */
class FPGXSpawnRuntimeModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

private:
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();
	UPGXSpawnSubsystem* ResolveSpawnSubsystem(UWorld* World, const TCHAR* CommandName) const;
	void HandleConsoleList(const TArray<FString>& Args, UWorld* World);
	void HandleConsoleCleanup(const TArray<FString>& Args, UWorld* World);
	void HandleConsoleBudget(const TArray<FString>& Args, UWorld* World);
	void HandleConsoleWaves(const TArray<FString>& Args, UWorld* World);
	void HandleConsoleTriggerPoint(const TArray<FString>& Args, UWorld* World);
	void HandleConsolePoolClear(const TArray<FString>& Args, UWorld* World);

	TArray<IConsoleCommand*> RegisteredConsoleCommands;
};
