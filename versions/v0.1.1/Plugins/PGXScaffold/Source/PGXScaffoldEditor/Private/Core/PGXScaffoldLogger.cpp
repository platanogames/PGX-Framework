// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Core/PGXScaffoldLogger.h"
#include "PGXScaffoldEditor.h"
#include "Logging/PGXLogMacros.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void FPGXScaffoldLogger::BeginSession(const FPGXScaffoldBuildPlan& Plan)
{
	SessionPlanId = Plan.PlanId;
	SessionTemplateId = Plan.TemplateId;
	SessionStartTime = FDateTime::Now();
	SessionEntries.Empty();

	PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXScaffoldLogger: Session started — Plan '%s', Template '%s'"),
		*SessionPlanId.ToString(), *SessionTemplateId.ToString());
}

void FPGXScaffoldLogger::LogStep(const FPGXScaffoldLogEntry& Entry)
{
	SessionEntries.Add(Entry);
}

FString FPGXScaffoldLogger::EndSession(const FPGXScaffoldExecutionResult& Result)
{
	// EN: Build JSON log
	// ES: Construir log JSON
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("PlanId"), SessionPlanId.ToString());
	Root->SetStringField(TEXT("TemplateId"), SessionTemplateId.ToString());
	Root->SetStringField(TEXT("StartTime"), SessionStartTime.ToString());
	Root->SetStringField(TEXT("EndTime"), FDateTime::Now().ToString());
	Root->SetStringField(TEXT("Status"), Result.Status == EPGXScaffoldOperationStatus::Completed ? TEXT("Completed") : TEXT("Failed"));
	Root->SetNumberField(TEXT("CompletedSteps"), Result.CompletedSteps);
	Root->SetNumberField(TEXT("SkippedSteps"), Result.SkippedSteps);
	Root->SetNumberField(TEXT("FailedSteps"), Result.FailedSteps);
	Root->SetNumberField(TEXT("TotalDurationMs"), Result.TotalDurationMs);

	// EN: Add step entries
	// ES: Agregar entradas de pasos
	TArray<TSharedPtr<FJsonValue>> EntryArray;
	for (const auto& Entry : SessionEntries)
	{
		TSharedRef<FJsonObject> EntryObj = MakeShared<FJsonObject>();
		EntryObj->SetStringField(TEXT("Timestamp"), Entry.Timestamp.ToString());
		EntryObj->SetNumberField(TEXT("StepIndex"), Entry.StepIndex);
		EntryObj->SetStringField(TEXT("Status"), FString::FromInt(static_cast<int32>(Entry.Status)));
		EntryObj->SetStringField(TEXT("Message"), Entry.Message);
		EntryObj->SetNumberField(TEXT("DurationMs"), Entry.DurationMs);
		EntryArray.Add(MakeShared<FJsonValueObject>(EntryObj));
	}
	Root->SetArrayField(TEXT("Steps"), EntryArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(Root, Writer);

	// EN: Write to disk
	// ES: Escribir a disco
	FString LogDir = GetLogDirectory();
	IFileManager::Get().MakeDirectory(*LogDir, true);

	FString FileName = FString::Printf(TEXT("scaffold_%s_%s.json"),
		*SessionStartTime.ToString(TEXT("%Y%m%d_%H%M%S")),
		*SessionTemplateId.ToString());
	FString FilePath = LogDir / FileName;

	if (FFileHelper::SaveStringToFile(JsonString, *FilePath))
	{
		PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXScaffoldLogger: Audit log saved to %s"), *FilePath);
	}
	else
	{
		PGX_LOG_WARNING(LogPGXScaffold, TEXT("FPGXScaffoldLogger: Failed to save audit log to %s"), *FilePath);
	}

	SessionEntries.Empty();
	return FilePath;
}

FString FPGXScaffoldLogger::GetLogDirectory()
{
	return FPaths::ProjectSavedDir() / TEXT("PGX") / TEXT("Scaffold") / TEXT("Logs");
}
