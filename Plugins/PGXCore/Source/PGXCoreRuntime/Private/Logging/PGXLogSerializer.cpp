// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/PGXLogSerializer.h"
#include "PGXCoreRuntime.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "GameplayTagContainer.h"

// EN: JSON serializer for PGX log entries with typed parameters and tags
// ES: Serializador JSON para entradas de log PGX con parametros tipados y tags

FString FPGXLogSerializer::ToJson(const TArray<FPGXLogEntry>& Entries, const FPGXLogSession& Session)
{
	// EN: Build JSON using UE's FJsonObject for proper escaping and formatting
	// ES: Construir JSON usando FJsonObject de UE para escape y formateo correcto
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();

	// EN: Framework metadata / ES: Metadatos del framework
	Root->SetStringField(TEXT("framework"), TEXT("PGX"));
	Root->SetStringField(TEXT("version"), PGXVersion::String);
	Root->SetStringField(TEXT("exportTime"), FDateTime::UtcNow().ToIso8601());

	// EN: Session metadata / ES: Metadatos de sesion
	TSharedRef<FJsonObject> SessionObj = MakeShared<FJsonObject>();
	SessionObj->SetNumberField(TEXT("id"), Session.SessionId);
	SessionObj->SetStringField(TEXT("startTime"), Session.StartTime.ToIso8601());
	SessionObj->SetStringField(TEXT("endTime"), Session.EndTime.ToIso8601());
	SessionObj->SetNumberField(TEXT("totalEntries"), Session.EntryCount);
	SessionObj->SetNumberField(TEXT("warnings"), Session.WarningCount);
	SessionObj->SetNumberField(TEXT("errors"), Session.ErrorCount);
	Root->SetObjectField(TEXT("session"), SessionObj);

	// EN: Entries array / ES: Array de entradas
	TArray<TSharedPtr<FJsonValue>> EntriesArray;
	EntriesArray.Reserve(Entries.Num());

	for (const FPGXLogEntry& Entry : Entries)
	{
		TSharedRef<FJsonObject> EntryObj = MakeShared<FJsonObject>();
		EntryObj->SetStringField(TEXT("timestamp"), Entry.Timestamp.ToIso8601());
		EntryObj->SetNumberField(TEXT("frame"), static_cast<double>(Entry.FrameNumber));
		EntryObj->SetStringField(TEXT("category"), Entry.Category.ToString());
		EntryObj->SetStringField(TEXT("verbosity"), VerbosityToString(Entry.Verbosity));
		EntryObj->SetStringField(TEXT("message"), Entry.Message);
		EntryObj->SetNumberField(TEXT("sessionId"), Entry.SessionId);

		// EN: Domain tag / ES: Tag de dominio
		if (Entry.DomainTag.IsValid())
		{
			EntryObj->SetStringField(TEXT("domainTag"), Entry.DomainTag.ToString());
		}

		// EN: Typed parameters / ES: Parametros tipados
		if (Entry.Params.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> ParamsArray;
			ParamsArray.Reserve(Entry.Params.Num());

			for (const FPGXLogParam& Param : Entry.Params)
			{
				TSharedRef<FJsonObject> ParamObj = MakeShared<FJsonObject>();
				ParamObj->SetStringField(TEXT("key"), Param.Key);
				ParamObj->SetStringField(TEXT("value"), Param.Value);
				ParamObj->SetStringField(TEXT("type"), DataTypeToString(Param.DataType));
				ParamsArray.Add(MakeShared<FJsonValueObject>(ParamObj));
			}

			EntryObj->SetArrayField(TEXT("params"), ParamsArray);
		}

		// EN: GameplayTags / ES: GameplayTags
		if (!Entry.Tags.IsEmpty())
		{
			TArray<TSharedPtr<FJsonValue>> TagsArray;
			TArray<FGameplayTag> TagList;
			Entry.Tags.GetGameplayTagArray(TagList);
			for (const FGameplayTag& Tag : TagList)
			{
				TagsArray.Add(MakeShared<FJsonValueString>(Tag.ToString()));
			}
			EntryObj->SetArrayField(TEXT("tags"), TagsArray);
		}

		EntriesArray.Add(MakeShared<FJsonValueObject>(EntryObj));
	}

	Root->SetArrayField(TEXT("entries"), EntriesArray);

	// EN: Serialize to string with pretty formatting
	// ES: Serializar a string con formateo bonito
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root, Writer);

	return OutputString;
}

