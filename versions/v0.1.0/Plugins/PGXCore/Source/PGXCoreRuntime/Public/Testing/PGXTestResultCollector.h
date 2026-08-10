// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

/**
 * EN: Result of a single test execution.
 * ES: Resultado de una ejecucion individual de test.
 */
struct PGXCORERUNTIME_API FPGXTestResult
{
	FString SystemName;
	FString SystemVersion;
	FString TestName;
	bool bPassed = false;
	TArray<FString> Issues;
	FDateTime Timestamp;
	double DurationMs = 0.0;
};

/**
 * EN: Summary of all test results for a single system.
 * ES: Resumen de todos los resultados de test para un solo sistema.
 */
struct PGXCORERUNTIME_API FPGXSystemTestSummary
{
	FString SystemName;
	FString SystemVersion;
	TArray<FPGXTestResult> Results;
	int32 PassedCount = 0;
	int32 FailedCount = 0;
	FDateTime LastRunTime;
};

/** EN: Language for Markdown report export / ES: Idioma para exportacion de informe Markdown */
enum class EPGXReportLanguage : uint8
{
	English,
	Spanish
};

DECLARE_MULTICAST_DELEGATE(FPGXOnTestResultsChanged);

/**
 * EN: Static singleton that collects test results from all PGX systems.
 *     Accessible from both runtime (TestUtilities) and editor (Test Dashboard).
 *     Thread-safe via critical section for delegate broadcasts.
 *
 * ES: Singleton estatico que recolecta resultados de test de todos los sistemas PGX.
 *     Accesible desde runtime (TestUtilities) y editor (Test Dashboard).
 *     Thread-safe via critical section para broadcasts de delegados.
 */
class PGXCORERUNTIME_API FPGXTestResultCollector
{
public:
	static FPGXTestResultCollector& Get();

	// EN: Record a test result / ES: Registrar un resultado de test
	void RecordResult(const FPGXTestResult& Result);

	// EN: Clear all results / ES: Limpiar todos los resultados
	void ClearAll();

	// EN: Clear results for a specific system / ES: Limpiar resultados de un sistema especifico
	void ClearSystem(const FString& SystemName);

	// EN: Get summaries grouped by system / ES: Obtener resumenes agrupados por sistema
	TArray<FPGXSystemTestSummary> GetSystemSummaries() const;

	// EN: Aggregate counts / ES: Conteos agregados
	int32 GetTotalPassed() const;
	int32 GetTotalFailed() const;
	int32 GetTotalTests() const;

	// EN: Export all results to JSON string / ES: Exportar todos los resultados a string JSON
	FString ExportToJson() const;

	// EN: Export results as detailed Markdown report / ES: Exportar resultados como informe MD detallado
	FString ExportToMarkdown(EPGXReportLanguage Language = EPGXReportLanguage::English) const;

	// EN: Delegate broadcast when results change / ES: Delegado broadcast cuando cambian los resultados
	FPGXOnTestResultsChanged OnResultsChanged;

private:
	TArray<FPGXTestResult> Results;
	mutable FCriticalSection Lock;
};
