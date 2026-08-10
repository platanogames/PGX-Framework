// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Core/PGXScaffoldExecutor.h"
#include "PGXScaffoldEditor.h"
#include "Logging/PGXLogMacros.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/DataAssetFactory.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "ISourceControlModule.h"
#include "SourceControlHelpers.h"

#define LOCTEXT_NAMESPACE "PGXScaffoldExecutor"

FPGXScaffoldExecutionResult FPGXScaffoldExecutor::Execute(
	FPGXScaffoldBuildPlan& Plan,
	const FPGXScaffoldProgressDelegate& ProgressCallback)
{
	// EN: Reset tracking for rollback / ES: Resetear tracking para rollback
	CreatedPaths.Empty();

	FPGXScaffoldExecutionResult Result;
	Result.Plan = Plan;
	Result.Status = EPGXScaffoldOperationStatus::Executing;

	const double StartTime = FPlatformTime::Seconds();
	const int32 TotalSteps = Plan.Steps.Num();

	PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXScaffoldExecutor: Starting execution — %d steps, template '%s'"),
		TotalSteps, *Plan.TemplateId.ToString());

	// EN: Wrap everything in a single transaction for Ctrl+Z rollback
	// ES: Envolver todo en una sola transaccion para rollback con Ctrl+Z
	FScopedTransaction Transaction(FText::Format(
		LOCTEXT("ScaffoldTransaction", "PGX Scaffold: {0}"),
		FText::FromName(Plan.TemplateId)));

	for (int32 i = 0; i < TotalSteps; ++i)
	{
		FPGXScaffoldPlanStep& Step = Plan.Steps[i];
		Step.Status = EPGXScaffoldStepStatus::InProgress;

		if (ProgressCallback.IsBound())
		{
			ProgressCallback.Execute(i, TotalSteps);
		}

		if (ExecuteStep(Step, Result.LogEntries))
		{
			Step.Status = EPGXScaffoldStepStatus::Completed;
			Result.CompletedSteps++;
		}
		else
		{
			// EN: Check if it was skipped (already exists) vs failed
			// ES: Verificar si fue skipped (ya existe) vs fallido
			if (Step.Status == EPGXScaffoldStepStatus::Skipped)
			{
				Result.SkippedSteps++;
			}
			else
			{
				Step.Status = EPGXScaffoldStepStatus::Failed;
				Result.FailedSteps++;

				PGX_LOG_ERROR(LogPGXScaffold, TEXT("FPGXScaffoldExecutor: Step %d FAILED — aborting remaining steps"), i);
				Result.Status = EPGXScaffoldOperationStatus::Failed;

				// EN: Mark remaining steps as skipped
				// ES: Marcar pasos restantes como skipped
				for (int32 j = i + 1; j < TotalSteps; ++j)
				{
					Plan.Steps[j].Status = EPGXScaffoldStepStatus::Skipped;
					Result.SkippedSteps++;
				}

				// EN: Clean up orphaned files from successful steps
				// ES: Limpiar archivos huerfanos de pasos exitosos
				RollbackCreatedPaths();
				break;
			}
		}
	}

	if (Result.Status != EPGXScaffoldOperationStatus::Failed)
	{
		Result.Status = EPGXScaffoldOperationStatus::Completed;
	}

	Result.TotalDurationMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;

	// EN: Final progress callback
	// ES: Callback final de progreso
	if (ProgressCallback.IsBound())
	{
		ProgressCallback.Execute(TotalSteps, TotalSteps);
	}

	PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXScaffoldExecutor: Execution %s — %d completed, %d skipped, %d failed (%.1fms)"),
		Result.Status == EPGXScaffoldOperationStatus::Completed ? TEXT("COMPLETED") : TEXT("FAILED"),
		Result.CompletedSteps, Result.SkippedSteps, Result.FailedSteps, Result.TotalDurationMs);

	return Result;
}

bool FPGXScaffoldExecutor::ExecuteStep(FPGXScaffoldPlanStep& Step, TArray<FPGXScaffoldLogEntry>& LogEntries)
{
	const double StepStart = FPlatformTime::Seconds();

	bool bSuccess = false;
	switch (Step.ActionType)
	{
	case EPGXScaffoldActionType::CreateFolder:
		bSuccess = ExecuteCreateFolder(Step, LogEntries);
		break;
	case EPGXScaffoldActionType::CreateDataAsset:
		bSuccess = ExecuteCreateDataAsset(Step, LogEntries);
		break;
	case EPGXScaffoldActionType::CreateBlueprint:
		bSuccess = ExecuteCreateBlueprint(Step, LogEntries);
		break;
	}

	FPGXScaffoldLogEntry Entry;
	Entry.Timestamp = FDateTime::Now();
	Entry.StepIndex = Step.StepIndex;
	Entry.Status = Step.Status;
	Entry.DurationMs = (FPlatformTime::Seconds() - StepStart) * 1000.0;
	Entry.Message = FString::Printf(TEXT("[%d] %s: %s"),
		Step.StepIndex,
		bSuccess ? (Step.Status == EPGXScaffoldStepStatus::Skipped ? TEXT("SKIP") : TEXT("OK")) : TEXT("FAIL"),
		*Step.AbsolutePath);
	LogEntries.Add(Entry);

	return bSuccess || Step.Status == EPGXScaffoldStepStatus::Skipped;
}