// ═══════════════════════════════════════════════════════════════
// FromJson — Import / Importar
// ═══════════════════════════════════════════════════════════════

bool FPGXLogSerializer::FromJson(const FString& JsonString, TArray<FPGXLogEntry>& OutEntries, FPGXLogSession& OutSession)
{
	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}

	// EN: Validate it's a PGX export / ES: Validar que es una exportacion PGX
	if (Root->GetStringField(TEXT("framework")) != TEXT("PGX"))
	{
		return false;
	}

	// EN: Parse session metadata / ES: Parsear metadatos de sesion
	OutSession = FPGXLogSession();
	if (const TSharedPtr<FJsonObject>* SessionObj; Root->TryGetObjectField(TEXT("session"), SessionObj))
	{
		OutSession.SessionId = static_cast<int32>((*SessionObj)->GetNumberField(TEXT("id")));
		FDateTime::ParseIso8601(*(*SessionObj)->GetStringField(TEXT("startTime")), OutSession.StartTime);
		FDateTime::ParseIso8601(*(*SessionObj)->GetStringField(TEXT("endTime")), OutSession.EndTime);
		OutSession.EntryCount = static_cast<int32>((*SessionObj)->GetNumberField(TEXT("totalEntries")));
		OutSession.WarningCount = static_cast<int32>((*SessionObj)->GetNumberField(TEXT("warnings")));
		OutSession.ErrorCount = static_cast<int32>((*SessionObj)->GetNumberField(TEXT("errors")));
	}

	// EN: Parse entries array / ES: Parsear array de entradas
	const TArray<TSharedPtr<FJsonValue>>* EntriesArray;
	if (!Root->TryGetArrayField(TEXT("entries"), EntriesArray))
	{
		return false;
	}

	OutEntries.Reset();
	OutEntries.Reserve(EntriesArray->Num());

	for (const TSharedPtr<FJsonValue>& EntryVal : *EntriesArray)
	{
		const TSharedPtr<FJsonObject>& EntryObj = EntryVal->AsObject();
		if (!EntryObj.IsValid())
		{
			continue;
		}

		FPGXLogEntry Entry;
		FDateTime::ParseIso8601(*EntryObj->GetStringField(TEXT("timestamp")), Entry.Timestamp);
		Entry.FrameNumber = static_cast<uint64>(EntryObj->GetNumberField(TEXT("frame")));
		Entry.Category = FName(*EntryObj->GetStringField(TEXT("category")));
		Entry.Verbosity = StringToVerbosity(EntryObj->GetStringField(TEXT("verbosity")));
		Entry.Message = EntryObj->GetStringField(TEXT("message"));
		Entry.SessionId = static_cast<int32>(EntryObj->GetNumberField(TEXT("sessionId")));

		// EN: Parse domain tag / ES: Parsear tag de dominio
		FString DomainTagStr;
		if (EntryObj->TryGetStringField(TEXT("domainTag"), DomainTagStr) && !DomainTagStr.IsEmpty())
		{
			Entry.DomainTag = FGameplayTag::RequestGameplayTag(FName(*DomainTagStr), false);
		}

		// EN: Parse params / ES: Parsear parametros
		const TArray<TSharedPtr<FJsonValue>>* ParamsArray;
		if (EntryObj->TryGetArrayField(TEXT("params"), ParamsArray))
		{
			Entry.Params.Reserve(ParamsArray->Num());
			for (const TSharedPtr<FJsonValue>& ParamVal : *ParamsArray)
			{
				const TSharedPtr<FJsonObject>& ParamObj = ParamVal->AsObject();
				if (!ParamObj.IsValid())
				{
					continue;
				}

				FPGXLogParam Param;
				Param.Key = ParamObj->GetStringField(TEXT("key"));
				Param.Value = ParamObj->GetStringField(TEXT("value"));
				Param.DataType = StringToDataType(ParamObj->GetStringField(TEXT("type")));
				Entry.Params.Add(MoveTemp(Param));
			}
		}

		// EN: Parse tags / ES: Parsear tags
		const TArray<TSharedPtr<FJsonValue>>* TagsArray;
		if (EntryObj->TryGetArrayField(TEXT("tags"), TagsArray))
		{
			for (const TSharedPtr<FJsonValue>& TagVal : *TagsArray)
			{
				FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagVal->AsString()), false);
				if (Tag.IsValid())
				{
					Entry.Tags.AddTag(Tag);
				}
			}
		}

		OutEntries.Add(MoveTemp(Entry));
	}

	return true;
}

