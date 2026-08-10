// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "Logging/PGXLogTypes.h"
#include "Logging/PGXLogEntryBuilder.h"
#include "Logging/PGXScopedTimer.h"
#include "Logging/PGXLogCategories.h"

// ═══════════════════════════════════════════════════════════════
// EN: PGX Log Macros - The Developer API
//     All interaction with the PGX Log System goes through these macros.
//     Never call UPGXLogSubsystem methods directly from gameplay code.
//
// ES: PGX Log Macros - La API del Desarrollador
//     Toda interaccion con el Sistema de Logs PGX va a traves de estas macros.
//     Nunca llamar metodos de UPGXLogSubsystem directamente desde codigo gameplay.
// ═══════════════════════════════════════════════════════════════

// EN: Internal helper for token concatenation
// ES: Helper interno para concatenacion de tokens
#define PGX_CONCAT_INNER(A, B) A##B
#define PGX_CONCAT(A, B) PGX_CONCAT_INNER(A, B)

// ─────────────────────────────────────────────────────────────
// SECTION 1: Category Declaration Helpers
// SECCION 1: Helpers de Declaracion de Categorias
// ─────────────────────────────────────────────────────────────

/** EN: Declare a log category in a header / ES: Declarar una categoria de log en un header */
#define PGX_DECLARE_LOG_CATEGORY(ModuleAPI, CategoryName) \
	ModuleAPI DECLARE_LOG_CATEGORY_EXTERN(CategoryName, Log, All)

/** EN: Define a log category in a .cpp / ES: Definir una categoria de log en un .cpp */
#define PGX_DEFINE_LOG_CATEGORY(CategoryName) \
	DEFINE_LOG_CATEGORY(CategoryName)

// ─────────────────────────────────────────────────────────────
// SECTION 2: Verbosity Mapping to UE_LOG
// SECCION 2: Mapeo de Verbosity a UE_LOG
// ─────────────────────────────────────────────────────────────

// EN: Function pointer for log submission — set by UPGXLogSubsystem at init.
//     Using a function pointer decouples PGX_LOG macros from the subsystem
//     header, avoiding circular include chains with PGXLogSubsystem.generated.h
//     (which forward-declares FPGXLogEntry and breaks compilation in Editor
//     modules). No-op by default; logs go to UE_LOG only until subsystem is up.
// ES: Puntero a funcion para envio de log — establecido por UPGXLogSubsystem
//     al inicializar. Usar un function pointer desacopla las macros PGX_LOG
//     del header del subsistema, evitando cadenas de include circular con
//     PGXLogSubsystem.generated.h (que forward-declara FPGXLogEntry y rompe
//     la compilacion en modulos Editor). No-op por defecto; logs solo van
//     a UE_LOG hasta que el subsistema se inicialice.
typedef void (*FPGXLogAddEntryFunc)(FPGXLogEntry&& Entry);
extern PGXCORERUNTIME_API FPGXLogAddEntryFunc GPGXLogAddEntry;

/**
 * EN: Internal macro that bridges PGX log to UE_LOG + subsystem entry.
 *     1. Calls UE_LOG with the appropriate category and verbosity
 *     2. Creates an FPGXLogEntry and submits it via GPGXLogAddEntry
 * ES: Macro interna que conecta PGX log con UE_LOG + entrada del subsistema.
 *     1. Llama a UE_LOG con la categoria y verbosity apropiadas
 *     2. Crea un FPGXLogEntry y lo envia via GPGXLogAddEntry
 */
