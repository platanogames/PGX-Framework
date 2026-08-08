// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/PGXLogBlueprintLibrary.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Logging/PGXLogTagHelper.h"
#include "Logging/PGXLogCategories.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

// ═══════════════════════════════════════════════════════════════
// EN: Blueprint Function Library implementation for PGX Log writing.
//     All public functions delegate to SubmitEntry() which handles:
//     1. UE_LOG bridge (for UE Output Log visibility)
//     2. FPGXLogEntry construction with auto-tagging
//     3. Submission to UPGXLogSubsystem ring buffer
//
// ES: Implementacion de la Blueprint Function Library para escritura de PGX Log.
//     Todas las funciones publicas delegan a SubmitEntry() que maneja:
//     1. Puente UE_LOG (para visibilidad en UE Output Log)
//     2. Construccion de FPGXLogEntry con auto-tagging
//     3. Envio al ring buffer de UPGXLogSubsystem
// ═══════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────
// Generic / Generico
// ─────────────────────────────────────────────────────────────

void UPGXLogBlueprintLibrary::PGXLog(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, FName Category)
{
	SubmitEntry(WorldContextObject, Verbosity, Message, Category);
}

// ─────────────────────────────────────────────────────────────
// Convenience / Conveniencia
// ─────────────────────────────────────────────────────────────

void UPGXLogBlueprintLibrary::PGXLogInfo(const UObject* WorldContextObject, const FString& Message, FName Category)
{
	SubmitEntry(WorldContextObject, EPGXLogVerbosity::Info, Message, Category);
}

void UPGXLogBlueprintLibrary::PGXLogWarning(const UObject* WorldContextObject, const FString& Message, FName Category)
{
	SubmitEntry(WorldContextObject, EPGXLogVerbosity::Warning, Message, Category);
}

void UPGXLogBlueprintLibrary::PGXLogError(const UObject* WorldContextObject, const FString& Message, FName Category)
{
	SubmitEntry(WorldContextObject, EPGXLogVerbosity::Error, Message, Category);
}

void UPGXLogBlueprintLibrary::PGXLogDebug(const UObject* WorldContextObject, const FString& Message, FName Category)
{
#if !UE_BUILD_SHIPPING
	SubmitEntry(WorldContextObject, EPGXLogVerbosity::Debug, Message, Category);
#endif
}

void UPGXLogBlueprintLibrary::PGXLogVerbose(const UObject* WorldContextObject, const FString& Message, FName Category)
{
#if !UE_BUILD_SHIPPING
	SubmitEntry(WorldContextObject, EPGXLogVerbosity::Verbose, Message, Category);
#endif
}

// ─────────────────────────────────────────────────────────────
// With Parameters / Con Parametros
// ─────────────────────────────────────────────────────────────

void UPGXLogBlueprintLibrary::PGXLogWithParams(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, const TMap<FString, FString>& Params, FName Category)
{
	SubmitEntry(WorldContextObject, Verbosity, Message, Category, &Params);
}

// ─────────────────────────────────────────────────────────────
// Full / Completo
// ─────────────────────────────────────────────────────────────

void UPGXLogBlueprintLibrary::PGXLogFull(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, const TMap<FString, FString>& Params, FGameplayTagContainer Tags, FName Category)
{
	SubmitEntry(WorldContextObject, Verbosity, Message, Category, &Params, &Tags);
}

// ─────────────────────────────────────────────────────────────
// Domain-Aware (v3.0) / Con Dominio (v3.0)
// ─────────────────────────────────────────────────────────────

void UPGXLogBlueprintLibrary::PGXLogDomain(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, FGameplayTag DomainTag, FName Category)
{
	SubmitEntry(WorldContextObject, Verbosity, Message, Category, nullptr, nullptr, &DomainTag);
}

void UPGXLogBlueprintLibrary::PGXLogDomainWithParams(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, FGameplayTag DomainTag, const TMap<FString, FString>& Params, FName Category)
{
	SubmitEntry(WorldContextObject, Verbosity, Message, Category, &Params, nullptr, &DomainTag);
}

TArray<FGameplayTag> UPGXLogBlueprintLibrary::GetRegisteredDomains(const UObject* /*WorldContextObject*/)
{
	TArray<FGameplayTag> Result;
	if (UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached())
	{
		LogSub->GetAllDomains().GetKeys(Result);
	}
	return Result;
}

// ─────────────────────────────────────────────────────────────
// Query / Consulta
// ─────────────────────────────────────────────────────────────