// ═══════════════════════════════════════════════════════════════
// String ↔ Enum Conversions / Conversiones String ↔ Enum
// ═══════════════════════════════════════════════════════════════

const TCHAR* FPGXLogSerializer::VerbosityToString(EPGXLogVerbosity Verbosity)
{
	switch (Verbosity)
	{
	case EPGXLogVerbosity::Verbose: return TEXT("Verbose");
	case EPGXLogVerbosity::Debug:   return TEXT("Debug");
	case EPGXLogVerbosity::Info:    return TEXT("Info");
	case EPGXLogVerbosity::Warning: return TEXT("Warning");
	case EPGXLogVerbosity::Error:   return TEXT("Error");
	case EPGXLogVerbosity::Fatal:   return TEXT("Fatal");
	default:                        return TEXT("Unknown");
	}
}

EPGXLogVerbosity FPGXLogSerializer::StringToVerbosity(const FString& Str)
{
	if (Str == TEXT("Verbose")) return EPGXLogVerbosity::Verbose;
	if (Str == TEXT("Debug"))   return EPGXLogVerbosity::Debug;
	if (Str == TEXT("Info"))    return EPGXLogVerbosity::Info;
	if (Str == TEXT("Warning")) return EPGXLogVerbosity::Warning;
	if (Str == TEXT("Error"))   return EPGXLogVerbosity::Error;
	if (Str == TEXT("Fatal"))   return EPGXLogVerbosity::Fatal;
	return EPGXLogVerbosity::Info;
}

const TCHAR* FPGXLogSerializer::DataTypeToString(EPGXLogDataType DataType)
{
	switch (DataType)
	{
	case EPGXLogDataType::String:      return TEXT("String");
	case EPGXLogDataType::Integer:     return TEXT("Integer");
	case EPGXLogDataType::Float:       return TEXT("Float");
	case EPGXLogDataType::Boolean:     return TEXT("Boolean");
	case EPGXLogDataType::Vector:      return TEXT("Vector");
	case EPGXLogDataType::Rotator:     return TEXT("Rotator");
	case EPGXLogDataType::Transform:   return TEXT("Transform");
	case EPGXLogDataType::GameplayTag: return TEXT("GameplayTag");
	case EPGXLogDataType::Object:      return TEXT("Object");
	case EPGXLogDataType::Color:       return TEXT("Color");
	case EPGXLogDataType::Custom:      return TEXT("Custom");
	default:                           return TEXT("Unknown");
	}
}

EPGXLogDataType FPGXLogSerializer::StringToDataType(const FString& Str)
{
	if (Str == TEXT("String"))      return EPGXLogDataType::String;
	if (Str == TEXT("Integer"))     return EPGXLogDataType::Integer;
	if (Str == TEXT("Float"))       return EPGXLogDataType::Float;
	if (Str == TEXT("Boolean"))     return EPGXLogDataType::Boolean;
	if (Str == TEXT("Vector"))      return EPGXLogDataType::Vector;
	if (Str == TEXT("Rotator"))     return EPGXLogDataType::Rotator;
	if (Str == TEXT("Transform"))   return EPGXLogDataType::Transform;
	if (Str == TEXT("GameplayTag")) return EPGXLogDataType::GameplayTag;
	if (Str == TEXT("Object"))      return EPGXLogDataType::Object;
	if (Str == TEXT("Color"))       return EPGXLogDataType::Color;
	return EPGXLogDataType::Custom;
}
