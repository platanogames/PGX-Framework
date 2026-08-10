// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXGameFlowRuntime.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "PGXGameFlowSubsystem.h"
#include "PGXGameFlowConfig.h"
#include "PGXFlowRulesConfig.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Observability/PGXObservabilityRegistry.h"

#define LOCTEXT_NAMESPACE "FPGXGameFlowRuntimeModule"


template <typename TSubsystem>
void FPGXGameFlowRuntimeModule::RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags)
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

void FPGXGameFlowRuntimeModule::RegisterConsoleCommands()
{
	if (!RegisteredConsoleCommands.IsEmpty())
	{
		return;
	}
	RegisterSubsystemConsoleCommand<UPGXGameFlowSubsystem>(TEXT("pgx.gameflow.canchange"), TEXT("Check if transition is valid: pgx.gameflow.canchange <0-7> <Tag>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXGameFlowSubsystem>(TEXT("pgx.gameflow.history"), TEXT("Show transition history: pgx.gameflow.history <0-7>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXGameFlowSubsystem>(TEXT("pgx.gameflow.revert"), TEXT("Revert channel to previous state (dev/editor gated): pgx.gameflow.revert <0-7>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXGameFlowSubsystem>(TEXT("pgx.gameflow.rules"), TEXT("Show rules for a channel: pgx.gameflow.rules <0-7>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXGameFlowSubsystem>(TEXT("pgx.gameflow.set"), TEXT("Set channel state (dev/editor gated): pgx.gameflow.set <0-7> <Tag>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXGameFlowSubsystem>(TEXT("pgx.gameflow.status"), TEXT("Show current state of all 8 GameFlow channels"), ECVF_Default);
}

void FPGXGameFlowRuntimeModule::UnregisterConsoleCommands()
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

void FPGXGameFlowRuntimeModule::StartupModule()
{
	// EN: PGXGameFlowRuntime module started. 8-channel FSM with data-driven validation.
	// ES: Modulo PGXGameFlowRuntime iniciado. FSM de 8 canales con validacion data-driven.
	PGX_LOG_INFO(LogPGX, TEXT("PGXGameFlowRuntime: Module started (8-channel FSM)"));

	// EN: manual fallback registration.
	// ES: registro manual fallback.
	FPGXObservabilityRegistry::Register(UPGXGameFlowConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXFlowRulesConfig::StaticClass());

	RegisterConsoleCommands();
}

void FPGXGameFlowRuntimeModule::ShutdownModule()
{
	UnregisterConsoleCommands();
	// EN: PGXGameFlowRuntime module shut down. Cleanup game flow resources.
	// ES: Modulo PGXGameFlowRuntime detenido. Limpieza de recursos de flujo de juego.
	PGX_LOG_INFO(LogPGX, TEXT("PGXGameFlowRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXGameFlowRuntimeModule, PGXGameFlowRuntime)
