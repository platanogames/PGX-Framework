// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Trace/PGXTraceHelper.h"
#include "Trace/PGXTraceTags.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "HAL/PlatformTime.h"

// ============================================================================
// EN: Static member initialization
// ES: Inicializacion de miembros estaticos
// ============================================================================

TMap<FGameplayTag, FPGXTraceConfig> FPGXTraceHelper::SystemTraceConfigs;
FCriticalSection FPGXTraceHelper::ConfigLock;
int32 FPGXTraceHelper::NextTraceId = 0;

// ============================================================================
// EN: System Configuration Registry / ES: Registro de Configuracion de Sistema
// ============================================================================

void FPGXTraceHelper::RegisterSystemTraceConfig(FGameplayTag SystemTag, const FPGXTraceConfig& Config)
{
	FScopeLock Lock(&ConfigLock);
	SystemTraceConfigs.Add(SystemTag, Config);
}

void FPGXTraceHelper::UnregisterSystemTraceConfig(FGameplayTag SystemTag)
{
	FScopeLock Lock(&ConfigLock);
	SystemTraceConfigs.Remove(SystemTag);
}

void FPGXTraceHelper::UpdateSystemTraceConfig(FGameplayTag SystemTag, const FPGXTraceConfig& Config)
{
	FScopeLock Lock(&ConfigLock);
	if (SystemTraceConfigs.Contains(SystemTag))
	{
		SystemTraceConfigs[SystemTag] = Config;
	}
	else
	{
		SystemTraceConfigs.Add(SystemTag, Config);
	}
}

// ============================================================================
// EN: Availability Checks / ES: Verificaciones de Disponibilidad
// ============================================================================

bool FPGXTraceHelper::IsTracingAvailable()
{
#if UE_BUILD_SHIPPING
	return false;
#else
	return UPGXLogSubsystem::GetCached() != nullptr;
#endif
}

bool FPGXTraceHelper::IsSystemTraceEnabled(FGameplayTag SystemTag)
{
#if UE_BUILD_SHIPPING
	return false;
#else
	if (!IsTracingAvailable())
	{
		return false;
	}

	FScopeLock Lock(&ConfigLock);
	if (const FPGXTraceConfig* Config = SystemTraceConfigs.Find(SystemTag))
	{
		return Config->bSystemTraceEnabled;
	}

	// EN: System not registered — default to enabled (trace even if config not loaded yet)
	// ES: Sistema no registrado — default a habilitado (trazar aunque config no este cargada aun)
	return true;
#endif
}

EPGXLogVerbosity FPGXTraceHelper::GetSystemTraceVerbosity(FGameplayTag SystemTag)
{
	FScopeLock Lock(&ConfigLock);
	if (const FPGXTraceConfig* Config = SystemTraceConfigs.Find(SystemTag))
	{
		return Config->TraceVerbosity;
	}
	return EPGXLogVerbosity::Debug;
}

// ============================================================================
// EN: Trace Submission / ES: Envio de Trazas
// ============================================================================

void FPGXTraceHelper::LogTracePoint(FName Category, FGameplayTag SystemTag,
	EPGXLogVerbosity Verbosity, const TCHAR* Message, const char* FunctionName)
{
	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub)
	{
		return;
	}

	FPGXLogEntry Entry;
	Entry.Message = Message;
	Entry.Category = Category;
	Entry.Verbosity = Verbosity;
	Entry.Timestamp = FDateTime::UtcNow();
	Entry.FrameNumber = GFrameCounter;

	// EN: Add function name / ES: Agregar nombre de funcion
	FPGXLogParam FuncParam;
	FuncParam.Key = TEXT("Function");
	FuncParam.Value = ANSI_TO_TCHAR(FunctionName);
	FuncParam.DataType = EPGXLogDataType::String;
	Entry.Params.Add(MoveTemp(FuncParam));

	// EN: Add system tag / ES: Agregar tag de sistema
	Entry.Tags.AddTag(SystemTag);

	LogSub->AddEntry(MoveTemp(Entry));
}

// ============================================================================
// EN: Blueprint Trace API / ES: API de Traza Blueprint
// ============================================================================

