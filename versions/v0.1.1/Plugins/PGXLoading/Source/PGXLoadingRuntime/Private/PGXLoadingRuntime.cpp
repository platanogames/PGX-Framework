// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingRuntime.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "PGXLevelFlowSubsystem.h"
#include "PGXLoadingSubsystem.h"
#include "Logging/PGXLogMacros.h"

#define LOCTEXT_NAMESPACE "FPGXLoadingRuntimeModule"

DEFINE_LOG_CATEGORY(LogPGXLoading);


template <typename TSubsystem>
void FPGXLoadingRuntimeModule::RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags)
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

void FPGXLoadingRuntimeModule::RegisterConsoleCommands()
{
	if (!RegisteredConsoleCommands.IsEmpty())
	{
		return;
	}
	RegisterSubsystemConsoleCommand<UPGXLevelFlowSubsystem>(TEXT("pgx.level.cancel"), TEXT("Cancel active level transition"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLevelFlowSubsystem>(TEXT("pgx.level.entrypoints"), TEXT("Show entry points from current LevelFlowActor"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLevelFlowSubsystem>(TEXT("pgx.level.history"), TEXT("Show transition history with timing data"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLevelFlowSubsystem>(TEXT("pgx.level.load"), TEXT("Request level transition: pgx.level.load <GameplayTag>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLevelFlowSubsystem>(TEXT("pgx.level.profiles"), TEXT("List discovered level profiles"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLevelFlowSubsystem>(TEXT("pgx.level.resolve"), TEXT("Resolve tag and show level data: pgx.level.resolve <Tag>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLevelFlowSubsystem>(TEXT("pgx.level.status"), TEXT("Show LevelFlow status: current state, level, sub-levels"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLevelFlowSubsystem>(TEXT("pgx.level.sublevels"), TEXT("List sub-levels and their load state"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.close"), TEXT("Force close loading screen"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.config"), TEXT("Show active config values"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.debug"), TEXT("Show detailed debug info (fade state, close conditions, active strategy)"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.history"), TEXT("Show loading history with metrics"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.profiles"), TEXT("List all discovered profiles and context mappings"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.request"), TEXT("Manual RequestLoading with context tag (e.g., pgx.loading.request PGX.Loading.Context.Default)"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.simulate"), TEXT("Simulate loading with artificial delay (e.g., pgx.loading.simulate 3.0)"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.simulate.pso"), TEXT("Simulate PSO delay — blocks close until timeout (e.g., pgx.loading.simulate.pso 5.0)"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.skip"), TEXT("Request skip (validates conditions)"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLoadingSubsystem>(TEXT("pgx.loading.status"), TEXT("Show current loading screen state, context, timing, and conditions"), ECVF_Default);
}

void FPGXLoadingRuntimeModule::UnregisterConsoleCommands()
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

void FPGXLoadingRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXLoading, TEXT("PGXLoadingRuntime: Module started."));

	RegisterConsoleCommands();
}

void FPGXLoadingRuntimeModule::ShutdownModule()
{
	UnregisterConsoleCommands();
	PGX_LOG_INFO(LogPGXLoading, TEXT("PGXLoadingRuntime: Module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXLoadingRuntimeModule, PGXLoadingRuntime)
