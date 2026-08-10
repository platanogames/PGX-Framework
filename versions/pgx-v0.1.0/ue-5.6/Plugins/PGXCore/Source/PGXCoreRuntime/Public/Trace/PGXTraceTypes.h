// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Logging/PGXLogTypes.h"
#include "PGXTraceTypes.generated.h"

// ============================================================================
// EN: Traceability Types for PGX Infrastructure v0.4.0
//     Provides system-level trace configuration and BP utility handles.
//
//     Design Evolution: FPGXTraceParams is NOT added to every BP function.
//     Instead, C++ subsystems trace automatically via PGX_TRACE_SCOPE,
//     and BP developers use the Wrapper Pattern with Begin/End Trace nodes.
//
// ES: Tipos de Trazabilidad para Infraestructura PGX v0.4.0
//     Proporciona configuracion de traza a nivel de sistema y handles de utilidad BP.
//
//     Evolucion de Diseno: FPGXTraceParams NO se agrega a cada funcion BP.
//     En su lugar, los subsistemas C++ trazan automaticamente via PGX_TRACE_SCOPE,
//     y los desarrolladores BP usan el Wrapper Pattern con nodos Begin/End Trace.
// ============================================================================

/**
 * EN: System-level tracing configuration. Each system's Config DA embeds one of these.
 *     This is the per-system master switch for automatic C++ tracing.
 *     When disabled, PGX_TRACE_SCOPE macros become no-ops for that system.
 *
 * ES: Configuracion de trazado a nivel de sistema. Cada Config DA de sistema incluye uno.
 *     Este es el interruptor maestro por sistema para trazado automatico C++.
 *     Cuando esta deshabilitado, los macros PGX_TRACE_SCOPE se vuelven no-ops para ese sistema.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXTraceConfig
{
	GENERATED_BODY()

	/**
	 * EN: Master switch — if false, no tracing for this system regardless of other settings.
	 *     Controlled from the Config DA editor, no code changes needed.
	 * ES: Interruptor maestro — si false, no hay trazado para este sistema sin importar otros ajustes.
	 *     Controlado desde el editor del Config DA, no requiere cambios de codigo.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trace")
	bool bSystemTraceEnabled = true;

	/**
	 * EN: Default verbosity for trace entries from this system.
	 *     ENTER/EXIT entries use this verbosity unless overridden.
	 * ES: Verbosity por defecto para entradas de traza de este sistema.
	 *     Entradas ENTER/EXIT usan esta verbosity a menos que se sobreescriba.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Trace")
	EPGXLogVerbosity TraceVerbosity = EPGXLogVerbosity::Debug;
};

/**
 * EN: Handle returned by BeginTrace BP node, consumed by EndTrace.
 *     Enables the Wrapper Pattern in Blueprint: wrap framework calls with
 *     Begin/End Trace to add observability without modifying the framework.
 *
 *     This is the BP-side equivalent of C++'s PGX_TRACE_SCOPE RAII guard.
 *
 * ES: Handle retornado por el nodo BP BeginTrace, consumido por EndTrace.
 *     Habilita el Wrapper Pattern en Blueprint: envolver llamadas del framework con
 *     Begin/End Trace para agregar observabilidad sin modificar el framework.
 *
 *     Este es el equivalente BP del guardia RAII PGX_TRACE_SCOPE de C++.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXTraceHandle
{
	GENERATED_BODY()

	/** EN: Unique trace ID for this operation / ES: ID de traza unico para esta operacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Trace")
	int32 TraceId = -1;

	/** EN: Start time for elapsed calculation / ES: Tiempo de inicio para calculo de elapsed */
	double StartTime = 0.0;

	/** EN: System tag for the traced operation / ES: Tag de sistema para la operacion trazada */
	UPROPERTY()
	FGameplayTag SystemTag;

	/** EN: Operation name for logging / ES: Nombre de operacion para logging */
	UPROPERTY()
	FString OperationName;

	/** EN: Caller location (human-readable) / ES: Ubicacion del caller (legible) */
	UPROPERTY()
	FString Location;

	/** EN: Description/intent of the operation / ES: Descripcion/intencion de la operacion */
	UPROPERTY()
	FString Description;

	/** EN: Whether this handle represents an active trace / ES: Si este handle representa una traza activa */
	bool IsValid() const { return TraceId >= 0; }
};