FPGXTraceHandle FPGXTraceHelper::BeginTrace(FGameplayTag SystemTag, const FString& OperationName,
	const FString& Location, const FString& Description)
{
	FPGXTraceHandle Handle;

	// EN: Check if tracing is possible and enabled for this system
	// ES: Verificar si el trazado es posible y esta habilitado para este sistema
	if (!IsTracingAvailable() || !IsSystemTraceEnabled(SystemTag))
	{
		return Handle; // Invalid handle — EndTrace will be a no-op
	}

	Handle.TraceId = NextTraceId++;
	Handle.StartTime = FPlatformTime::Seconds();
	Handle.SystemTag = SystemTag;
	Handle.OperationName = OperationName;
	Handle.Location = Location;
	Handle.Description = Description;

	// EN: Log ENTER
	// ES: Loguear ENTER
	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (LogSub)
	{
		FPGXLogEntry Entry;
		Entry.Message = FString::Printf(TEXT("ENTER %s"), *OperationName);
		Entry.Category = FName(TEXT("LogPGXTrace"));
		Entry.Verbosity = GetSystemTraceVerbosity(SystemTag);
		Entry.Timestamp = FDateTime::UtcNow();
		Entry.FrameNumber = GFrameCounter;

		// EN: Add location and description as params
		// ES: Agregar ubicacion y descripcion como params
		if (!Location.IsEmpty())
		{
			FPGXLogParam LocationParam;
			LocationParam.Key = TEXT("Location");
			LocationParam.Value = Location;
			LocationParam.DataType = EPGXLogDataType::String;
			Entry.Params.Add(MoveTemp(LocationParam));
		}

		if (!Description.IsEmpty())
		{
			FPGXLogParam DescParam;
			DescParam.Key = TEXT("Description");
			DescParam.Value = Description;
			DescParam.DataType = EPGXLogDataType::String;
			Entry.Params.Add(MoveTemp(DescParam));
		}

		Entry.Tags.AddTag(SystemTag);
		Entry.Tags.AddTag(TAG_PGX_Trace_State_Enter);

		LogSub->AddEntry(MoveTemp(Entry));
	}

	return Handle;
}

void FPGXTraceHelper::EndTrace(const FPGXTraceHandle& Handle, bool bSuccess,
	const FString& ErrorReason)
{
	if (!Handle.IsValid())
	{
		return;
	}

	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub)
	{
		return;
	}

	const double ElapsedMs = (FPlatformTime::Seconds() - Handle.StartTime) * 1000.0;

	FPGXLogEntry Entry;
	Entry.Category = FName(TEXT("LogPGXTrace"));
	Entry.Verbosity = GetSystemTraceVerbosity(Handle.SystemTag);
	Entry.Timestamp = FDateTime::UtcNow();
	Entry.FrameNumber = GFrameCounter;

	if (bSuccess)
	{
		Entry.Message = FString::Printf(TEXT("EXIT  %s | SUCCESS | %.2fms"), *Handle.OperationName, ElapsedMs);
		Entry.Tags.AddTag(TAG_PGX_Trace_State_Success);
	}
	else
	{
		Entry.Message = ErrorReason.IsEmpty()
			? FString::Printf(TEXT("EXIT  %s | ERROR | %.2fms"), *Handle.OperationName, ElapsedMs)
			: FString::Printf(TEXT("EXIT  %s | ERROR: %s | %.2fms"), *Handle.OperationName, *ErrorReason, ElapsedMs);
		Entry.Tags.AddTag(TAG_PGX_Trace_State_Error);
	}

	Entry.Tags.AddTag(Handle.SystemTag);
	Entry.Tags.AddTag(TAG_PGX_Trace_State_Exit);

	// EN: Add location, description, and elapsed as params
	// ES: Agregar ubicacion, descripcion y elapsed como params
	if (!Handle.Location.IsEmpty())
	{
		FPGXLogParam LocationParam;
		LocationParam.Key = TEXT("Location");
		LocationParam.Value = Handle.Location;
		LocationParam.DataType = EPGXLogDataType::String;
		Entry.Params.Add(MoveTemp(LocationParam));
	}

	if (!Handle.Description.IsEmpty())
	{
		FPGXLogParam DescParam;
		DescParam.Key = TEXT("Description");
		DescParam.Value = Handle.Description;
		DescParam.DataType = EPGXLogDataType::String;
		Entry.Params.Add(MoveTemp(DescParam));
	}

	{
		FPGXLogParam ElapsedParam;
		ElapsedParam.Key = TEXT("ElapsedMs");
		ElapsedParam.Value = FString::Printf(TEXT("%.2f"), ElapsedMs);
		ElapsedParam.DataType = EPGXLogDataType::Float;
		Entry.Params.Add(MoveTemp(ElapsedParam));
	}

	LogSub->AddEntry(MoveTemp(Entry));
}

// ============================================================================
// EN: Diagnostics / ES: Diagnosticos
// ============================================================================

int32 FPGXTraceHelper::GetRegisteredSystemCount()
{
	FScopeLock Lock(&ConfigLock);
	return SystemTraceConfigs.Num();
}

void FPGXTraceHelper::ClearAll()
{
	FScopeLock Lock(&ConfigLock);
	SystemTraceConfigs.Empty();
	NextTraceId = 0;
}