#define PGX_LOG_INTERNAL(CategoryArg, PgxVerbosity, UeVerbosity, Format, ...) \
	do { \
		UE_LOG(CategoryArg, UeVerbosity, Format, ##__VA_ARGS__); \
		{ \
			FPGXLogEntry _PgxEntry; \
			_PgxEntry.Message = FString::Printf(Format, ##__VA_ARGS__); \
			_PgxEntry.Category = FName(TEXT(#CategoryArg)); \
			_PgxEntry.Verbosity = PgxVerbosity; \
			_PgxEntry.Timestamp = FDateTime::UtcNow(); \
			_PgxEntry.FrameNumber = GFrameCounter; \
			GPGXLogAddEntry(MoveTemp(_PgxEntry)); \
		} \
	} while(0)

// ─────────────────────────────────────────────────────────────
// SECTION 3: Primary Macros
// SECCION 3: Macros Principales
// ─────────────────────────────────────────────────────────────

#if !UE_BUILD_SHIPPING

/**
 * EN: Classic printf-style log. Bridges to both UE_LOG and PGX ring buffer.
 * ES: Log clasico estilo printf. Conecta tanto con UE_LOG como con ring buffer PGX.
 *
 * Usage: PGX_LOG(LogPGXSave, EPGXLogVerbosity::Info, TEXT("Slot '%s' saved"), *SlotName);
 */
#define PGX_LOG(CategoryArg, Verbosity, Format, ...) \
	do { \
		/* EN: Map EPGXLogVerbosity to UE_LOG ELogVerbosity */ \
		/* ES: Mapear EPGXLogVerbosity a ELogVerbosity de UE_LOG */ \
		if ((Verbosity) == EPGXLogVerbosity::Verbose)      { PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Verbose, Verbose, Format, ##__VA_ARGS__); } \
		else if ((Verbosity) == EPGXLogVerbosity::Debug)    { PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Debug, Log, Format, ##__VA_ARGS__); } \
		else if ((Verbosity) == EPGXLogVerbosity::Info)     { PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Info, Log, Format, ##__VA_ARGS__); } \
		else if ((Verbosity) == EPGXLogVerbosity::Warning)  { PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Warning, Warning, Format, ##__VA_ARGS__); } \
		else if ((Verbosity) == EPGXLogVerbosity::Error)    { PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Error, Error, Format, ##__VA_ARGS__); } \
		else if ((Verbosity) == EPGXLogVerbosity::Fatal)    { PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Fatal, Error, Format, ##__VA_ARGS__); } \
	} while(0)

/**
 * EN: Type-adaptive builder log. Returns FPGXLogEntryBuilder for .Add() chaining.
 * ES: Log builder adaptativo por tipo. Retorna FPGXLogEntryBuilder para encadenar .Add().
 *
 * Usage: PGX_LOG_ENTRY(LogPGXSave, EPGXLogVerbosity::Info, "Save completed")
 *            .Add("slot", SlotName)
 *            .Add("bytes", ByteCount);
 */
#define PGX_LOG_ENTRY(CategoryArg, Verbosity, Message) \
	FPGXLogEntryBuilder(FName(TEXT(#CategoryArg)), Verbosity, FString(TEXT(Message)))

// ─────────────────────────────────────────────────────────────
// SECTION 4: Convenience Macros (Classic printf)
// SECCION 4: Macros de Conveniencia (Printf clasico)
// ─────────────────────────────────────────────────────────────

#define PGX_LOG_VERBOSE(CategoryArg, Format, ...)  PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Verbose, Verbose, Format, ##__VA_ARGS__)
#define PGX_LOG_DEBUG(CategoryArg, Format, ...)    PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Debug, Log, Format, ##__VA_ARGS__)
#define PGX_LOG_INFO(CategoryArg, Format, ...)     PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Info, Log, Format, ##__VA_ARGS__)
#define PGX_LOG_WARNING(CategoryArg, Format, ...)  PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Warning, Warning, Format, ##__VA_ARGS__)
#define PGX_LOG_ERROR(CategoryArg, Format, ...)    PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Error, Error, Format, ##__VA_ARGS__)
#define PGX_LOG_FATAL(CategoryArg, Format, ...)    PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Fatal, Error, Format, ##__VA_ARGS__)

// ─────────────────────────────────────────────────────────────
// SECTION 5: Convenience Macros (Builder type-adaptive)
// SECCION 5: Macros de Conveniencia (Builder adaptativo por tipo)
// ─────────────────────────────────────────────────────────────

#define PGX_ENTRY_VERBOSE(CategoryArg, Msg)  PGX_LOG_ENTRY(CategoryArg, EPGXLogVerbosity::Verbose, Msg)
#define PGX_ENTRY_DEBUG(CategoryArg, Msg)    PGX_LOG_ENTRY(CategoryArg, EPGXLogVerbosity::Debug, Msg)
#define PGX_ENTRY_INFO(CategoryArg, Msg)     PGX_LOG_ENTRY(CategoryArg, EPGXLogVerbosity::Info, Msg)
#define PGX_ENTRY_WARNING(CategoryArg, Msg)  PGX_LOG_ENTRY(CategoryArg, EPGXLogVerbosity::Warning, Msg)
#define PGX_ENTRY_ERROR(CategoryArg, Msg)    PGX_LOG_ENTRY(CategoryArg, EPGXLogVerbosity::Error, Msg)

// ─────────────────────────────────────────────────────────────
// SECTION 6: Flow Control Macros
// SECCION 6: Macros de Control de Flujo
// ─────────────────────────────────────────────────────────────

/**
 * EN: Rate-limited: logs at most once per N seconds for this call site.
 * ES: Con limite de frecuencia: loguea como maximo una vez cada N segundos para este call site.
 */
#define PGX_LOG_THROTTLED(CategoryArg, Verbosity, IntervalSeconds, Format, ...) \
	do { \
		static double PGX_CONCAT(_pgxLastLog_, __LINE__) = 0.0; \
		const double _pgxNow = FPlatformTime::Seconds(); \
		if (_pgxNow - PGX_CONCAT(_pgxLastLog_, __LINE__) >= IntervalSeconds) \
		{ \
			PGX_CONCAT(_pgxLastLog_, __LINE__) = _pgxNow; \
			PGX_LOG(CategoryArg, Verbosity, Format, ##__VA_ARGS__); \
		} \
	} while(0)

/**
 * EN: Logs only once per application lifetime for this call site.
 * ES: Loguea solo una vez por vida de la aplicacion para este call site.
 */
#define PGX_LOG_ONCE(CategoryArg, Verbosity, Format, ...) \
	do { \
		static bool PGX_CONCAT(_pgxLogged_, __LINE__) = false; \
		if (!PGX_CONCAT(_pgxLogged_, __LINE__)) \
		{ \
			PGX_CONCAT(_pgxLogged_, __LINE__) = true; \
			PGX_LOG(CategoryArg, Verbosity, Format, ##__VA_ARGS__); \
		} \
	} while(0)

// ─────────────────────────────────────────────────────────────
// SECTION 7: Domain-Aware Log Macros (v3.0)
// SECCION 7: Macros de Log con Dominio (v3.0)
// ─────────────────────────────────────────────────────────────

/**
 * EN: Domain-aware printf-style log. Associates the entry with a specific PGX domain
 *     for polymorphic rendering in the Log Viewer.
 * ES: Log estilo printf con dominio. Asocia la entrada con un dominio PGX especifico
 *     para rendering polimorfico en el Log Viewer.
 *
 * Usage: PGX_LOG_DOMAIN(LogPGXAudio, EPGXLogVerbosity::Info, TAG_PGX_Log_Domain_Audio, TEXT("Playing %s"), *SoundName);
 */
#define PGX_LOG_DOMAIN(CategoryArg, Verbosity, DomainTag, Format, ...) \
	do { \
		UE_LOG(CategoryArg, Log, Format, ##__VA_ARGS__); \
		{ \
			FPGXLogEntry _PgxEntry; \
			_PgxEntry.Message = FString::Printf(Format, ##__VA_ARGS__); \
			_PgxEntry.Category = FName(TEXT(#CategoryArg)); \
			_PgxEntry.Verbosity = Verbosity; \
			_PgxEntry.Timestamp = FDateTime::UtcNow(); \
			_PgxEntry.FrameNumber = GFrameCounter; \
			_PgxEntry.DomainTag = DomainTag; \
			GPGXLogAddEntry(MoveTemp(_PgxEntry)); \
		} \
	} while(0)

/**
 * EN: Domain-aware builder log. Returns FPGXLogEntryBuilder with .Domain() pre-set.
 * ES: Log builder con dominio. Retorna FPGXLogEntryBuilder con .Domain() pre-establecido.
 *
 * Usage: PGX_LOG_DOMAIN_ENTRY(LogPGXAudio, EPGXLogVerbosity::Info, TAG_PGX_Log_Domain_Audio, "Sound started")
 *            .Add("event", EventName)
 *            .Add("volume", Volume);
 */
#define PGX_LOG_DOMAIN_ENTRY(CategoryArg, Verbosity, DomainTag, Message) \
	FPGXLogEntryBuilder(FName(TEXT(#CategoryArg)), Verbosity, FString(TEXT(Message))).Domain(DomainTag)

// ─────────────────────────────────────────────────────────────
// SECTION 8: Scope Timer
// SECCION 8: Temporizador de Scope
// ─────────────────────────────────────────────────────────────

/**
 * EN: RAII scope timer - logs elapsed time when scope exits.
 * ES: Temporizador RAII de scope - loguea tiempo al salir del scope.
 */
#define PGX_LOG_SCOPE_TIMER(CategoryArg, Label) \
	FPGXScopedTimer PGX_CONCAT(_pgxTimer_, __LINE__)(FName(TEXT(#CategoryArg)), TEXT(Label))

#else // UE_BUILD_SHIPPING

// ═══════════════════════════════════════════════════════════════
// EN: SHIPPING BUILD - Zero-cost stripping
//     Verbose/Debug/Timer are no-ops. Warning/Error/Fatal survive.
// ES: BUILD SHIPPING - Eliminacion con coste cero
//     Verbose/Debug/Timer son no-ops. Warning/Error/Fatal sobreviven.
// ═══════════════════════════════════════════════════════════════

#define PGX_LOG(CategoryArg, Verbosity, Format, ...) \
	do { \
		if ((Verbosity) >= EPGXLogVerbosity::Warning) \
		{ \
			if ((Verbosity) == EPGXLogVerbosity::Warning)       { PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Warning, Warning, Format, ##__VA_ARGS__); } \
			else if ((Verbosity) == EPGXLogVerbosity::Error)    { PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Error, Error, Format, ##__VA_ARGS__); } \
			else if ((Verbosity) == EPGXLogVerbosity::Fatal)    { PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Fatal, Error, Format, ##__VA_ARGS__); } \
		} \
	} while(0)

#define PGX_LOG_ENTRY(CategoryArg, Verbosity, Message) \
	((Verbosity) >= EPGXLogVerbosity::Warning) \
		? FPGXLogEntryBuilder(FName(TEXT(#CategoryArg)), Verbosity, FString(TEXT(Message))) \
		: FPGXLogNullBuilder()
// EN: Note: This uses ternary to return the correct type at compile time
// ES: Nota: Esto usa ternario para retornar el tipo correcto en compile time

// EN: Classic shortcuts - Verbose/Debug stripped
// ES: Atajos clasicos - Verbose/Debug eliminados
#define PGX_LOG_VERBOSE(CategoryArg, Format, ...)  do {} while(0)
#define PGX_LOG_DEBUG(CategoryArg, Format, ...)    do {} while(0)
#define PGX_LOG_INFO(CategoryArg, Format, ...)     PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Info, Log, Format, ##__VA_ARGS__)
#define PGX_LOG_WARNING(CategoryArg, Format, ...)  PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Warning, Warning, Format, ##__VA_ARGS__)
#define PGX_LOG_ERROR(CategoryArg, Format, ...)    PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Error, Error, Format, ##__VA_ARGS__)
#define PGX_LOG_FATAL(CategoryArg, Format, ...)    PGX_LOG_INTERNAL(CategoryArg, EPGXLogVerbosity::Fatal, Error, Format, ##__VA_ARGS__)

// EN: Builder shortcuts - Verbose/Debug stripped
// ES: Atajos builder - Verbose/Debug eliminados
#define PGX_ENTRY_VERBOSE(CategoryArg, Msg)  FPGXLogNullBuilder()
#define PGX_ENTRY_DEBUG(CategoryArg, Msg)    FPGXLogNullBuilder()
#define PGX_ENTRY_INFO(CategoryArg, Msg)     PGX_LOG_ENTRY(CategoryArg, EPGXLogVerbosity::Info, Msg)
#define PGX_ENTRY_WARNING(CategoryArg, Msg)  PGX_LOG_ENTRY(CategoryArg, EPGXLogVerbosity::Warning, Msg)
#define PGX_ENTRY_ERROR(CategoryArg, Msg)    PGX_LOG_ENTRY(CategoryArg, EPGXLogVerbosity::Error, Msg)

// EN: Flow control - stripped in shipping
// ES: Control de flujo - eliminados en shipping
#define PGX_LOG_THROTTLED(CategoryArg, Verbosity, IntervalSeconds, Format, ...) do {} while(0)
#define PGX_LOG_ONCE(CategoryArg, Verbosity, Format, ...) do {} while(0)

// EN: Timer - stripped in shipping
// ES: Timer - eliminado en shipping
#define PGX_LOG_SCOPE_TIMER(CategoryArg, Label) do {} while(0)

#endif // UE_BUILD_SHIPPING
