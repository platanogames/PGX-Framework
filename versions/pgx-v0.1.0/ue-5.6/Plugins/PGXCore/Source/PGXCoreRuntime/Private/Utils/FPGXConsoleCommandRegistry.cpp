// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Utils/FPGXConsoleCommandRegistry.h"

#include "Logging/PGXLogCategories.h"

// EN: RAII wrapper for IConsoleManager. Implementation pattern: IConsoleManager
//     is a global singleton; we just need to track pointers so we can unregister
//     on destruction. The console manager is automatically cleaned up at engine
//     shutdown, so we don't need to handle that path here.
// ES: Wrapper RAII para IConsoleManager. Patrón: IConsoleManager es singleton
//     global; solo necesitamos trackear punteros para desregistrar en
//     destruccion. El console manager se limpia solo al apagar el engine.

IConsoleCommand* FPGXConsoleCommandRegistry::RegisterCommand(
	const TCHAR* Name,
	const TCHAR* Help,
	const FConsoleCommandDelegate& Command,
	uint32 Flags)
{
	IConsoleManager& Manager = IConsoleManager::Get();
	IConsoleCommand* Cmd = Manager.RegisterConsoleCommand(Name, Help, Command, Flags);
	if (Cmd)
	{
		RegisteredCommands.Add(Cmd);
	}
	else
	{
		UE_LOG(LogPGXSettings, Warning, TEXT("[FPGXConsoleCommandRegistry] RegisterCommand failed: %s"), Name);
	}
	return Cmd;
}

IConsoleCommand* FPGXConsoleCommandRegistry::RegisterCommandWithArgs(
	const TCHAR* Name,
	const TCHAR* Help,
	const FConsoleCommandWithArgsDelegate& Command,
	uint32 Flags)
{
	IConsoleManager& Manager = IConsoleManager::Get();
	IConsoleCommand* Cmd = Manager.RegisterConsoleCommand(Name, Help, Command, Flags);
	if (Cmd)
	{
		RegisteredCommands.Add(Cmd);
	}
	else
	{
		UE_LOG(LogPGXSettings, Warning, TEXT("[FPGXConsoleCommandRegistry] RegisterCommandWithArgs failed: %s"), Name);
	}
	return Cmd;
}

IConsoleCommand* FPGXConsoleCommandRegistry::RegisterCommandWithWorldAndArgs(
	const TCHAR* Name,
	const TCHAR* Help,
	const FConsoleCommandWithWorldAndArgsDelegate& Command,
	uint32 Flags)
{
	IConsoleManager& Manager = IConsoleManager::Get();
	IConsoleCommand* Cmd = Manager.RegisterConsoleCommand(Name, Help, Command, Flags);
	if (Cmd)
	{
		RegisteredCommands.Add(Cmd);
	}
	else
	{
		UE_LOG(LogPGXSettings, Warning, TEXT("[FPGXConsoleCommandRegistry] RegisterCommandWithWorldAndArgs failed: %s"), Name);
	}
	return Cmd;
}

void FPGXConsoleCommandRegistry::Unregister(IConsoleCommand* Command)
{
	if (!Command)
	{
		return;
	}

	// EN: UnregisterConsoleObject is idempotent in UE 5.x (no-op on unknown
	//     pointer). We still remove from our list to keep Num() accurate.
	// ES: UnregisterConsoleObject es idempotente en UE 5.x (no-op con puntero
	//     desconocido). Aun removemos de nuestra lista para mantener Num() preciso.
	IConsoleManager::Get().UnregisterConsoleObject(Command);
	RegisteredCommands.RemoveSingleSwap(Command);
}

void FPGXConsoleCommandRegistry::UnregisterAll()
{
	if (RegisteredCommands.Num() == 0)
	{
		return;
	}

	IConsoleManager& Manager = IConsoleManager::Get();
	for (IConsoleCommand* Cmd : RegisteredCommands)
	{
		if (Cmd)
		{
			Manager.UnregisterConsoleObject(Cmd);
		}
	}
	RegisteredCommands.Reset();
}
