// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCoreRuntime.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "Tags/PGXNativeGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXCoreRuntime, Log, All);

#define LOCTEXT_NAMESPACE "FPGXCoreRuntimeModule"

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
}

void FPGXCoreRuntimeModule::ShutdownModule()
{
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
