// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

/**
 * EN: RAII helper that registers console commands on construction and
 *     unregisters them all on destruction. Replaces the 100+ duplicated
 *     console-command boilerplate patterns across the 15 PGX subsystems
 *     so console-command lifetimes remain symmetric and reviewable.
 *
 *     Typical usage:
 *
 *     class UMySubsystem : public UGameInstanceSubsystem
 *     {
 *     public:
 *         void Initialize(FSubsystemCollectionBase& Collection) override
 *         {
 *             Super::Initialize(Collection);
 *             CmdRegistry = MakeUnique<FPGXConsoleCommandRegistry>();
 *             CmdRegistry->RegisterCommand(
 *                 TEXT("pgx.mysys.dump"),
 *                 TEXT("Dump MySubsystem state to log."),
 *                 FConsoleCommandDelegate::CreateUObject(this, &UMySubsystem::Cmd_Dump));
 *         }
 *
 *         void Deinitialize() override
 *         {
 *             CmdRegistry.Reset();  // unregisters all
 *             Super::Deinitialize();
 *         }
 *
 *     private:
 *         TUniquePtr<FPGXConsoleCommandRegistry> CmdRegistry;
 *     };
 *
 *     Editor-only tooling plugins can anchor the registry to
 *     StartupModule / ShutdownModule
 *     plugins (Tutorials, EditorTools) that don't have a subsystem.
 *
 * ES: Helper RAII que registra comandos de consola en construccion y los
 *     desregistra todos en destruccion. Reemplaza los 100+ boilerplates
 *     duplicados a traves de los 15 subsistemas PGX.
 */
class PGXCORERUNTIME_API FPGXConsoleCommandRegistry
{
public:
	FPGXConsoleCommandRegistry() = default;

	~FPGXConsoleCommandRegistry()
	{
		UnregisterAll();
	}

	// EN: Non-copyable (each instance owns its own IConsoleCommand* list).
	// ES: No copiable.
	FPGXConsoleCommandRegistry(const FPGXConsoleCommandRegistry&) = delete;
	FPGXConsoleCommandRegistry& operator=(const FPGXConsoleCommandRegistry&) = delete;

	/**
	 * EN: Register a no-arg console command. Returns the IConsoleCommand*
	 *     pointer (nullptr on failure) which can be passed to Unregister() if
	 *     the caller wants to remove a specific command early. The pointer
	 *     is also stored internally for UnregisterAll() on destruction.
	 *
	 * ES: Registra un comando de consola sin argumentos. Retorna el puntero
	 *     IConsoleCommand* (nullptr en fallo) que puede pasarse a
	 *     Unregister() si el caller quiere remover un comando especifico
	 *     antes de tiempo.
	 */
	IConsoleCommand* RegisterCommand(
		const TCHAR* Name,
		const TCHAR* Help,
		const FConsoleCommandDelegate& Command,
		uint32 Flags = ECVF_Default);

	/**
	 * EN: Register a console command that takes a TArray<FString> of args.
	 *     Useful for `pgx.foo bar 42` style commands.
	 *
	 * ES: Registra un comando de consola que toma TArray<FString> de args.
	 */
	IConsoleCommand* RegisterCommandWithArgs(
		const TCHAR* Name,
		const TCHAR* Help,
		const FConsoleCommandWithArgsDelegate& Command,
		uint32 Flags = ECVF_Default);

	/**
	 * EN: Register a console command that takes args + a UWorld* context.
	 *     Useful for `pgx.world.spawn foo 100 200 300` style commands.
	 *
	 * ES: Registra un comando de consola que toma args + UWorld* context.
	 */
	IConsoleCommand* RegisterCommandWithWorldAndArgs(
		const TCHAR* Name,
		const TCHAR* Help,
		const FConsoleCommandWithWorldAndArgsDelegate& Command,
		uint32 Flags = ECVF_Default);

	/**
	 * EN: Manually unregister a specific command. Safe to call on a null
	 *     pointer or an already-unregistered command (idempotent).
	 *
	 * ES: Desregistra manualmente un comando especifico. Safe de llamar
	 *     con nullptr o comando ya desregistrado (idempotente).
	 */
	void Unregister(IConsoleCommand* Command);

	/**
	 * EN: Unregister all commands registered through this instance. Called
	 *     automatically by the destructor; exposed for explicit lifecycle
	 *     control (e.g., a subsystem Deinitialize wanting to unregister before
	 *     the registry's destruction).
	 *
	 * ES: Desregistra todos los comandos registrados via esta instancia.
	 *     Llamado automaticamente por el destructor; expuesto para control
	 *     de ciclo de vida explicito.
	 */
	void UnregisterAll();

	/**
	 * EN: Number of currently-registered (live) commands. Stale-removed
	 *     commands don't count.
	 *
	 * ES: Numero de comandos actualmente registrados (live). Comandos
	 *     stale-removidos no cuentan.
	 */
	int32 Num() const { return RegisteredCommands.Num(); }

private:
	/** EN: All registered commands. Unregistered in destructor.
	 *  ES: Todos los comandos registrados. Desregistrados en destructor. */
	TArray<IConsoleCommand*> RegisteredCommands;
};
