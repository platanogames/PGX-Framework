// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveRuntime.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "PGXSaveSubsystem.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "PGXSaveConfig.h"

#define LOCTEXT_NAMESPACE "FPGXSaveRuntimeModule"


template <typename TSubsystem>
void FPGXSaveRuntimeModule::RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags)
{
	IConsoleCommand* Command = IConsoleManager::Get().RegisterConsoleCommand(
		Name, Help,
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([Name](const TArray<FString>& Args, UWorld* World)
		{
			UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
			TSubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<TSubsystem>() : nullptr;
			if (Subsystem)
			{
				Subsystem->ExecuteConsoleCommand(FString(Name), Args, World);
			}
		}),
		Flags);
	if (Command)
	{
		RegisteredConsoleCommands.Add(Command);
	}
}

void FPGXSaveRuntimeModule::RegisterConsoleCommands()
{
	if (!RegisteredConsoleCommands.IsEmpty())
	{
		return;
	}
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.autosave"), TEXT("Trigger auto-save for a context. Usage: pgx.save.autosave <ContextTag>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.delete"), TEXT("Delete a save slot. Usage: pgx.save.delete <ContextTag> <SlotName>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.info"), TEXT("Show detailed info for a slot. Usage: pgx.save.info <ContextTag> <SlotName>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.list"), TEXT("List all registered save contexts and their domains"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.load"), TEXT("Load a context from a slot. Usage: pgx.save.load <ContextTag> <SlotName>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.quickload"), TEXT("Quick load a context. Usage: pgx.save.quickload <ContextTag>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.quicksave"), TEXT("Quick save a context. Usage: pgx.save.quicksave <ContextTag>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.save"), TEXT("Save a context to a slot. Usage: pgx.save.save <ContextTag> <SlotName>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.slots"), TEXT("List all save slots for a context. Usage: pgx.save.slots <ContextTag>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXSaveSubsystem>(TEXT("pgx.save.stats"), TEXT("Show PGX Save system statistics"), ECVF_Default);
}

void FPGXSaveRuntimeModule::UnregisterConsoleCommands()
{
	IConsoleManager& Manager = IConsoleManager::Get();
	for (IConsoleCommand* Command : RegisteredConsoleCommands)
	{
		if (Command)
		{
			Manager.UnregisterConsoleObject(Command);
		}
	}
	RegisteredConsoleCommands.Reset();
}

void FPGXSaveRuntimeModule::StartupModule()
{
	// EN: PGXSaveRuntime module started. Initializes save/load systems.
	// ES: Modulo PGXSaveRuntime iniciado. Inicializa sistemas de guardado/carga.
	PGX_LOG_INFO(LogPGXSave, TEXT("PGXSaveRuntime: Module started"));

	// Register the authoring configuration with the shared observability registry.
	FPGXObservabilityRegistry::Register(UPGXSaveConfig::StaticClass());

	RegisterConsoleCommands();
}

void FPGXSaveRuntimeModule::ShutdownModule()
{
	UnregisterConsoleCommands();
	// EN: PGXSaveRuntime module shut down. Cleanup save system resources.
	// ES: Modulo PGXSaveRuntime detenido. Limpieza de recursos del sistema de guardado.
	PGX_LOG_INFO(LogPGXSave, TEXT("PGXSaveRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXSaveRuntimeModule, PGXSaveRuntime)
