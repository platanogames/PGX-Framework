// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Logging/PGXLogTypes.h"

// Forward declarations
class FPGXTraceHelper;

/**
 * EN: RAII trace object created by PGX_TRACE_SCOPE macro.
 *     On construction: checks system trace toggle, logs "ENTER OpName".
 *     On destruction:  logs "EXIT OpName" with elapsed time + state (Success/Error).
 *
 *     If tracing is disabled for the system (Config DA toggle), both construction
 *     and destruction are effectively no-ops (bActive = false, no log entries created).
 *
 *     All entries include the C++ function name for precise code traceability.
 *     Entries are submitted as standard FPGXLogEntry to the LogSubsystem ring buffer,
 *     making them visible in the Log Viewer with tag-based filtering.
 *
 *     Non-copyable, non-movable (RAII scope semantics).
 *
 * ES: Objeto de traza RAII creado por el macro PGX_TRACE_SCOPE.
 *     Al construirse: verifica toggle de traza del sistema, loguea "ENTER OpName".
 *     Al destruirse:  loguea "EXIT OpName" con tiempo transcurrido + estado (Success/Error).
 *
 *     Si el trazado esta deshabilitado para el sistema (toggle del Config DA), tanto
 *     construccion como destruccion son no-ops (bActive = false, no se crean entradas).
 *
 *     Todas las entradas incluyen el nombre de la funcion C++ para trazabilidad precisa.
 *     Las entradas se envian como FPGXLogEntry estandar al ring buffer del LogSubsystem,
 *     haciendolas visibles en el Log Viewer con filtrado por tag.
 *
 *     No-copiable, no-movible (semantica de scope RAII).
 */
class PGXCORERUNTIME_API FPGXScopedTrace
{
public:
	/**
	 * EN: Constructor — checks trace toggle, logs ENTER if active.
	 * ES: Constructor — verifica toggle de traza, loguea ENTER si activo.
	 *
	 * @param InCategory    Log category name (e.g., LogPGXSave)
	 * @param InSystemTag   System identification tag (e.g., TAG_PGX_System_Save)
	 * @param InOpName      Operation name (e.g., "SaveContext")
	 * @param InFunctionName Auto-captured via __FUNCTION__ in macro
	 */
	FPGXScopedTrace(FName InCategory, FGameplayTag InSystemTag,
		const TCHAR* InOpName, const char* InFunctionName);

	/**
	 * EN: Destructor — logs EXIT with elapsed time + result if active.
	 *     If SetSuccess/SetError was never called, logs as "NoResult".
	 * ES: Destructor — loguea EXIT con tiempo transcurrido + resultado si activo.
	 *     Si SetSuccess/SetError nunca fue llamado, loguea como "NoResult".
	 */
	~FPGXScopedTrace();

	// EN: Non-copyable, non-movable / ES: No-copiable, no-movible
	FPGXScopedTrace(const FPGXScopedTrace&) = delete;
	FPGXScopedTrace& operator=(const FPGXScopedTrace&) = delete;
	FPGXScopedTrace(FPGXScopedTrace&&) = delete;
	FPGXScopedTrace& operator=(FPGXScopedTrace&&) = delete;

	/**
	 * EN: Mark operation as successful. Called before scope exit.
	 * ES: Marcar operacion como exitosa. Llamar antes de salir del scope.
	 */
	void SetSuccess();

	/**
	 * EN: Mark operation as failed with optional reason.
	 * ES: Marcar operacion como fallida con razon opcional.
	 */
	void SetError(const FString& Reason = FString());

	/**
	 * EN: Add extra key-value param to the EXIT log entry.
	 *     Useful for enriching trace context (e.g., counts, IDs, sizes).
	 * ES: Agregar param extra key-value a la entrada de log EXIT.
	 *     Util para enriquecer contexto de traza (e.g., conteos, IDs, tamanos).
	 */
	void AddParam(const FString& Key, const FString& Value);

private:
	FName LogCategory;
	FGameplayTag System;
	FString OperationName;
	FString Function;
	double StartTime;
	bool bResultSet = false;
	bool bSuccess = false;
	FString ErrorReason;
	TArray<FPGXLogParam> ExtraParams;

	/**
	 * EN: If false, tracing was disabled at construction — destructor is a no-op.
	 *     Set once in constructor based on IsSystemTraceEnabled + IsTracingAvailable.
	 * ES: Si false, el trazado estaba deshabilitado al construirse — destructor es no-op.
	 *     Se establece una vez en constructor basado en IsSystemTraceEnabled + IsTracingAvailable.
	 */
	bool bActive = false;
};

/**
 * EN: No-op trace object for Shipping builds. All methods are empty inline functions
 *     that get optimized away by the compiler. This ensures code using Trace.SetSuccess()
 *     etc. compiles cleanly in Shipping without #if guards around every call site.
 *
 * ES: Objeto de traza no-op para builds Shipping. Todos los metodos son funciones inline
 *     vacias que se optimizan por el compilador. Esto asegura que codigo usando Trace.SetSuccess()
 *     etc. compile limpiamente en Shipping sin #if guards alrededor de cada call site.
 */
struct PGXCORERUNTIME_API FPGXScopedTraceNoop
{
	FORCEINLINE void SetSuccess() {}
	FORCEINLINE void SetError(const FString& = FString()) {}
	FORCEINLINE void AddParam(const FString&, const FString&) {}
};
