// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"

class UClass;

/**
 * EN: Static registry of PGX observable types. Discovers `UPGXObservableBase` subclasses
 *     automatically at module startup via `TObjectIterator<UClass>` (Pattern A:
 *     auto-registration). For types that cannot inherit `UPGXObservableBase` but still
 *     implement `IPGXObservable` directly (e.g. existing `UPGXConfigDataAsset` hierarchies
 *     that should not restructure), `Register()` provides a manual fallback (Pattern B).
 *
 *     Discovery surface for external tooling (doc generators, validation
 *     pipelines): `GetRegisteredClasses()` returns the full list of UClass entries.
 *     `FindClassByTypeName()` resolves a class by canonical type name FName.
 *
 *     Threading: registry mutation (`Register` / `BootstrapDiscovery`) MUST run on the
 *     Game / Editor thread. Reads (`GetRegisteredClasses` / `FindClassByTypeName`) are
 *     thread-safe IF no concurrent mutation is in flight; PGX convention is read-only
 *     access from Editor / Game thread.
 *
 *     Lifecycle: `BootstrapDiscovery()` is called by `FPGXCoreRuntimeModule::StartupModule()`
 *     (or equivalent module-startup hook) — subclass enumeration happens once at module load.
 *     `Reset()` clears the registry (used by editor hot-reload / module unload).
 *
 *     The concrete TObjectIterator scan implementation lives in
 *     `Private/Observability/PGXObservabilityRegistry.cpp`. The registry remains valid
 *     when no observable subclasses are present.
 *
 * ES: Registro estatico de tipos observables PGX. Auto-descubre subclases de
 *     UPGXObservableBase + manual fallback para tipos que no heredan. Threading: mutaciones
 *     en Game/Editor thread; lecturas thread-safe sin mutacion concurrente.
 */
class PGXCORERUNTIME_API FPGXObservabilityRegistry
{
public:
	/**
	 * EN: Manual registration for non-`UPGXObservableBase` types that implement
	 *     `IPGXObservable` directly. Idempotent — registering the same UClass twice is a
	 *     no-op. Returns true if newly registered; false if already present or invalid input.
	 * ES: Registro manual para tipos no-UPGXObservableBase. Idempotente.
	 */
	static bool Register(UClass* ObservableClass);

	/**
	 * EN: Walks `TObjectIterator<UClass>` and registers all `UPGXObservableBase` subclasses
	 *     plus all classes that implement `IPGXObservable` directly. Called by module
	 *     startup; safe to call multiple times (idempotent).
	 * ES: Itera todas las UClass y registra subclases de UPGXObservableBase + clases que
	 *     implementan IPGXObservable directamente. Llamado en module startup.
	 */
	static void BootstrapDiscovery();

	/** EN: Returns the full list of registered observable UClass entries (snapshot). */
	static TArray<TWeakObjectPtr<UClass>> GetRegisteredClasses();

	/** EN: Number of registered observable classes. */
	static int32 GetRegisteredClassCount();

	/**
	 * EN: Finds a registered class by canonical type name FName (UClass::GetFName()).
	 *     Returns nullptr if not found.
	 * ES: Busca clase registrada por nombre canonico.
	 */
	static UClass* FindClassByTypeName(FName TypeName);

	/**
	 * EN: Clears the registry. Used by editor hot-reload / module unload. Safe to call
	 *     before BootstrapDiscovery() re-runs.
	 * ES: Limpia el registro. Hot-reload / unload safe.
	 */
	static void Reset();
};
