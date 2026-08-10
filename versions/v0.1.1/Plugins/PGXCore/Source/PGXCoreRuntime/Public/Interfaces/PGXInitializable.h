// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PGXInitializable.generated.h"

/**
 * EN: Contract for objects with ordered initialization and shutdown
 *     with dependency support.
 *
 *     IsReady() and IsInitialized() are inline-default virtual methods.
 *     Existing implementers continue to compile without changes; new
 *     implementers MAY override to surface async-init / dependency-wait
 *     state to consumers without breaking the contract. IsReady() defaults
 *     to true (synchronous-init happy path); IsInitialized() defaults to
 *     false (fail-safe — not initialized until PGXInitialize() runs).
 *     IsInitialized defaults to false so callers never assume initialization
 *     before PGXInitialize() has completed.
 *
 * ES: Contrato para objetos con inicializacion y shutdown ordenados
 *     con soporte de dependencias.
 *
 *     IsReady() (default true) e IsInitialized() (default false, fail-safe)
 *     agregadas. Los implementadores existentes compilan sin cambios; los
 *     nuevos pueden override para exponer estado async-init / dependency-wait.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UPGXInitializable : public UInterface
{
	GENERATED_BODY()
};

class PGXCORERUNTIME_API IPGXInitializable
{
	GENERATED_BODY()

public:
	/** EN: Called during ordered initialization / ES: Llamado durante la inicializacion ordenada */
	virtual void PGXInitialize() = 0;

	/** EN: Called during ordered shutdown / ES: Llamado durante el shutdown ordenado */
	virtual void PGXShutdown() = 0;

	/**
	 * EN: Reports whether the object is ready to receive work. Default
	 *     implementation returns true (synchronous-init happy path).
	 *     Override to return false during async initialization, warm-up,
	 *     dependency-wait, or any other transient state where consumers
	 *     should defer work.
	 *
	 *     Consumers (Harness, dependent subsystems, editor inspectors)
	 *     should call IsReady() before invoking public API methods.
	 *
	 * ES: Reporta si el objeto esta listo para recibir trabajo. Default
	 *     retorna true (synchronous-init happy path). Override a false
	 *     durante async-init, warm-up, dependency-wait, o cualquier estado
	 *     transitorio donde los consumers deben diferir trabajo.
	 */
	virtual bool IsReady() const { return true; }

	/**
	 * EN: Reports whether PGXInitialize() has been called. Default returns
	 *     FALSE (fail-safe): a stateless interface cannot observe init state,
	 *     and an object has NOT been initialized until PGXInitialize() runs.
	 *     A default of true would report "initialized" for an object that
	 *     merely loaded (PluginLoaded) without Initialize being called — a
	 *     false positive. Implementers that track init MUST override to return
	 *     a real bInitialized flag (false before PGXInitialize(), true after).
	 *
	 *     This keeps the method coherent with its stated purpose: distinguish
	 *     "never initialized" (default false / PluginLoaded) from "initialized"
	 *     (override returns true). Pair with IsReady() for the full lifecycle.
	 *
	 *     The default is false: returning true before initialization would
	 *     collapse the distinction between plugin load and initialized state,
	 *     preventing callers from observing the
	 *     PluginLoaded state. False is the conservative, purpose-coherent
	 *     default and preserves compatibility for implementers that do not override it.
	 *
	 * ES: Reporta si PGXInitialize() ha sido llamado. Default retorna FALSE
	 *     (fail-safe): una interfaz sin estado no puede observar el init, y un
	 *     objeto NO esta inicializado hasta que PGXInitialize() corre. Un
	 *     default true reportaria "initialized" para un objeto solo-cargado
	 *     (falso positivo). Los implementadores que rastrean init DEBEN
	 *     override con un bInitialized real.
	 */
	virtual bool IsInitialized() const { return false; }
};
