// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Trace/PGXTraceTypes.h"
#include "Logging/PGXLogTypes.h"

/**
 * EN: Static helper for trace operations. Central authority for:
 *     - System trace configuration registry (filled by each subsystem at Initialize)
 *     - Trace availability checks (LogSubsystem alive, not shipping)
 *     - Single-point trace entry submission
 *     - BeginTrace/EndTrace for Blueprint wrapper pattern support
 *
 *     Each subsystem calls RegisterSystemTraceConfig() during Initialize(),
 *     passing its Config DA's FPGXTraceConfig. The helper then knows which
 *     systems have tracing enabled without coupling to specific Config DA types.
 *
 * ES: Helper estatico para operaciones de traza. Autoridad central para:
 *     - Registro de configuracion de traza por sistema (llenado por cada subsistema al Initialize)
 *     - Verificaciones de disponibilidad de traza (LogSubsystem vivo, no shipping)
 *     - Envio de entradas de traza en punto unico
 *     - BeginTrace/EndTrace para soporte del wrapper pattern en Blueprint
 *
 *     Cada subsistema llama RegisterSystemTraceConfig() durante Initialize(),
 *     pasando el FPGXTraceConfig de su Config DA. El helper entonces sabe que
 *     sistemas tienen trazado habilitado sin acoplarse a tipos especificos de Config DA.
 */
class PGXCORERUNTIME_API FPGXTraceHelper
{
public:
	// ========================================================================
	// EN: System Configuration Registry / ES: Registro de Configuracion de Sistema
	// ========================================================================

	/**
	 * EN: Register a system's trace configuration. Called by each subsystem during Initialize().
	 * ES: Registrar la configuracion de traza de un sistema. Llamado por cada subsistema durante Initialize().
	 */
	static void RegisterSystemTraceConfig(FGameplayTag SystemTag, const FPGXTraceConfig& Config);

	/**
	 * EN: Unregister a system's trace configuration. Called during Deinitialize().
	 * ES: Desregistrar la configuracion de traza de un sistema. Llamado durante Deinitialize().
	 */
	static void UnregisterSystemTraceConfig(FGameplayTag SystemTag);

	/**
	 * EN: Update an existing system's trace config (e.g., when Config DA is reloaded).
	 * ES: Actualizar la config de traza de un sistema existente (e.g., cuando el Config DA se recarga).
	 */
	static void UpdateSystemTraceConfig(FGameplayTag SystemTag, const FPGXTraceConfig& Config);

	// ========================================================================
	// EN: Availability Checks / ES: Verificaciones de Disponibilidad
	// ========================================================================

	/**
	 * EN: Check if tracing infrastructure is currently available.
	 *     Returns false in Shipping builds, when LogSubsystem doesn't exist,
	 *     or during engine shutdown.
	 * ES: Verificar si la infraestructura de trazado esta disponible.
	 *     Retorna false en builds Shipping, cuando LogSubsystem no existe,
	 *     o durante shutdown del engine.
	 */
	static bool IsTracingAvailable();

	/**
	 * EN: Check if a specific system has tracing enabled via its Config DA.
	 *     Returns false if system not registered or bSystemTraceEnabled is false.
	 * ES: Verificar si un sistema especifico tiene trazado habilitado via su Config DA.
	 *     Retorna false si el sistema no esta registrado o bSystemTraceEnabled es false.
	 */
	static bool IsSystemTraceEnabled(FGameplayTag SystemTag);

	/**
	 * EN: Get the configured trace verbosity for a system.
	 *     Returns Debug if system not registered (safe default).
	 * ES: Obtener la verbosity de traza configurada para un sistema.
	 *     Retorna Debug si el sistema no esta registrado (default seguro).
	 */
	static EPGXLogVerbosity GetSystemTraceVerbosity(FGameplayTag SystemTag);

	// ========================================================================
	// EN: Trace Submission / ES: Envio de Trazas
	// ========================================================================

	/**
	 * EN: Submit a single trace point entry to the LogSubsystem.
	 *     Used by PGX_TRACE_POINT macro for lightweight one-shot traces.
	 * ES: Enviar una entrada de punto de traza al LogSubsystem.
	 *     Usado por el macro PGX_TRACE_POINT para trazas ligeras de un disparo.
	 */
	static void LogTracePoint(FName Category, FGameplayTag SystemTag,
		EPGXLogVerbosity Verbosity, const TCHAR* Message, const char* FunctionName);

	// ========================================================================
	// EN: Blueprint Trace API (Begin/End Pattern) / ES: API de Traza Blueprint
	// ========================================================================

	/**
	 * EN: Begin a trace operation (BP wrapper pattern support).
	 *     Returns a handle that must be passed to EndTrace().
	 *     If tracing is disabled for the system, returns an invalid handle (no overhead).
	 *
	 * ES: Iniciar una operacion de traza (soporte para wrapper pattern BP).
	 *     Retorna un handle que debe pasarse a EndTrace().
	 *     Si el trazado esta deshabilitado para el sistema, retorna handle invalido (sin overhead).
	 */
	static FPGXTraceHandle BeginTrace(FGameplayTag SystemTag, const FString& OperationName,
		const FString& Location, const FString& Description);

	/**
	 * EN: End a trace operation started by BeginTrace.
	 *     Logs EXIT with elapsed time and result.
	 *     Safe to call with invalid handle (no-op).
	 *
	 * ES: Finalizar una operacion de traza iniciada por BeginTrace.
	 *     Loguea EXIT con tiempo transcurrido y resultado.
	 *     Seguro llamar con handle invalido (no-op).
	 */
	static void EndTrace(const FPGXTraceHandle& Handle, bool bSuccess,
		const FString& ErrorReason = FString());

	/**
	 * EN: Get the number of registered system trace configs (diagnostics).
	 * ES: Obtener el numero de configs de traza de sistema registradas (diagnosticos).
	 */
	static int32 GetRegisteredSystemCount();

	/**
	 * EN: Clear all registered configs (called during engine shutdown).
	 * ES: Limpiar todas las configs registradas (llamado durante shutdown del engine).
	 */
	static void ClearAll();

private:
	// EN: System trace configuration registry / ES: Registro de configuracion de traza de sistema
	static TMap<FGameplayTag, FPGXTraceConfig> SystemTraceConfigs;
	static FCriticalSection ConfigLock;

	// EN: Counter for BeginTrace handle IDs / ES: Contador para IDs de handle de BeginTrace
	static int32 NextTraceId;
};
