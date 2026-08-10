// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Core/PGXScaffoldTypes.h"

/**
 * EN: Transactional scaffold executor. Executes a build plan step-by-step
 *     within an FScopedTransaction for Ctrl+Z rollback.
 *
 * ES: Ejecutor transaccional de scaffold. Ejecuta un plan paso a paso
 *     dentro de un FScopedTransaction para rollback con Ctrl+Z.
 */
class PGXSCAFFOLDEDITOR_API FPGXScaffoldExecutor
{
public:
	/**
	 * EN: Execute the build plan. Returns execution result with detailed log.
	 * ES: Ejecutar el plan de build. Retorna resultado con log detallado.
	 */
	FPGXScaffoldExecutionResult Execute(
		FPGXScaffoldBuildPlan& Plan,
		const FPGXScaffoldProgressDelegate& ProgressCallback = FPGXScaffoldProgressDelegate());

private:
	/** EN: Execute a single step / ES: Ejecutar un paso individual */
	bool ExecuteStep(FPGXScaffoldPlanStep& Step, TArray<FPGXScaffoldLogEntry>& LogEntries);

	bool ExecuteCreateFolder(const FPGXScaffoldPlanStep& Step, TArray<FPGXScaffoldLogEntry>& LogEntries);
	bool ExecuteCreateDataAsset(const FPGXScaffoldPlanStep& Step, TArray<FPGXScaffoldLogEntry>& LogEntries);
	bool ExecuteCreateBlueprint(const FPGXScaffoldPlanStep& Step, TArray<FPGXScaffoldLogEntry>& LogEntries);

	/** EN: Mark file for source control add (best-effort, silent) / ES: Marcar archivo para add en source control (best-effort, silencioso) */
	void MarkForSourceControl(const FString& FilePath);

	/** EN: Roll back created paths on failure (empty folders only) / ES: Revertir paths creados en fallo (solo carpetas vacias) */
	void RollbackCreatedPaths();

	/** EN: Paths created during execution (for rollback) / ES: Paths creados durante ejecucion (para rollback) */
	TArray<FString> CreatedPaths;
};
