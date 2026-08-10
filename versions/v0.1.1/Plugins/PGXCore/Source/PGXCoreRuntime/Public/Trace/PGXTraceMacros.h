// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "Trace/PGXScopedTrace.h"
#include "Logging/PGXLogMacros.h" // PGX_CONCAT

// ============================================================================
// EN: PGX Trace Macros — The C++ Internal Tracing API
//     Complements PGXLogMacros.h — focused on automatic operation tracing.
//
//     Design: These macros are used INSIDE subsystem methods. They check the
//     system's Config DA toggle (bSystemTraceEnabled) automatically.
//     NO caller-provided trace params needed — tracing is infrastructure.
//
//     In Shipping builds: all macros compile to no-ops (zero cost).
//
// ES: PGX Trace Macros — La API Interna de Trazado C++
//     Complementa PGXLogMacros.h — enfocado en trazado automatico de operaciones.
//
//     Diseno: Estos macros se usan DENTRO de metodos de subsistema. Verifican
//     el toggle del Config DA del sistema (bSystemTraceEnabled) automaticamente.
//     NO se necesitan params de traza del caller — el trazado es infraestructura.
//
//     En builds Shipping: todos los macros compilan a no-ops (cero costo).
// ============================================================================

#if !UE_BUILD_SHIPPING

/**
 * EN: RAII operation trace guard. Creates a scoped trace object that:
 *     - On construction: logs "ENTER OpName" with system tag + function name
 *     - On destruction:  logs "EXIT OpName" with elapsed time + result
 *     Checks system Config DA toggle internally — no overhead if disabled.
 *
 *     The created variable is named 'Trace', accessible for:
 *       Trace.SetSuccess();
 *       Trace.SetError("reason");
 *       Trace.AddParam("Key", "Value");
 *
 * ES: Guardia de traza RAII. Crea un objeto de traza con scope que:
 *     - Al construirse: loguea "ENTER OpName" con tag de sistema + nombre de funcion
 *     - Al destruirse:  loguea "EXIT OpName" con tiempo transcurrido + resultado
 *     Verifica toggle del Config DA del sistema internamente — sin overhead si deshabilitado.
 *
 *     La variable creada se llama 'Trace', accesible para:
 *       Trace.SetSuccess();
 *       Trace.SetError("razon");
 *       Trace.AddParam("Key", "Value");
 *
 * Usage / Uso:
 *   void UPGXSaveSubsystem::SaveContext(FGameplayTag ContextTag, const FString& SlotName)
 *   {
 *       PGX_TRACE_SCOPE(LogPGXSave, TAG_PGX_System_Save, "SaveContext");
 *       Trace.AddParam("ContextTag", ContextTag.ToString());
 *       Trace.AddParam("SlotName", SlotName);
 *       // ... implementation ...
 *       Trace.SetSuccess();
 *   }
 */
#define PGX_TRACE_SCOPE(Category, SystemTag, OpName) \
	FPGXScopedTrace Trace( \
		FName(TEXT(#Category)), SystemTag, TEXT(OpName), __FUNCTION__)

/**
 * EN: Lightweight trace — logs a single entry, no RAII.
 *     Use for simple operations that don't need entry/exit tracking.
 *     Checks system trace toggle before logging.
 *
 * ES: Traza ligera — loguea una entrada unica, sin RAII.
 *     Usar para operaciones simples que no necesitan tracking de entrada/salida.
 *     Verifica toggle de traza del sistema antes de loguear.
 *
 * Usage / Uso:
 *   PGX_TRACE_POINT(LogPGXSave, TAG_PGX_System_Save, EPGXLogVerbosity::Info, "Config loaded");
 */
#define PGX_TRACE_POINT(Category, SystemTag, Verbosity, Message) \
	do { \
		if (FPGXTraceHelper::IsSystemTraceEnabled(SystemTag)) \
		{ \
			FPGXTraceHelper::LogTracePoint(FName(TEXT(#Category)), SystemTag, \
				Verbosity, TEXT(Message), __FUNCTION__); \
		} \
	} while(0)

#else // UE_BUILD_SHIPPING

// EN: In Shipping builds: PGX_TRACE_SCOPE creates a no-op object so that
//     Trace.SetSuccess(), Trace.SetError(), Trace.AddParam() still compile.
//     All methods are empty inline functions optimized away by the compiler.
// ES: En builds Shipping: PGX_TRACE_SCOPE crea un objeto no-op para que
//     Trace.SetSuccess(), Trace.SetError(), Trace.AddParam() aun compilen.
//     Todos los metodos son funciones inline vacias optimizadas por el compilador.

#define PGX_TRACE_SCOPE(Category, SystemTag, OpName) \
	FPGXScopedTraceNoop Trace

#define PGX_TRACE_POINT(Category, SystemTag, Verbosity, Message) \
	do {} while(0)

#endif // UE_BUILD_SHIPPING
