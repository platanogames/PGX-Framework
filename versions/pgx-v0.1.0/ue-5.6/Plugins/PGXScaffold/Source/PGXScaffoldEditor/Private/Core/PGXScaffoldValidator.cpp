// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Core/PGXScaffoldValidator.h"
#include "PGXScaffoldEditor.h"
#include "Logging/PGXLogMacros.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "PGXScaffoldValidator"

TArray<FPGXScaffoldValidationResult> FPGXScaffoldValidator::Validate(
	const TArray<FPGXScaffoldTemplateItem>& Items,
	const TMap<FString, FString>& Variables,
	const FPGXScaffoldProjectInfo& ProjectInfo)
{
	TArray<FPGXScaffoldValidationResult> Results;
	bHasErrors = false;

	ValidateVariables(Items, Variables, Results);
	ValidatePaths(Items, Variables, ProjectInfo, Results);
	ValidateDuplicates(Items, Variables, Results);
	ValidateDependencies(Items, Results);

	// EN: Check if any result is Error or Fatal
	// ES: Verificar si algun resultado es Error o Fatal
	for (const auto& R : Results)
	{
		if (R.Severity == EPGXScaffoldSeverity::Error || R.Severity == EPGXScaffoldSeverity::Fatal)
		{
			bHasErrors = true;
			break;
		}
	}

	if (Results.Num() == 0)
	{
		FPGXScaffoldValidationResult Ok;
		Ok.Severity = EPGXScaffoldSeverity::Info;
		Ok.Message = LOCTEXT("ValidationPassed", "All checks passed. Ready to generate plan.");
		Results.Add(Ok);
	}

	PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXScaffoldValidator: Validation complete — %d findings, HasErrors=%s"),
		Results.Num(), bHasErrors ? TEXT("true") : TEXT("false"));

	return Results;
}