bool FPGXScaffoldExecutor::ExecuteCreateFolder(const FPGXScaffoldPlanStep& Step, TArray<FPGXScaffoldLogEntry>& /*LogEntries*/)
{
	// EN: Idempotent — skip if already exists
	// ES: Idempotente — skip si ya existe
	if (IFileManager::Get().DirectoryExists(*Step.AbsolutePath))
	{
		const_cast<FPGXScaffoldPlanStep&>(Step).Status = EPGXScaffoldStepStatus::Skipped;
		return false;
	}

	if (IFileManager::Get().MakeDirectory(*Step.AbsolutePath, true))
	{
		CreatedPaths.Add(Step.AbsolutePath);
		PGX_LOG_VERBOSE(LogPGXScaffold, TEXT("  Created folder: %s"), *Step.AbsolutePath);
		return true;
	}

	PGX_LOG_ERROR(LogPGXScaffold, TEXT("  Failed to create folder: %s"), *Step.AbsolutePath);
	return false;
}

bool FPGXScaffoldExecutor::ExecuteCreateDataAsset(const FPGXScaffoldPlanStep& Step, TArray<FPGXScaffoldLogEntry>& /*LogEntries*/)
{
	// EN: Convert disk path to UE package path: /Game/...
	// ES: Convertir ruta de disco a package path UE: /Game/...
	FString PackagePath;
	{
		FString RelPath = Step.AbsolutePath;
		FString ContentDir = FPaths::ProjectContentDir();
		FPaths::MakePathRelativeTo(RelPath, *ContentDir);
		// EN: Remove extension if any
		// ES: Remover extension si hay
		RelPath = FPaths::GetPath(RelPath);
		PackagePath = TEXT("/Game/") + RelPath;
		PackagePath.ReplaceInline(TEXT("\\"), TEXT("/"));
	}

	FString AssetName = FPaths::GetBaseFilename(Step.AbsolutePath);

	// EN: Check if asset already exists (idempotent)
	// ES: Verificar si el asset ya existe (idempotente)
	FString FullAssetPath = PackagePath / AssetName + TEXT(".") + AssetName;
	FAssetRegistryModule& ARModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	if (ARModule.Get().GetAssetByObjectPath(FSoftObjectPath(FullAssetPath)).IsValid())
	{
		const_cast<FPGXScaffoldPlanStep&>(Step).Status = EPGXScaffoldStepStatus::Skipped;
		return false;
	}

	// EN: Find the UClass for the DataAsset
	// ES: Encontrar la UClass del DataAsset
	UClass* AssetClass = FindObject<UClass>(nullptr, *FString::Printf(TEXT("/Script/PGXCoreRuntime.%s"), *Step.AssetClassName.ToString()));
	if (!IsValid(AssetClass))
	{
		// EN: Try engine classes (e.g. UPrimaryDataAsset, UDataAsset)
		// ES: Intentar clases del engine
		AssetClass = FindObject<UClass>(nullptr, *FString::Printf(TEXT("/Script/Engine.%s"), *Step.AssetClassName.ToString()));
	}
	if (!IsValid(AssetClass))
	{
		PGX_LOG_ERROR(LogPGXScaffold, TEXT("  Cannot find UClass '%s' for DataAsset creation"), *Step.AssetClassName.ToString());
		return false;
	}

	// EN: Use IAssetTools to create the asset
	// ES: Usar IAssetTools para crear el asset
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	UDataAssetFactory* Factory = NewObject<UDataAssetFactory>();
	Factory->AddToRoot(); // EN: Prevent GC during creation / ES: Prevenir GC durante creacion

	UObject* NewAsset = AssetTools.CreateAsset(AssetName, PackagePath, AssetClass, Factory);
	Factory->RemoveFromRoot();

	if (IsValid(NewAsset))
	{
		// EN: Save the new package
		// ES: Guardar el paquete nuevo
		UPackage* Package = NewAsset->GetOutermost();
		if (IsValid(Package))
		{
			Package->MarkPackageDirty();

			// EN: Track for rollback + source control / ES: Trackear para rollback + source control
			FString DiskPath = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
			CreatedPaths.Add(DiskPath);
			MarkForSourceControl(DiskPath);
		}
		PGX_LOG_VERBOSE(LogPGXScaffold, TEXT("  Created DataAsset: %s (%s)"), *AssetName, *AssetClass->GetName());
		return true;
	}

	PGX_LOG_ERROR(LogPGXScaffold, TEXT("  Failed to create DataAsset: %s"), *AssetName);
	return false;
}

