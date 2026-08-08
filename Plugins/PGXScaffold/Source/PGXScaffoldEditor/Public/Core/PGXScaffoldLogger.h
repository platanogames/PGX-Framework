// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Core/PGXScaffoldTypes.h"

/**
 * EN: Scaffold audit logger. Writes JSON logs to Saved/PGX/Scaffold/Logs/.
 *     One file per execution session.
 *
 * ES: Logger de auditoria de scaffold. Escribe logs JSON en Saved/PGX/Scaffold/Logs/.
 *     Un archivo por sesion de ejecucion.
 */
class PGXSCAFFOLDEDITOR_API FPGXScaffoldLogger
{
public:
	/** EN: Begin a logging session / ES: Iniciar una sesion de logging */
	void BeginSession(const FPGXScaffoldBuildPlan& Plan);

	/** EN: Log a step result / ES: Loggear un resultado de paso */
	void LogStep(const FPGXScaffoldLogEntry& Entry);

	/** EN: End session and flush to disk / ES: Terminar sesion y escribir a disco */
	FString EndSession(const FPGXScaffoldExecutionResult& Result);

	/** EN: Get the log directory / ES: Obtener el directorio de logs */
	static FString GetLogDirectory();

private:
	TArray<FPGXScaffoldLogEntry> SessionEntries;
	FGuid SessionPlanId;
	FName SessionTemplateId;
	FDateTime SessionStartTime;
};
