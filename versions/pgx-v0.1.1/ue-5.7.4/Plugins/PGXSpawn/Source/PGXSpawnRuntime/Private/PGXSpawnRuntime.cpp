// Copyright PGX Framework. All Rights Reserved.

#include "PGXSpawnRuntime.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "PGXSpawnConfig.h"
#include "PGXSpawnSubsystem.h"
#include "PGXWaveDefinition.h"
#include "Logging/PGXLogMacros.h"
#include "Subsystems/PGXLogSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXSpawn, Log, All);

#define LOCTEXT_NAMESPACE "FPGXSpawnRuntimeModule"

void FPGXSpawnRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXSpawn, TEXT("PGXSpawnRuntime: Module started."));

	FPGXObservabilityRegistry::Register(UPGXSpawnConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXWaveDefinition::StaticClass());
	RegisterConsoleCommands();
}

void FPGXSpawnRuntimeModule::ShutdownModule()
{
	UnregisterConsoleCommands();
	PGX_LOG_INFO(LogPGXSpawn, TEXT("PGXSpawnRuntime: Module shut down."));
}

void FPGXSpawnRuntimeModule::RegisterConsoleCommands()
{
	IConsoleManager& Manager = IConsoleManager::Get();

	RegisteredConsoleCommands.Add(Manager.RegisterConsoleCommand(
		TEXT("pgx.spawn.list"),
		TEXT("PGXSpawn: dump all active spawn records to log."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(this, &FPGXSpawnRuntimeModule::HandleConsoleList),
		ECVF_Default));
	RegisteredConsoleCommands.Add(Manager.RegisterConsoleCommand(
		TEXT("pgx.spawn.cleanup"),
		TEXT("PGXSpawn: force cleanup of all inactive records."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(this, &FPGXSpawnRuntimeModule::HandleConsoleCleanup),
		ECVF_Default));
	RegisteredConsoleCommands.Add(Manager.RegisterConsoleCommand(
		TEXT("pgx.spawn.budget"),
		TEXT("PGXSpawn: show budget state (current / peak / max)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(this, &FPGXSpawnRuntimeModule::HandleConsoleBudget),
		ECVF_Default));
	RegisteredConsoleCommands.Add(Manager.RegisterConsoleCommand(
		TEXT("pgx.spawn.waves"),
		TEXT("PGXSpawn: show active waves."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(this, &FPGXSpawnRuntimeModule::HandleConsoleWaves),
		ECVF_Default));
	RegisteredConsoleCommands.Add(Manager.RegisterConsoleCommand(
		TEXT("pgx.spawn.triggerpoint"),
		TEXT("PGXSpawn: trigger spawn at a specific SpawnPoint. Usage: pgx.spawn.triggerpoint <PointName>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(this, &FPGXSpawnRuntimeModule::HandleConsoleTriggerPoint),
		ECVF_Default));
	RegisteredConsoleCommands.Add(Manager.RegisterConsoleCommand(
		TEXT("pgx.spawn.pool.clear"),
		TEXT("PGXSpawn: clear the object pool (destroys pooled actors)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateRaw(this, &FPGXSpawnRuntimeModule::HandleConsolePoolClear),
		ECVF_Default));
}

void FPGXSpawnRuntimeModule::UnregisterConsoleCommands()
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

UPGXSpawnSubsystem* FPGXSpawnRuntimeModule::ResolveSpawnSubsystem(UWorld* World, const TCHAR* CommandName) const
{
	if (!World)
	{
		PGX_LOG_WARNING(LogPGXSpawn, TEXT("PGXSpawn: %s failed - no World."), CommandName);
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	UPGXSpawnSubsystem* SpawnSubsystem = GameInstance ? World->GetSubsystem<UPGXSpawnSubsystem>() : nullptr;
	if (!SpawnSubsystem)
	{
		PGX_LOG_WARNING(LogPGXSpawn, TEXT("PGXSpawn: %s failed - subsystem unavailable for World '%s'."),
			CommandName, *World->GetName());
	}
	return SpawnSubsystem;
}

void FPGXSpawnRuntimeModule::HandleConsoleList(const TArray<FString>& Args, UWorld* World)
{
	if (UPGXSpawnSubsystem* Spawn = ResolveSpawnSubsystem(World, TEXT("pgx.spawn.list"))) Spawn->HandleConsoleList(Args);
}

void FPGXSpawnRuntimeModule::HandleConsoleCleanup(const TArray<FString>& Args, UWorld* World)
{
	if (UPGXSpawnSubsystem* Spawn = ResolveSpawnSubsystem(World, TEXT("pgx.spawn.cleanup"))) Spawn->HandleConsoleCleanup(Args);
}

void FPGXSpawnRuntimeModule::HandleConsoleBudget(const TArray<FString>& Args, UWorld* World)
{
	if (UPGXSpawnSubsystem* Spawn = ResolveSpawnSubsystem(World, TEXT("pgx.spawn.budget"))) Spawn->HandleConsoleBudget(Args);
}

void FPGXSpawnRuntimeModule::HandleConsoleWaves(const TArray<FString>& Args, UWorld* World)
{
	if (UPGXSpawnSubsystem* Spawn = ResolveSpawnSubsystem(World, TEXT("pgx.spawn.waves"))) Spawn->HandleConsoleWaves(Args);
}

void FPGXSpawnRuntimeModule::HandleConsoleTriggerPoint(const TArray<FString>& Args, UWorld* World)
{
	if (UPGXSpawnSubsystem* Spawn = ResolveSpawnSubsystem(World, TEXT("pgx.spawn.triggerpoint"))) Spawn->HandleConsoleTriggerPoint(Args);
}

void FPGXSpawnRuntimeModule::HandleConsolePoolClear(const TArray<FString>& Args, UWorld* World)
{
	if (UPGXSpawnSubsystem* Spawn = ResolveSpawnSubsystem(World, TEXT("pgx.spawn.pool.clear"))) Spawn->HandleConsolePoolClear(Args);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXSpawnRuntimeModule, PGXSpawnRuntime)
