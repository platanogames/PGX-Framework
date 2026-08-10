// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Trace/PGXScopedTrace.h"
#include "Trace/PGXTraceHelper.h"
#include "Trace/PGXTraceTags.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "HAL/PlatformTime.h"

FPGXScopedTrace::FPGXScopedTrace(FName InCategory, FGameplayTag InSystemTag,
	const TCHAR* InOpName, const char* InFunctionName)
	: LogCategory(InCategory)
	, System(InSystemTag)
	, OperationName(InOpName)
	, Function(ANSI_TO_TCHAR(InFunctionName))
	, StartTime(FPlatformTime::Seconds())
	, bActive(false)
{
	// EN: Gate 1: Is the log subsystem available?
	// ES: Gate 1: Esta disponible el subsistema de log?
	if (!FPGXTraceHelper::IsTracingAvailable())
	{
		return;
	}

	// EN: Gate 2: Is tracing enabled for this system (Config DA toggle)?
	// ES: Gate 2: Esta habilitado el trazado para este sistema (toggle del Config DA)?
	if (!FPGXTraceHelper::IsSystemTraceEnabled(InSystemTag))
	{
		return;
	}

	bActive = true;

	// EN: Log ENTER with system tag + function name
	// ES: Loguear ENTER con tag de sistema + nombre de funcion
	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (LogSub)
	{
		FPGXLogEntry Entry;
		Entry.Message = FString::Printf(TEXT("ENTER %s"), *OperationName);
		Entry.Category = LogCategory;
		Entry.Verbosity = FPGXTraceHelper::GetSystemTraceVerbosity(InSystemTag);
		Entry.Timestamp = FDateTime::UtcNow();
		Entry.FrameNumber = GFrameCounter;

		// EN: Add function name as param / ES: Agregar nombre de funcion como param
		FPGXLogParam FuncParam;
		FuncParam.Key = TEXT("Function");
		FuncParam.Value = Function;
		FuncParam.DataType = EPGXLogDataType::String;
		Entry.Params.Add(MoveTemp(FuncParam));

		// EN: Build tag container: System + Enter state
		// ES: Construir contenedor de tags: Sistema + Estado Enter
		Entry.Tags.AddTag(InSystemTag);
		Entry.Tags.AddTag(TAG_PGX_Trace_State_Enter);

		LogSub->AddEntry(MoveTemp(Entry));
	}
}

FPGXScopedTrace::~FPGXScopedTrace()
{
	if (!bActive)
	{
		return;
	}

	const double ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;

	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub)
	{
		return;
	}

	FPGXLogEntry Entry;
	Entry.Category = LogCategory;
	Entry.Verbosity = FPGXTraceHelper::GetSystemTraceVerbosity(System);
	Entry.Timestamp = FDateTime::UtcNow();
	Entry.FrameNumber = GFrameCounter;

	// EN: Build EXIT message with result
	// ES: Construir mensaje EXIT con resultado
	if (bResultSet)
	{
		if (bSuccess)
		{
			Entry.Message = FString::Printf(TEXT("EXIT  %s | SUCCESS | %.2fms"), *OperationName, ElapsedMs);
			Entry.Tags.AddTag(TAG_PGX_Trace_State_Success);
		}
		else
		{
			Entry.Message = ErrorReason.IsEmpty()
				? FString::Printf(TEXT("EXIT  %s | ERROR | %.2fms"), *OperationName, ElapsedMs)
				: FString::Printf(TEXT("EXIT  %s | ERROR: %s | %.2fms"), *OperationName, *ErrorReason, ElapsedMs);
			Entry.Tags.AddTag(TAG_PGX_Trace_State_Error);
		}
	}
	else
	{
		// EN: No result set — scope exited without calling SetSuccess/SetError
		// ES: Sin resultado — el scope termino sin llamar SetSuccess/SetError
		Entry.Message = FString::Printf(TEXT("EXIT  %s | NoResult | %.2fms"), *OperationName, ElapsedMs);
	}

	// EN: System tag + Exit state
	// ES: Tag de sistema + Estado Exit
	Entry.Tags.AddTag(System);
	Entry.Tags.AddTag(TAG_PGX_Trace_State_Exit);

	// EN: Add function name / ES: Agregar nombre de funcion
	{
		FPGXLogParam FuncParam;
		FuncParam.Key = TEXT("Function");
		FuncParam.Value = Function;
		FuncParam.DataType = EPGXLogDataType::String;
		Entry.Params.Add(MoveTemp(FuncParam));
	}

	// EN: Add elapsed time as structured param / ES: Agregar tiempo transcurrido como param estructurado
	{
		FPGXLogParam ElapsedParam;
		ElapsedParam.Key = TEXT("ElapsedMs");
		ElapsedParam.Value = FString::Printf(TEXT("%.2f"), ElapsedMs);
		ElapsedParam.DataType = EPGXLogDataType::Float;
		Entry.Params.Add(MoveTemp(ElapsedParam));
	}

	// EN: Add any extra params from AddParam calls / ES: Agregar params extra de llamadas AddParam
	Entry.Params.Append(ExtraParams);

	LogSub->AddEntry(MoveTemp(Entry));
}

void FPGXScopedTrace::SetSuccess()
{
	bResultSet = true;
	bSuccess = true;
}

void FPGXScopedTrace::SetError(const FString& Reason)
{
	bResultSet = true;
	bSuccess = false;
	ErrorReason = Reason;
}

void FPGXScopedTrace::AddParam(const FString& Key, const FString& Value)
{
	if (!bActive)
	{
		return;
	}

	FPGXLogParam Param;
	Param.Key = Key;
	Param.Value = Value;
	Param.DataType = EPGXLogDataType::String;
	ExtraParams.Add(MoveTemp(Param));
}
