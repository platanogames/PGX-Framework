// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Commandlet/PGXRegistryValidateCommandlet.h"
#include "PGXRegistryEditorModule.h"
#include "Validation/PGXRegistryValidationService.h"
#include "Validation/PGXRegistryValidationTypes.h"
#include "Export/PGXRegistryReportExporter.h"
#include "Misc/CommandLine.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace PGXRegistryValidateCommandlet
{
enum class EExitCode : int32
{
	Success = 0,
	ValidationFailed = 1,
	ReportWriteFailed = 2,
	InternalError = 3
};

struct FOptions
{
	bool bStrict = false;
	bool bExportJSON = true;
	bool bExportCSV = false;
	bool bHelp = false;
	FString JSONReportFileName = TEXT("validation_report.json");
	FString CSVReportFileName = TEXT("validation_report.csv");
};

bool HasSwitch(const TArray<FString>& Switches, const TCHAR* SwitchName)
{
	for (const FString& Switch : Switches)
	{
		if (Switch.Equals(SwitchName, ESearchCase::IgnoreCase))
		{
			return true;
		}
	}
	return false;
}

bool TryGetSwitchValue(const TArray<FString>& Switches, const TCHAR* SwitchName, FString& OutValue)
{
	const FString Prefix = FString::Printf(TEXT("%s="), SwitchName);
	for (const FString& Switch : Switches)
	{
		if (Switch.StartsWith(Prefix, ESearchCase::IgnoreCase))
		{
			OutValue = Switch.RightChop(Prefix.Len()).TrimStartAndEnd();
			return !OutValue.IsEmpty();
		}
	}
	return false;
}

FOptions ParseOptions(const FString& Params)
{
	TArray<FString> Tokens;
	TArray<FString> Switches;
	FCommandLine::Parse(*Params, Tokens, Switches);

	FOptions Options;
	Options.bStrict = HasSwitch(Switches, TEXT("strict"));
	Options.bExportJSON = !HasSwitch(Switches, TEXT("noexport"));
	Options.bExportCSV = HasSwitch(Switches, TEXT("csv"));
	Options.bHelp = HasSwitch(Switches, TEXT("help")) || HasSwitch(Switches, TEXT("?"));

	FString ReportName;
	if (TryGetSwitchValue(Switches, TEXT("report"), ReportName))
	{
		Options.JSONReportFileName = ReportName;
	}

	FString CSVReportName;
	if (TryGetSwitchValue(Switches, TEXT("csvreport"), CSVReportName))
	{
		Options.CSVReportFileName = CSVReportName;
		Options.bExportCSV = true;
	}

	return Options;
}

void PrintUsage()
{
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("Usage: UnrealEditor-Cmd.exe <ProjectPath> -run=PGXRegistryValidate [-strict] [-csv] [-noexport] [-report=<file>] [-csvreport=<file>]"));
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("Exit codes: 0=success, 1=validation failed, 2=report export failed, 3=internal error"));
}

void LogIssue(const FPGXRegistryValidationIssue& Issue)
{
	switch (Issue.Severity)
	{
	case EPGXRegistryValidationSeverity::Error:
		UE_LOG(LogPGXRegistryEditor, Error, TEXT("  [%s] %s | %s | Row:%s | %s"),
			*Issue.GetSeverityString(),
			*Issue.GetRuleIdString(),
			*Issue.TableName,
			*Issue.RowName.ToString(),
			*Issue.Message);
		break;
	case EPGXRegistryValidationSeverity::Warning:
		UE_LOG(LogPGXRegistryEditor, Warning, TEXT("  [%s] %s | %s | Row:%s | %s"),
			*Issue.GetSeverityString(),
			*Issue.GetRuleIdString(),
			*Issue.TableName,
			*Issue.RowName.ToString(),
			*Issue.Message);
		break;
	default:
		UE_LOG(LogPGXRegistryEditor, Display, TEXT("  [%s] %s | %s | Row:%s | %s"),
			*Issue.GetSeverityString(),
			*Issue.GetRuleIdString(),
			*Issue.TableName,
			*Issue.RowName.ToString(),
			*Issue.Message);
		break;
	}
}

FString BuildMachineSummaryJSON(
	const FOptions& Options,
	int32 TotalIssues,
	int32 ErrorCount,
	int32 WarningCount,
	int32 InfoCount,
	const FString& JSONReportPath,
	const FString& CSVReportPath,
	EExitCode ExitCode)
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("commandlet"), TEXT("PGXRegistryValidate"));
	Root->SetStringField(TEXT("status"), ExitCode == EExitCode::Success ? TEXT("PASS") : TEXT("FAIL"));
	Root->SetNumberField(TEXT("exitCode"), static_cast<int32>(ExitCode));
	Root->SetBoolField(TEXT("strict"), Options.bStrict);
	Root->SetNumberField(TEXT("totalIssues"), TotalIssues);
	Root->SetNumberField(TEXT("errorCount"), ErrorCount);
	Root->SetNumberField(TEXT("warningCount"), WarningCount);
	Root->SetNumberField(TEXT("infoCount"), InfoCount);
	Root->SetStringField(TEXT("jsonReportPath"), JSONReportPath);
	Root->SetStringField(TEXT("csvReportPath"), CSVReportPath);

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(Root, Writer);
	return Output;
}
} // namespace PGXRegistryValidateCommandlet

