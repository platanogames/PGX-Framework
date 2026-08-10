// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Export/PGXRegistryReportExporter.h"
#include "PGXRegistryEditorModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

FString FPGXRegistryReportExporter::ExportToJSON(const TArray<FPGXRegistryValidationIssue>& Issues,
	const FString& OptionalFileName)
{
	const FString Dir = GetExportDirectory();
	const FString FileName = OptionalFileName.IsEmpty()
		? FString::Printf(TEXT("validation_report_%s.json"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")))
		: OptionalFileName;
	const FString FullPath = FPaths::Combine(Dir, FileName);

	const FString JSONString = BuildJSONString(Issues);

	if (FFileHelper::SaveStringToFile(JSONString, *FullPath))
	{
		UE_LOG(LogPGXRegistryEditor, Log, TEXT("PGXRegistryReportExporter: JSON report saved to [%s]"), *FullPath);
		return FullPath;
	}

	UE_LOG(LogPGXRegistryEditor, Warning, TEXT("PGXRegistryReportExporter: Failed to save JSON report to [%s]"), *FullPath);
	return FString();
}

FString FPGXRegistryReportExporter::ExportToCSV(const TArray<FPGXRegistryValidationIssue>& Issues,
	const FString& OptionalFileName)
{
	const FString Dir = GetExportDirectory();
	const FString FileName = OptionalFileName.IsEmpty()
		? FString::Printf(TEXT("validation_report_%s.csv"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")))
		: OptionalFileName;
	const FString FullPath = FPaths::Combine(Dir, FileName);

	// EN: Build CSV content / ES: Construir contenido CSV
	FString CSV;
	CSV += TEXT("RuleId,Severity,Table,Row,Key,AssetPath,AutoFixable,Message\n");

	for (const FPGXRegistryValidationIssue& Issue : Issues)
	{
		// EN: Escape commas in message / ES: Escapar comas en mensaje
		FString EscapedMessage = Issue.Message.Replace(TEXT("\""), TEXT("\"\""));

		CSV += FString::Printf(TEXT("%s,%s,%s,%s,%s,%s,%s,\"%s\"\n"),
			*Issue.GetRuleIdString(),
			*Issue.GetSeverityString(),
			*Issue.TableName,
			*Issue.RowName.ToString(),
			*Issue.KeyContext,
			*Issue.AssetPath,
			Issue.bAutoFixable ? TEXT("Yes") : TEXT("No"),
			*EscapedMessage);
	}

	if (FFileHelper::SaveStringToFile(CSV, *FullPath))
	{
		UE_LOG(LogPGXRegistryEditor, Log, TEXT("PGXRegistryReportExporter: CSV report saved to [%s]"), *FullPath);
		return FullPath;
	}

	UE_LOG(LogPGXRegistryEditor, Warning, TEXT("PGXRegistryReportExporter: Failed to save CSV report to [%s]"), *FullPath);
	return FString();
}

FString FPGXRegistryReportExporter::GetExportDirectory()
{
	const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("PGXRegistry"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Dir))
	{
		PlatformFile.CreateDirectoryTree(*Dir);
	}
	return Dir;
}

FString FPGXRegistryReportExporter::BuildJSONString(const TArray<FPGXRegistryValidationIssue>& Issues)
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();

	// EN: Summary / ES: Resumen
	Root->SetStringField(TEXT("timestamp"), FDateTime::UtcNow().ToIso8601());
	Root->SetNumberField(TEXT("totalIssues"), Issues.Num());

	int32 ErrorCount = 0;
	int32 WarningCount = 0;
	for (const FPGXRegistryValidationIssue& Issue : Issues)
	{
		if (Issue.Severity == EPGXRegistryValidationSeverity::Error) ++ErrorCount;
		else if (Issue.Severity == EPGXRegistryValidationSeverity::Warning) ++WarningCount;
	}
	Root->SetNumberField(TEXT("errorCount"), ErrorCount);
	Root->SetNumberField(TEXT("warningCount"), WarningCount);

	// EN: Issues array / ES: Array de problemas
	TArray<TSharedPtr<FJsonValue>> IssuesArray;
	for (const FPGXRegistryValidationIssue& Issue : Issues)
	{
		TSharedRef<FJsonObject> IssueObj = MakeShared<FJsonObject>();
		IssueObj->SetStringField(TEXT("ruleId"), Issue.GetRuleIdString());
		IssueObj->SetStringField(TEXT("severity"), Issue.GetSeverityString());
		IssueObj->SetStringField(TEXT("table"), Issue.TableName);
		IssueObj->SetStringField(TEXT("row"), Issue.RowName.ToString());
		IssueObj->SetStringField(TEXT("key"), Issue.KeyContext);
		IssueObj->SetStringField(TEXT("assetPath"), Issue.AssetPath);
		IssueObj->SetBoolField(TEXT("autoFixable"), Issue.bAutoFixable);
		IssueObj->SetStringField(TEXT("message"), Issue.Message);
		IssuesArray.Add(MakeShared<FJsonValueObject>(IssueObj));
	}
	Root->SetArrayField(TEXT("issues"), IssuesArray);

	// EN: Serialize / ES: Serializar
	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutputString);
	FJsonSerializer::Serialize(Root, Writer);
	return OutputString;
}
