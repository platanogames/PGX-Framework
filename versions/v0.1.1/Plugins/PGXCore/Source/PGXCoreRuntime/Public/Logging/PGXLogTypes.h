// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXLogTypes.generated.h"

// ═══════════════════════════════════════════════════════════════
// EN: Core data types for the PGX Log System.
//     Enumerations, typed parameters, log entries, and sessions.
// ES: Tipos de datos centrales para el Sistema de Logs PGX.
//     Enumeraciones, parametros tipados, entradas de log y sesiones.
// ═══════════════════════════════════════════════════════════════

/**
 * EN: Log severity level. Maps to UE_LOG verbosity for the UE_LOG bridge.
 *     Verbose/Debug are compiled out in Shipping builds (zero-cost).
 * ES: Nivel de severidad de log. Se mapea a la verbosity de UE_LOG para el puente.
 *     Verbose/Debug se eliminan en builds Shipping (coste cero).
 */
UENUM(BlueprintType)
enum class EPGXLogVerbosity : uint8
{
	Verbose,    // EN: Detailed trace / ES: Traza detallada
	Debug,      // EN: Development diagnostics / ES: Diagnosticos de desarrollo
	Info,       // EN: Operational messages / ES: Mensajes operacionales
	Warning,    // EN: Potential issues / ES: Problemas potenciales
	Error,      // EN: Errors needing attention / ES: Errores que requieren atencion
	Fatal       // EN: Unrecoverable / ES: Irrecuperable
};

/**
 * EN: Identifies the C++ type of data attached to a log parameter.
 *     Used by the builder, JSON exporter, and Widget Developer for typed display.
 * ES: Identifica el tipo C++ del dato adjunto a un parametro de log.
 *     Usado por el builder, exportador JSON y Widget Developer para display tipado.
 */
UENUM()
enum class EPGXLogDataType : uint8
{
	String,         // FString, FName, FText
	Integer,        // int32, int64, uint32
	Float,          // float, double
	Boolean,        // bool
	Vector,         // FVector
	Rotator,        // FRotator
	Transform,      // FTransform
	GameplayTag,    // FGameplayTag
	Object,         // UObject* → logged as Name + Class
	Color,          // FLinearColor, FColor
	Custom          // Anything with LexToString fallback
};

/**
 * EN: A single typed parameter attached to a log entry.
 *     Stores key, stringified value, and original data type for rich export.
 * ES: Un parametro tipado individual adjunto a una entrada de log.
 *     Almacena clave, valor stringificado y tipo de dato original para exportacion enriquecida.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXLogParam
{
	GENERATED_BODY()

	/** EN: Parameter name / ES: Nombre del parametro */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	FString Key;

	/** EN: Stringified value / ES: Valor stringificado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	FString Value;

	/** EN: Original C++ data type / ES: Tipo de dato C++ original */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	EPGXLogDataType DataType = EPGXLogDataType::String;
};

/**
 * EN: A single structured log entry with typed parameters.
 *     Created by PGX_LOG() macros or FPGXLogEntryBuilder, stored in the ring buffer.
 * ES: Una entrada de log estructurada con parametros tipados.
 *     Creada por macros PGX_LOG() o FPGXLogEntryBuilder, almacenada en el ring buffer.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXLogEntry
{
	GENERATED_BODY()

	/** EN: Main message text / ES: Texto del mensaje principal */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	FString Message;

	/** EN: Log category name (LogPGX, LogPGXSave...) / ES: Nombre de categoria */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	FName Category;

	/** EN: Severity level / ES: Nivel de severidad */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	EPGXLogVerbosity Verbosity = EPGXLogVerbosity::Info;

	/** EN: When the log was created / ES: Cuando se creo el log */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	FDateTime Timestamp;

	/**
	 * EN: Engine frame number. Not BlueprintReadOnly (uint64 not supported in BP).
	 * ES: Frame number del engine. No BlueprintReadOnly (uint64 no soportado en BP).
	 */
	uint64 FrameNumber = 0;

	/** EN: Session ID for grouping / ES: ID de sesion para agrupacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	int32 SessionId = 0;

	/** EN: Typed parameters from builder .Add() calls / ES: Parametros tipados de llamadas .Add() del builder */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	TArray<FPGXLogParam> Params;

	/** EN: GameplayTags for filtering and categorization / ES: GameplayTags para filtrado y categorizacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	FGameplayTagContainer Tags;

	/**
	 * EN: Domain tag identifying which subsystem produced this entry (e.g. PGX.Log.Domain.Audio).
	 *     Used by the polymorphic viewer to select the appropriate renderer.
	 *     Empty = generic domain (auto-inferred from Category if possible).
	 * ES: Tag de dominio que identifica cual subsistema produjo esta entrada (e.g. PGX.Log.Domain.Audio).
	 *     Usado por el viewer polimorfico para seleccionar el renderer apropiado.
	 *     Vacio = dominio generico (auto-inferido de Category si es posible).
	 */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	FGameplayTag DomainTag;
};

/**
 * EN: Metadata for a logging session. A new session starts on each subsystem initialization.
 *     Tracks entry counts and timing for analytics and export.
 * ES: Metadatos de una sesion de logging. Una nueva sesion inicia con cada inicializacion del subsistema.
 *     Rastrea conteos de entradas y tiempos para analiticas y exportacion.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXLogSession
{
	GENERATED_BODY()

	/** EN: Unique session identifier / ES: Identificador unico de sesion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	int32 SessionId = 0;

	/** EN: When this session started / ES: Cuando inicio esta sesion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	FDateTime StartTime;

	/** EN: When this session ended (zero if still active) / ES: Cuando termino (cero si aun activa) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	FDateTime EndTime;

	/** EN: Total entries logged / ES: Total de entradas logueadas */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	int32 EntryCount = 0;

	/** EN: Warning-level entries / ES: Entradas nivel Warning */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	int32 WarningCount = 0;

	/** EN: Error-level entries (includes Fatal) / ES: Entradas nivel Error (incluye Fatal) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Log")
	int32 ErrorCount = 0;
};

/**
 * EN: Refresh mode for the Log Viewer panel.
 *     Live: new entries appear immediately with auto-scroll.
 *     Manual: entries accumulate in a delta buffer; user flushes on demand.
 * ES: Modo de refresco para el panel del Log Viewer.
 *     Live: las entradas aparecen inmediatamente con auto-scroll.
 *     Manual: las entradas se acumulan en un buffer delta; el usuario las muestra bajo demanda.
 */
UENUM(BlueprintType)
enum class EPGXLogRefreshMode : uint8
{
	Live    UMETA(DisplayName = "Live"),
	Manual  UMETA(DisplayName = "Manual")
};

/**
 * EN: Defines a single column for domain-specific rendering in the Log Viewer.
 *     Each domain renderer declares which extra columns it wants to display.
 *     The viewer builds column headers from this definition.
 * ES: Define una columna individual para rendering especifico de dominio en el Log Viewer.
 *     Cada renderer de dominio declara que columnas extra quiere mostrar.
 *     El viewer construye headers de columna desde esta definicion.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXLogColumnDef
{
	GENERATED_BODY()

	/** EN: Column header text / ES: Texto del header de columna */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Log")
	FText HeaderText;

	/** EN: Fixed width in pixels (0 = fill remaining) / ES: Ancho fijo en pixeles (0 = llenar restante) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Log")
	float Width = 80.0f;

	/** EN: Param key to extract from FPGXLogEntry::Params / ES: Key de param a extraer de FPGXLogEntry::Params */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Log")
	FString ParamKey;
};