TArray<FPGXLogEntry> UPGXLogBlueprintLibrary::GetCurrentSessionEntries(const UObject* /*WorldContextObject*/)
{
	if (UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached())
	{
		return LogSub->GetCurrentSessionEntries();
	}
	return {};
}

FPGXLogSession UPGXLogBlueprintLibrary::GetCurrentSession(const UObject* /*WorldContextObject*/)
{
	if (UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached())
	{
		return LogSub->GetCurrentSession();
	}
	return {};
}

int32 UPGXLogBlueprintLibrary::GetEntryCount(const UObject* /*WorldContextObject*/)
{
	if (UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached())
	{
		return LogSub->GetEntryCount();
	}
	return 0;
}

// ─────────────────────────────────────────────────────────────
// Internal / Interno
// ─────────────────────────────────────────────────────────────

void UPGXLogBlueprintLibrary::SubmitEntry(const UObject* WorldContextObject, EPGXLogVerbosity Verbosity, const FString& Message, FName Category, const TMap<FString, FString>* Params, const FGameplayTagContainer* CustomTags, const FGameplayTag* InDomainTag)
{
	// EN: Step 1: Bridge to UE_LOG so message appears in Output Log
	// ES: Paso 1: Puente a UE_LOG para que el mensaje aparezca en Output Log
	switch (Verbosity)
	{
	case EPGXLogVerbosity::Verbose:
		UE_LOG(LogPGX, Verbose, TEXT("[BP][%s] %s"), *Category.ToString(), *Message);
		break;
	case EPGXLogVerbosity::Debug:
		UE_LOG(LogPGX, Log, TEXT("[BP][%s] %s"), *Category.ToString(), *Message);
		break;
	case EPGXLogVerbosity::Info:
		UE_LOG(LogPGX, Log, TEXT("[BP][%s] %s"), *Category.ToString(), *Message);
		break;
	case EPGXLogVerbosity::Warning:
		UE_LOG(LogPGX, Warning, TEXT("[BP][%s] %s"), *Category.ToString(), *Message);
		break;
	case EPGXLogVerbosity::Error:
	case EPGXLogVerbosity::Fatal:
		UE_LOG(LogPGX, Error, TEXT("[BP][%s] %s"), *Category.ToString(), *Message);
		break;
	}

	// EN: Step 2: Find the subsystem (try cached first, then WorldContext fallback)
	// ES: Paso 2: Encontrar el subsistema (intentar cache primero, luego fallback WorldContext)
	UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached();
	if (!LogSub && WorldContextObject)
	{
		if (const UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				LogSub = GI->GetSubsystem<UPGXLogSubsystem>();
			}
		}
	}

	if (!LogSub)
	{
		return;
	}

	// EN: Step 3: Build the log entry
	// ES: Paso 3: Construir la entrada de log
	FPGXLogEntry Entry;
	Entry.Message = Message;
	Entry.Category = Category;
	Entry.Verbosity = Verbosity;
	Entry.Timestamp = FDateTime::UtcNow();
	Entry.FrameNumber = GFrameCounter;

	// EN: Convert TMap params to TArray<FPGXLogParam> (all typed as String from Blueprint)
	// ES: Convertir TMap params a TArray<FPGXLogParam> (todos tipados como String desde Blueprint)
	if (Params)
	{
		Entry.Params.Reserve(Params->Num());
		for (const auto& Pair : *Params)
		{
			FPGXLogParam Param;
			Param.Key = Pair.Key;
			Param.Value = Pair.Value;
			Param.DataType = EPGXLogDataType::String;
			Entry.Params.Add(MoveTemp(Param));
		}
	}

	// EN: Step 4: Set domain tag if provided
	// ES: Paso 4: Establecer tag de dominio si se provee
	if (InDomainTag && InDomainTag->IsValid())
	{
		Entry.DomainTag = *InDomainTag;
	}

	// EN: Step 5: Auto-tag (verbosity + context) then append custom tags
	// ES: Paso 5: Auto-tag (verbosity + contexto) luego agregar tags custom
	FPGXLogTagHelper::AutoPopulateTags(Entry);
	if (CustomTags)
	{
		Entry.Tags.AppendTags(*CustomTags);
	}

	// EN: Step 6: Submit to subsystem (filter → buffer → broadcast)
	// ES: Paso 6: Enviar al subsistema (filtro → buffer → broadcast)
	LogSub->AddEntry(MoveTemp(Entry));
}
