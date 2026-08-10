// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Logging/PGXLogTypes.h"

/**
 * EN: Static serializer for PGX log entries to JSON format.
 *     Receives already-copied data (snapshot from ring buffer).
 *     Never accesses the ring buffer directly — caller provides the snapshot.
 *
 * ES: Serializador estatico para entradas de log PGX a formato JSON.
 *     Recibe datos ya copiados (snapshot del ring buffer).
 *     Nunca accede al ring buffer directamente — el caller provee el snapshot.
 */
class PGXCORERUNTIME_API FPGXLogSerializer
{
public:
	FPGXLogSerializer() = delete;

	/**
	 * EN: Serialize entries and session metadata to a complete JSON string.
	 *     Includes framework version, session info, typed parameters, and tags.
	 * ES: Serializar entradas y metadatos de sesion a un string JSON completo.
	 *     Incluye version del framework, info de sesion, parametros tipados y tags.
	 */
	static FString ToJson(const TArray<FPGXLogEntry>& Entries, const FPGXLogSession& Session);

	/**
	 * EN: Deserialize a PGX JSON log file back to entries and session metadata.
	 *     Used by the editor Log Viewer's Import feature to load shipping/runtime logs.
	 *     Returns false if the JSON is invalid or not a PGX log export.
	 * ES: Deserializar un archivo JSON de log PGX a entradas y metadatos de sesion.
	 *     Usado por la funcion Import del Log Viewer del editor para cargar logs de shipping/runtime.
	 *     Retorna false si el JSON es invalido o no es una exportacion de log PGX.
	 */
	static bool FromJson(const FString& JsonString, TArray<FPGXLogEntry>& OutEntries, FPGXLogSession& OutSession);

	/**
	 * EN: Get the verbosity name as string for JSON export.
	 * ES: Obtener el nombre de verbosity como string para exportacion JSON.
	 */
	static const TCHAR* VerbosityToString(EPGXLogVerbosity Verbosity);

	/**
	 * EN: Parse a verbosity string back to enum (for JSON import).
	 * ES: Parsear un string de verbosity de vuelta al enum (para importacion JSON).
	 */
	static EPGXLogVerbosity StringToVerbosity(const FString& Str);

	/**
	 * EN: Get the data type name as string for JSON export.
	 * ES: Obtener el nombre del tipo de dato como string para exportacion JSON.
	 */
	static const TCHAR* DataTypeToString(EPGXLogDataType DataType);

	/**
	 * EN: Parse a data type string back to enum (for JSON import).
	 * ES: Parsear un string de tipo de dato de vuelta al enum (para importacion JSON).
	 */
	static EPGXLogDataType StringToDataType(const FString& Str);
};
