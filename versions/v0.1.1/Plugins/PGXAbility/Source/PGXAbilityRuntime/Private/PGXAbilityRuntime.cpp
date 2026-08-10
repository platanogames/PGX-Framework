// Copyright PGX Framework. All Rights Reserved.
#include "PGXAbilityRuntime.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "PGXAbilitySubsystem.h"
#include "Subsystems/PGXLogSubsystem.h"

PGX_DEFINE_LOG_CATEGORY(LogPGXAbility);

/**
 * EN: StartupModule - Called when the PGXAbilityRuntime module is loaded.
 *     Logs initialization of the PGX Ability facade layer.
 * ES: StartupModule - Se llama cuando el modulo PGXAbilityRuntime se carga.
 *     Registra la inicializacion de la capa facade de PGX Ability.
 */

template <typename TSubsystem>
void FPGXAbilityRuntimeModule::RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags)
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

void FPGXAbilityRuntimeModule::RegisterConsoleCommands()
{
	if (!RegisteredConsoleCommands.IsEmpty())
	{
		return;
	}
	RegisterSubsystemConsoleCommand<UPGXAbilitySubsystem>(TEXT("pgx.ability.components"), TEXT("List PGXAbilityComponent instances currently registered with the subsystem."), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXAbilitySubsystem>(TEXT("pgx.ability.status"), TEXT("Print PGXAbilitySubsystem readiness, config source, and aggregate counts."), ECVF_Default);
}

void FPGXAbilityRuntimeModule::UnregisterConsoleCommands()
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

void FPGXAbilityRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXAbility, TEXT("PGXAbilityRuntime module started. GAS facade layer initialized."));

	RegisterConsoleCommands();
}

/**
 * EN: ShutdownModule - Called when the PGXAbilityRuntime module is unloaded.
 *     Logs shutdown of the PGX Ability facade layer.
 * ES: ShutdownModule - Se llama cuando el modulo PGXAbilityRuntime se descarga.
 *     Registra el shutdown de la capa facade de PGX Ability.
 */
void FPGXAbilityRuntimeModule::ShutdownModule()
{
	UnregisterConsoleCommands();
	PGX_LOG_INFO(LogPGXAbility, TEXT("PGXAbilityRuntime module shut down. GAS facade layer deinitialized."));
}

IMPLEMENT_MODULE(FPGXAbilityRuntimeModule, PGXAbilityRuntime)