bool FPGXScaffoldExecutor::ExecuteCreateBlueprint(const FPGXScaffoldPlanStep& Step, TArray<FPGXScaffoldLogEntry>& /*LogEntries*/)
{
	// EN: Convert disk path to package path
	// ES: Convertir ruta de disco a package path
	FString PackagePath;
	{
		FString RelPath = Step.AbsolutePath;
		FString ContentDir = FPaths::ProjectContentDir();
		FPaths::MakePathRelativeTo(RelPath, *ContentDir);
		RelPath = FPaths::GetPath(RelPath);
		PackagePath = TEXT("/Game/") + RelPath;
		PackagePath.ReplaceInline(TEXT("\\"), TEXT("/"));
	}

	FString AssetName = FPaths::GetBaseFilename(Step.AbsolutePath);

	// EN: Check if already exists (idempotent)
	// ES: Verificar si ya existe (idempotente)
	FString FullAssetPath = PackagePath / AssetName + TEXT(".") + AssetName;
	FAssetRegistryModule& ARModule2 = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	if (ARModule2.Get().GetAssetByObjectPath(FSoftObjectPath(FullAssetPath)).IsValid())
	{
		const_cast<FPGXScaffoldPlanStep&>(Step).Status = EPGXScaffoldStepStatus::Skipped;
		return false;
	}

	// EN: Find parent class
	// ES: Encontrar clase padre
	UClass* ParentClass = nullptr;
	if (!Step.ParentClassPath.IsEmpty())
	{
		ParentClass = LoadObject<UClass>(nullptr, *Step.ParentClassPath);
	}
	if (!IsValid(ParentClass))
	{
		ParentClass = AActor::StaticClass();
	}

	// EN: Create Blueprint using Kismet utilities
	// ES: Crear Blueprint usando utilidades Kismet
	FString PackageFullPath = PackagePath / AssetName;
	UPackage* Package = CreatePackage(*PackageFullPath);
	if (!IsValid(Package))
	{
		PGX_LOG_ERROR(LogPGXScaffold, TEXT("  Failed to create package for Blueprint: %s"), *PackageFullPath);
		return false;
	}

	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		Package,
		FName(*AssetName),
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass());

	if (IsValid(NewBP))
	{
		FAssetRegistryModule::AssetCreated(NewBP);
		Package->MarkPackageDirty();

		// EN: Track for rollback + source control / ES: Trackear para rollback + source control
		FString DiskPath = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		CreatedPaths.Add(DiskPath);
		MarkForSourceControl(DiskPath);

		PGX_LOG_VERBOSE(LogPGXScaffold, TEXT("  Created Blueprint: %s (parent: %s)"), *AssetName, *ParentClass->GetName());
		return true;
	}

	PGX_LOG_ERROR(LogPGXScaffold, TEXT("  Failed to create Blueprint: %s"), *AssetName);
	return false;
}

// ============================================================================
// F1: Source Control Integration
// ============================================================================

void FPGXScaffoldExecutor::MarkForSourceControl(const FString& FilePath)
{
	// EN: Best-effort, silent — if SC is not enabled, nothing happens
	// ES: Best-effort, silencioso — si SC no esta habilitado, no pasa nada
	ISourceControlModule* SCModule = ISourceControlModule::GetPtr();
	if (!SCModule || !SCModule->IsEnabled() || !SCModule->GetProvider().IsAvailable())
	{
		return;
	}
	USourceControlHelpers::MarkFileForAdd(FilePath, /*bSilent=*/true);
	PGX_LOG_VERBOSE(LogPGXScaffold, TEXT("  Marked for source control: %s"), *FilePath);
}

// ============================================================================
// F3: Pseudo-Rollback
// ============================================================================

void FPGXScaffoldExecutor::RollbackCreatedPaths()
{
	if (CreatedPaths.Num() == 0) { return; }

	PGX_LOG_WARNING(LogPGXScaffold, TEXT("FPGXScaffoldExecutor: Rolling back %d created items..."),
		CreatedPaths.Num());

	// EN: Reverse order — children before parents so folders are empty when we delete them
	// ES: Orden inverso — hijos antes que padres para que las carpetas esten vacias al borrarlas
	for (int32 i = CreatedPaths.Num() - 1; i >= 0; --i)
	{
		const FString& Path = CreatedPaths[i];

		if (IFileManager::Get().DirectoryExists(*Path))
		{
			// EN: Only delete if directory is empty (safety — never delete user content)
			// ES: Solo borrar si el directorio esta vacio (seguridad — nunca borrar contenido del usuario)
			TArray<FString> FilesInDir;
			IFileManager::Get().FindFilesRecursive(FilesInDir, *Path, TEXT("*"), true, true);
			if (FilesInDir.Num() == 0)
			{
				IFileManager::Get().DeleteDirectory(*Path, false, false);
				PGX_LOG_VERBOSE(LogPGXScaffold, TEXT("  Rolled back empty folder: %s"), *Path);
			}
			else
			{
				PGX_LOG_VERBOSE(LogPGXScaffold, TEXT("  Skipped rollback (not empty): %s"), *Path);
			}
		}
		// EN: Assets are handled by FScopedTransaction undo (Ctrl+Z)
		// ES: Los assets se manejan por el undo de FScopedTransaction (Ctrl+Z)
	}

	CreatedPaths.Empty();
}

#undef LOCTEXT_NAMESPACE