void FPGXScaffoldValidator::ValidateVariables(
	const TArray<FPGXScaffoldTemplateItem>& Items,
	const TMap<FString, FString>& Variables,
	TArray<FPGXScaffoldValidationResult>& OutResults)
{
	// EN: Find all {Variable} references in items and check they have values
	// ES: Encontrar todas las referencias {Variable} en items y verificar que tienen valores
	TSet<FString> RequiredVars;
	for (const auto& Item : Items)
	{
		FString Path = Item.RelativePath;
		int32 Start = 0;
		while ((Start = Path.Find(TEXT("{"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start)) != INDEX_NONE)
		{
			int32 End = Path.Find(TEXT("}"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start);
			if (End != INDEX_NONE)
			{
				FString VarName = Path.Mid(Start + 1, End - Start - 1);
				RequiredVars.Add(VarName);
				Start = End + 1;
			}
			else
			{
				break;
			}
		}
	}

	for (const FString& VarName : RequiredVars)
	{
		const FString* Value = Variables.Find(VarName);
		if (!Value || Value->IsEmpty())
		{
			FPGXScaffoldValidationResult R;
			R.Severity = EPGXScaffoldSeverity::Error;
			R.Message = FText::Format(LOCTEXT("VarMissing", "Variable '{0}' is required but empty"),
				FText::FromString(VarName));
			R.Suggestion = FText::Format(LOCTEXT("VarMissingSuggestion", "Enter a value for '{0}' in the configuration panel"),
				FText::FromString(VarName));
			OutResults.Add(R);
		}
	}
}

void FPGXScaffoldValidator::ValidatePaths(
	const TArray<FPGXScaffoldTemplateItem>& Items,
	const TMap<FString, FString>& Variables,
	const FPGXScaffoldProjectInfo& ProjectInfo,
	TArray<FPGXScaffoldValidationResult>& OutResults)
{
	for (const auto& Item : Items)
	{
		FString ResolvedPath = ResolveVariables(Item.RelativePath, Variables);

		// EN: Check if path already exists (idempotent: skip, not error for folders)
		// ES: Verificar si la ruta ya existe (idempotente: skip, no error para carpetas)
		if (Item.ActionType == EPGXScaffoldActionType::CreateFolder)
		{
			FString FullPath = ProjectInfo.ContentDir / ResolvedPath;
			if (IFileManager::Get().DirectoryExists(*FullPath))
			{
				FPGXScaffoldValidationResult R;
				R.Severity = EPGXScaffoldSeverity::Info;
				R.ItemId = Item.ItemId;
				R.Message = FText::Format(LOCTEXT("FolderExists", "Folder already exists: {0} (will be skipped)"),
					FText::FromString(ResolvedPath));
				OutResults.Add(R);
			}
		}
		else if (Item.ActionType == EPGXScaffoldActionType::CreateDataAsset ||
				 Item.ActionType == EPGXScaffoldActionType::CreateBlueprint)
		{
			// EN: Check if asset already exists
			// ES: Verificar si el asset ya existe
			FString AssetName = FPaths::GetBaseFilename(ResolvedPath);
			if (ProjectInfo.ExistingAssets.Contains(AssetName))
			{
				FPGXScaffoldValidationResult R;
				R.Severity = EPGXScaffoldSeverity::Warning;
				R.ItemId = Item.ItemId;
				R.Message = FText::Format(LOCTEXT("AssetExists", "Asset '{0}' already exists (will be skipped)"),
					FText::FromString(AssetName));
				OutResults.Add(R);
			}
		}
	}
}

void FPGXScaffoldValidator::ValidateDuplicates(
	const TArray<FPGXScaffoldTemplateItem>& Items,
	const TMap<FString, FString>& Variables,
	TArray<FPGXScaffoldValidationResult>& OutResults)
{
	// EN: Check for duplicate paths within the template selection
	// ES: Verificar rutas duplicadas dentro de la seleccion del template
	TMap<FString, int32> PathCounts;
	for (const auto& Item : Items)
	{
		FString Resolved = ResolveVariables(Item.RelativePath, Variables);
		PathCounts.FindOrAdd(Resolved, 0)++;
	}

	for (const auto& Pair : PathCounts)
	{
		if (Pair.Value > 1)
		{
			FPGXScaffoldValidationResult R;
			R.Severity = EPGXScaffoldSeverity::Error;
			R.Message = FText::Format(LOCTEXT("DuplicatePath", "Duplicate path in template: {0} ({1} occurrences)"),
				FText::FromString(Pair.Key), FText::AsNumber(Pair.Value));
			OutResults.Add(R);
		}
	}
}

void FPGXScaffoldValidator::ValidateDependencies(
	const TArray<FPGXScaffoldTemplateItem>& Items,
	TArray<FPGXScaffoldValidationResult>& OutResults)
{
	// EN: Build set of selected ItemIds
	// ES: Construir set de ItemIds seleccionados
	TSet<FName> SelectedIds;
	for (const auto& Item : Items)
	{
		SelectedIds.Add(Item.ItemId);
	}

	// EN: Check all DependsOn references are in the selection
	// ES: Verificar que todas las referencias DependsOn estan en la seleccion
	for (const auto& Item : Items)
	{
		for (const FName& DepId : Item.DependsOn)
		{
			if (!SelectedIds.Contains(DepId))
			{
				FPGXScaffoldValidationResult R;
				R.Severity = EPGXScaffoldSeverity::Error;
				R.ItemId = Item.ItemId;
				R.Message = FText::Format(LOCTEXT("MissingDep", "Item '{0}' depends on '{1}' which is not selected"),
					FText::FromName(Item.ItemId), FText::FromName(DepId));
				R.Suggestion = LOCTEXT("MissingDepSuggestion", "Select the required dependency item in the tree");
				OutResults.Add(R);
			}
		}
	}
}

FString FPGXScaffoldValidator::ResolveVariables(const FString& Path, const TMap<FString, FString>& Variables)
{
	FString Result = Path;
	for (const auto& Pair : Variables)
	{
		FString Token = FString::Printf(TEXT("{%s}"), *Pair.Key);
		Result = Result.Replace(*Token, *Pair.Value);
	}
	return Result;
}

#undef LOCTEXT_NAMESPACE
