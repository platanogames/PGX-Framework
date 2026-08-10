// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXPSORuntimeModule.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "PGXPSOSubsystem.h"
#include "PGXPSOWarmUpConfig.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Observability/PGXObservabilityRegistry.h"

#define LOCTEXT_NAMESPACE "FPGXPSORuntimeModule"


template <typename TSubsystem>
void FPGXPSORuntimeModule::RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags)
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

void FPGXPSORuntimeModule::RegisterConsoleCommands()
{
	if (!RegisteredConsoleCommands.IsEmpty())
	{
		return;
	}
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.cancel"), TEXT("Cancel active PSO warm-up"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.configs"), TEXT("List all discovered PSO WarmUpConfig DataAssets"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.pause"), TEXT("Pause active PSO warm-up"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.progress"), TEXT("Show PSO warm-up progress details"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.record.clear"), TEXT("Clear recorded PSO data"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.record.export"), TEXT("Export recorded PSO data to JSON: pgx.pso.record.export [Path]"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.record.start"), TEXT("Start PSO recording session: pgx.pso.record.start [SessionName]"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.record.stop"), TEXT("Stop PSO recording session"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.resume"), TEXT("Resume paused PSO warm-up"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.save"), TEXT("Save PSO cache to disk"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.stats"), TEXT("Show UE native PSO pipeline cache statistics"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.status"), TEXT("Show PSO subsystem status: state, contexts, configs, cache"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.validate"), TEXT("Validate PSO config entries: pgx.pso.validate [ConfigName] (no args = all)"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXPSOSubsystem>(TEXT("pgx.pso.warmup"), TEXT("Start warm-up: pgx.pso.warmup [ContextTag] (no args = all)"), ECVF_Default);
}

void FPGXPSORuntimeModule::UnregisterConsoleCommands()
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

void FPGXPSORuntimeModule::StartupModule()
{
	// EN: PGXPSORuntime module started. PSO warm-up subsystem will initialize per GameInstance.
	// ES: Modulo PGXPSORuntime iniciado. El subsistema de warm-up PSO se inicializara por GameInstance.
	PGX_LOG_INFO(LogPGXPSO, TEXT("PGXPSORuntime: Module started"));

	// EN: manual fallback registration.
	// ES: registro manual fallback.
	FPGXObservabilityRegistry::Register(UPGXPSOWarmUpConfig::StaticClass());

	RegisterConsoleCommands();
}

void FPGXPSORuntimeModule::ShutdownModule()
{
	UnregisterConsoleCommands();
	// EN: PGXPSORuntime module shut down.
	// ES: Modulo PGXPSORuntime detenido.
	PGX_LOG_INFO(LogPGXPSO, TEXT("PGXPSORuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXPSORuntimeModule, PGXPSORuntime)