UPGXRegistryValidateCommandlet::UPGXRegistryValidateCommandlet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LogToConsole = true;
	IsClient = false;
	IsServer = false;
	IsEditor = true;
}

int32 UPGXRegistryValidateCommandlet::Main(const FString& Params)
{
	using namespace PGXRegistryValidateCommandlet;

	UE_LOG(LogPGXRegistryEditor, Display, TEXT("============================================"));
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("PGX Registry Validation Commandlet"));
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("============================================"));

	const FOptions Options = ParseOptions(Params);
	if (Options.bHelp)
	{
		PrintUsage();
		return static_cast<int32>(EExitCode::Success);
	}

	if (Options.bStrict)
	{
		UE_LOG(LogPGXRegistryEditor, Display, TEXT("Mode: STRICT (warnings treated as errors)"));
	}
	if (!Options.bExportJSON)
	{
		UE_LOG(LogPGXRegistryEditor, Display, TEXT("JSON report export disabled by -noexport"));
	}

	// EN: Run full validation / ES: Ejecutar validacion completa
	FPGXRegistryValidationService Validator;
	TArray<FPGXRegistryValidationIssue> Issues;
	const int32 TotalIssues = Validator.ValidateAll(Issues);

	const int32 ErrorCount = FPGXRegistryValidationService::CountErrors(Issues);
	const int32 WarningCount = FPGXRegistryValidationService::CountWarnings(Issues);
	const int32 InfoCount = TotalIssues - ErrorCount - WarningCount;

	// EN: Print summary to stdout / ES: Imprimir resumen a stdout
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("--------------------------------------------"));
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("Validation Summary:"));
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("  Total issues:  %d"), TotalIssues);
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("  Errors:        %d"), ErrorCount);
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("  Warnings:      %d"), WarningCount);
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("  Info:          %d"), InfoCount);
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("--------------------------------------------"));

	// EN: Print each issue / ES: Imprimir cada issue
	for (const FPGXRegistryValidationIssue& Issue : Issues)
	{
		LogIssue(Issue);
	}

	// EN: Export JSON report / ES: Exportar reporte JSON
	FString JSONReportPath;
	FString CSVReportPath;
	bool bReportWriteFailed = false;
	if (Options.bExportJSON)
	{
		JSONReportPath = FPGXRegistryReportExporter::ExportToJSON(Issues, Options.JSONReportFileName);
		if (!JSONReportPath.IsEmpty())
		{
			UE_LOG(LogPGXRegistryEditor, Display, TEXT("JSON report: %s"), *JSONReportPath);
		}
		else
		{
			UE_LOG(LogPGXRegistryEditor, Error, TEXT("Failed to write JSON report [%s]"), *Options.JSONReportFileName);
			bReportWriteFailed = true;
		}
	}

	if (Options.bExportCSV)
	{
		CSVReportPath = FPGXRegistryReportExporter::ExportToCSV(Issues, Options.CSVReportFileName);
		if (!CSVReportPath.IsEmpty())
		{
			UE_LOG(LogPGXRegistryEditor, Display, TEXT("CSV report: %s"), *CSVReportPath);
		}
		else
		{
			UE_LOG(LogPGXRegistryEditor, Error, TEXT("Failed to write CSV report [%s]"), *Options.CSVReportFileName);
			bReportWriteFailed = true;
		}
	}

	// EN: Determine exit code / ES: Determinar codigo de salida
	const bool bValidationFailed = Options.bStrict ? (ErrorCount > 0 || WarningCount > 0) : (ErrorCount > 0);
	const EExitCode ExitCode = bReportWriteFailed
		? EExitCode::ReportWriteFailed
		: (bValidationFailed ? EExitCode::ValidationFailed : EExitCode::Success);
	const FString MachineSummary = BuildMachineSummaryJSON(Options, TotalIssues, ErrorCount, WarningCount, InfoCount,
		JSONReportPath, CSVReportPath, ExitCode);
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("PGXRegistryValidateResult: %s"), *MachineSummary);

	UE_LOG(LogPGXRegistryEditor, Display, TEXT("============================================"));
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("Result: %s (exit code %d)"),
		ExitCode == EExitCode::Success ? TEXT("PASS") : TEXT("FAIL"),
		static_cast<int32>(ExitCode));
	UE_LOG(LogPGXRegistryEditor, Display, TEXT("============================================"));

	return static_cast<int32>(ExitCode);
}
