// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Validation/PGXRegistryValidationTypes.h"

/**
 * EN: Export validation results to JSON and CSV files.
 *     Output path: {ProjectSaved}/PGXRegistry/
 *
 * ES: Exportar resultados de validacion a archivos JSON y CSV.
 *     Ruta de salida: {ProjectSaved}/PGXRegistry/
 */
class PGXREGISTRYEDITOR_API FPGXRegistryReportExporter
{
public:
	/**
	 * EN: Export issues to JSON format.
	 * ES: Exportar problemas a formato JSON.
	 * @return Full path to the exported file, or empty string on failure.
	 */
	static FString ExportToJSON(const TArray<FPGXRegistryValidationIssue>& Issues, const FString& OptionalFileName = TEXT(""));

	/**
	 * EN: Export issues to CSV format.
	 * ES: Exportar problemas a formato CSV.
	 * @return Full path to the exported file, or empty string on failure.
	 */
	static FString ExportToCSV(const TArray<FPGXRegistryValidationIssue>& Issues, const FString& OptionalFileName = TEXT(""));

	/**
	 * EN: Get the default export directory (creates if needed).
	 * ES: Obtener el directorio de exportacion por defecto (crea si es necesario).
	 */
	static FString GetExportDirectory();

	/**
	 * EN: Build JSON string from issues (used by commandlet and export).
	 * ES: Construir string JSON desde problemas (usado por commandlet y exportacion).
	 */
	static FString BuildJSONString(const TArray<FPGXRegistryValidationIssue>& Issues);
};
