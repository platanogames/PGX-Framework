// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCoreRuntime.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "Tags/PGXNativeGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXCoreRuntime, Log, All);

#define LOCTEXT_NAMESPACE "FPGXCoreRuntimeModule"


template <typename TSubsystem>
void FPGXCoreRuntimeModule::RegisterSubsystemConsoleCommand(const TCHAR* Name, const TCHAR* Help, uint32 Flags)
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

void FPGXCoreRuntimeModule::RegisterConsoleCommands()
{
	if (!RegisteredConsoleCommands.IsEmpty())
	{
		return;
	}
	RegisterSubsystemConsoleCommand<UPGXEventHandlerSubsystem>(TEXT("pgx.event.blackbox"), TEXT("Dump blackbox execution history"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXEventHandlerSubsystem>(TEXT("pgx.event.cache"), TEXT("Show handler cache state"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXEventHandlerSubsystem>(TEXT("pgx.event.cache.clear"), TEXT("Clear the handler cache"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXEventHandlerSubsystem>(TEXT("pgx.event.list"), TEXT("List all registered event handlers"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXEventHandlerSubsystem>(TEXT("pgx.event.report"), TEXT("Export a full event handler report"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXEventHandlerSubsystem>(TEXT("pgx.event.status"), TEXT("Display PGX Event Handler status"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXEventHandlerSubsystem>(TEXT("pgx.event.telemetry"), TEXT("Show handler execution telemetry"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLogSubsystem>(TEXT("pgx.log.clear"), TEXT("Clear the PGX log ring buffer"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLogSubsystem>(TEXT("pgx.log.domains"), TEXT("List all registered PGX log domains with entry counts"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLogSubsystem>(TEXT("pgx.log.export"), TEXT("Export PGX logs to JSON file. Usage: pgx.log.export [optional_path]"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLogSubsystem>(TEXT("pgx.log.filter"), TEXT("Filter control. Usage: pgx.log.filter [category level] | pgx.log.filter screen [on|off] | pgx.log.filter global [0-5]"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLogSubsystem>(TEXT("pgx.log.list"), TEXT("List all PGX log categories and their verbosity overrides"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXLogSubsystem>(TEXT("pgx.log.stats"), TEXT("Show PGX log statistics for current session"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXMessageSubsystem>(TEXT("pgx.message.broadcast"), TEXT("Broadcast a test message on a tag (dev/editor only; record marked bIsTestOrigin)"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXMessageSubsystem>(TEXT("pgx.message.channels"), TEXT("List all active message channels"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXMessageSubsystem>(TEXT("pgx.message.history"), TEXT("Show recent message history"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXMessageSubsystem>(TEXT("pgx.message.status"), TEXT("Display PGX Message System status"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXProfileSubsystem>(TEXT("pgx.profile.budget"), TEXT("Query a budget: pgx.profile.budget <name>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXProfileSubsystem>(TEXT("pgx.profile.capability"), TEXT("Query a capability: pgx.profile.capability <name>"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXProfileSubsystem>(TEXT("pgx.profile.dump"), TEXT("Dump full resolved profile details"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXProfileSubsystem>(TEXT("pgx.profile.feature"), TEXT("Query a feature: pgx.profile.feature <name>"), ECVF_Default);
#if WITH_EDITOR
	RegisterSubsystemConsoleCommand<UPGXProfileSubsystem>(TEXT("pgx.profile.simulate.build"), TEXT("Simulate a build context: pgx.profile.simulate.build <Development|Test|Shipping>"), ECVF_Default);
#endif
#if WITH_EDITOR
	RegisterSubsystemConsoleCommand<UPGXProfileSubsystem>(TEXT("pgx.profile.simulate.clear"), TEXT("Clear all simulation overrides and restore original profile"), ECVF_Default);
#endif
#if WITH_EDITOR
	RegisterSubsystemConsoleCommand<UPGXProfileSubsystem>(TEXT("pgx.profile.simulate.platform"), TEXT("Simulate a target platform: pgx.profile.simulate.platform <PC|Console_PS|Console_Xbox|Console_Switch|Mobile_iOS|Mobile_Android|XR>"), ECVF_Default);
#endif
#if WITH_EDITOR
	RegisterSubsystemConsoleCommand<UPGXProfileSubsystem>(TEXT("pgx.profile.simulate.status"), TEXT("Show current simulation status"), ECVF_Default);
#endif
	RegisterSubsystemConsoleCommand<UPGXProfileSubsystem>(TEXT("pgx.profile.status"), TEXT("Print current profile status summary"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXDataRegistrySubsystem>(TEXT("pgx.registry.definitions"), TEXT("EN: List active registry definitions / ES: Listar definiciones activas de registro"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXDataRegistrySubsystem>(TEXT("pgx.registry.export"), TEXT("EN: Export a database as JSON / ES: Exportar una base de datos como JSON"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXDataRegistrySubsystem>(TEXT("pgx.registry.export.all"), TEXT("EN: Export all databases as JSON / ES: Exportar todas las bases de datos como JSON"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXDataRegistrySubsystem>(TEXT("pgx.registry.ingest"), TEXT("EN: Force re-ingest all definitions / ES: Forzar re-ingesta de todas las definiciones"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXDataRegistrySubsystem>(TEXT("pgx.registry.list"), TEXT("EN: List all databases / ES: Listar todas las bases de datos"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXDataRegistrySubsystem>(TEXT("pgx.registry.stats"), TEXT("EN: Show stats for a database / ES: Mostrar stats de una base de datos"), ECVF_Default);
	RegisterSubsystemConsoleCommand<UPGXDataRegistrySubsystem>(TEXT("pgx.registry.validate"), TEXT("EN: Run validation on databases / ES: Ejecutar validacion en bases de datos"), ECVF_Default);
}

void FPGXCoreRuntimeModule::UnregisterConsoleCommands()
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

void FPGXCoreRuntimeModule::StartupModule()
{
	// EN: PGXCoreRuntime is the first PGX module to initialize.
	//     All other PGX modules depend on this one.
	// ES: PGXCoreRuntime es el primer modulo PGX en inicializarse.
	//     Todos los demas modulos PGX dependen de este.
	UE_LOG(LogPGXCoreRuntime, Log, TEXT("PGXCoreRuntime: Module started"));

	// EN: Bootstrap the PGX Observability Registry (IPGXObservable framework
	//     auto-discovery). Walks TObjectIterator<UClass>
	//     and registers all `UPGXObservableBase` subclasses + classes implementing
	//     `IPGXObservable` directly. Idempotent — safe to re-invoke during hot-reload. The
	//     registry header (`PGXObservabilityRegistry.h`) declares this lifecycle binding;
	//     this call site fulfills the contract by wiring BootstrapDiscovery to module startup,
	//     paired with Reset() in ShutdownModule.
	// ES: Bootstrap del Registry de Observability — auto-descubre subclases UPGXObservableBase
	//     + implementaciones IPGXObservable directas. Idempotente.
	FPGXObservabilityRegistry::BootstrapDiscovery();

	RegisterConsoleCommands();
}

void FPGXCoreRuntimeModule::ShutdownModule()
{
	UnregisterConsoleCommands();
	// EN: Reset the PGX Observability Registry — clears the registered observable UClass
	//     entries. Hot-reload / module-unload safe; pairs with `BootstrapDiscovery()` in
	//     StartupModule. Order: registry reset BEFORE the module-shutdown log line so any
	//     post-reset diagnostics still see a clean log statement.
	// ES: Reset del Registry de Observability — clears entries de UClass observables.
	//     Pair con BootstrapDiscovery() en StartupModule.
	FPGXObservabilityRegistry::Reset();

	// EN: Cleanup core systems before unloading
	// ES: Limpiar sistemas centrales antes de descargar
	UE_LOG(LogPGXCoreRuntime, Log, TEXT("PGXCoreRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXCoreRuntimeModule, PGXCoreRuntime)
