// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: Tutorial action executor — asset/folder creation, navigation, cleanup.
// ES: Ejecutor de acciones de tutorial — creacion de assets/carpetas, navegacion, limpieza.

#include "PGXTutorialActionExecutor.h"
#include "Logging/PGXLogMacros.h"
#include "PGXTutorialTypes.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/DataAssetFactory.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ObjectTools.h"
#include "Editor.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXTutorialAction, Log, All);

// EN: Static tracking arrays / ES: Arrays estaticos de tracking
TArray<FString> FPGXTutorialActionExecutor::CreatedAssetPaths;
TArray<FString> FPGXTutorialActionExecutor::CreatedFolderPaths;
FString FPGXTutorialActionExecutor::CurrentTutorialRoot;

// EN: C3 path-whitelist helper. Returns true iff Path is at or under
//     CurrentTutorialRoot, with a strict `/` boundary to prevent
//     false-positive matches like '/Game/PGX_Tutorials_Other' passing
//     for root '/Game/PGX_Tutorials'. Empty root = fail-safe (refuse all).
// ES: Helper de path-whitelist para C3. Retorna true si Path esta en o
//     debajo de CurrentTutorialRoot, con boundary `/` estricto.
static bool IsPathUnderTutorialRoot(const FString& Path)
{
	const FString& Root = FPGXTutorialActionExecutor::CurrentTutorialRoot;
	if (Root.IsEmpty())
	{
		return false;
	}
	if (!Path.StartsWith(Root))
	{
		return false;
	}
	// EN: Boundary check: same length = exact match (allowed), or next char
	//     must be `/` to ensure Path is a descendant (not a sibling with
	//     same prefix).
	// ES: Boundary check: misma longitud = match exacto (permitido), o el
	//     siguiente caracter debe ser `/` para asegurar que Path es
	//     descendiente (no hermano con mismo prefijo).
	const int32 RootLen = Root.Len();
	return Path.Len() == RootLen || Path[RootLen] == TEXT('/');
}

// ============================================================================
// Public API
// ============================================================================

FPGXTutorialActionResult FPGXTutorialActionExecutor::Execute(const FPGXTutorialStep& Step, const FString& BasePath)
{
	// EN: C3 fix — record the BasePath as the current tutorial root BEFORE
	//     any action runs. Subsequent CleanupTutorialAssets() will use this
	//     as the path-whitelist anchor; tracked paths outside this root
	//     will be refused (defensive against stale array state).
	// ES: Fix C3 — registrar BasePath como root del tutorial actual ANTES
	//     de cualquier action. CleanupTutorialAssets() lo usara como anchor
	//     del whitelist; paths tracked fuera de este root seran rechazados.
	CurrentTutorialRoot = BasePath;

	if (Step.Action == EPGXTutorialAction::None || Step.Action == EPGXTutorialAction::ConfigBasePath)
	{
		return { true, FText::GetEmpty() };
	}

	const FString FullPath = BasePath / Step.ActionPath;

	switch (Step.Action)
	{
	case EPGXTutorialAction::CreateFolder:
		return CreateFolder(FullPath);

	case EPGXTutorialAction::CreateAsset:
		return CreateDataAsset(FullPath, Step.AssetClass, Step.AssetName);

	case EPGXTutorialAction::OpenAsset:
		return OpenAsset(FullPath, Step.AssetName);

	case EPGXTutorialAction::NavigateCB:
		return NavigateContentBrowser(FullPath);

	default:
		return { false, NSLOCTEXT("PGXTutorialAction", "UnknownAction", "Unknown action type") };
	}
}

bool FPGXTutorialActionExecutor::HasCreatedAssets()
{
	return CreatedAssetPaths.Num() > 0 || CreatedFolderPaths.Num() > 0;
}

void FPGXTutorialActionExecutor::ResetTracking()
{
	CreatedAssetPaths.Empty();
	CreatedFolderPaths.Empty();
	// EN: C3 fix — clear the path-whitelist root on reset. A subsequent
	// Execute() must set it again before cleanup can run safely.
	// ES: Fix C3 — limpiar el root del path-whitelist al reset. Un
	// Execute() subsecuente debe setearlo antes de que cleanup corra safe.
	CurrentTutorialRoot.Empty();
}

int32 FPGXTutorialActionExecutor::GetCreatedAssetCount()
{
	return CreatedAssetPaths.Num();
}

int32 FPGXTutorialActionExecutor::GetCreatedFolderCount()
{
	return CreatedFolderPaths.Num();
}

// ============================================================================
// Action implementations
// ============================================================================

FPGXTutorialActionResult FPGXTutorialActionExecutor::CreateFolder(const FString& FullPath)
{
	UEditorAssetSubsystem* AssetSub = GEditor ? GEditor->GetEditorSubsystem<UEditorAssetSubsystem>() : nullptr;
	if (!AssetSub)
	{
		return { false, NSLOCTEXT("PGXTutorialAction", "NoEditorSubsystem", "Editor subsystem not available") };
	}

	if (AssetSub->DoesDirectoryExist(FullPath))
	{
		PGX_LOG_INFO(LogPGXTutorialAction, TEXT("Directory already exists: %s"), *FullPath);
		return { true, FText::Format(
			NSLOCTEXT("PGXTutorialAction", "FolderExists", "Folder already exists: {0}"),
			FText::FromString(FullPath)) };
	}

	const bool bCreated = AssetSub->MakeDirectory(FullPath);
	if (bCreated)
	{
		CreatedFolderPaths.Add(FullPath);
		PGX_LOG_INFO(LogPGXTutorialAction, TEXT("Created directory: %s"), *FullPath);
		return { true, FText::Format(
			NSLOCTEXT("PGXTutorialAction", "FolderCreated", "Folder created: {0}"),
			FText::FromString(FullPath)) };
	}

	PGX_LOG_WARNING(LogPGXTutorialAction, TEXT("Failed to create directory: %s"), *FullPath);
	return { false, FText::Format(
		NSLOCTEXT("PGXTutorialAction", "FolderFailed", "Failed to create folder: {0}"),
		FText::FromString(FullPath)) };
}

FPGXTutorialActionResult FPGXTutorialActionExecutor::CreateDataAsset(
	const FString& FolderPath, const FString& AssetClassName, const FString& AssetName)
{
	// EN: Ensure folder exists first / ES: Asegurar que la carpeta existe primero
	UEditorAssetSubsystem* AssetSub = GEditor ? GEditor->GetEditorSubsystem<UEditorAssetSubsystem>() : nullptr;
	if (AssetSub && !AssetSub->DoesDirectoryExist(FolderPath))
	{
		AssetSub->MakeDirectory(FolderPath);
		CreatedFolderPaths.AddUnique(FolderPath);
	}

	// EN: Check if asset already exists — just open it / ES: Si ya existe, solo abrirlo
	const FString FullAssetPath = FolderPath / AssetName;
	if (AssetSub && AssetSub->DoesAssetExist(FullAssetPath))
	{
		UObject* Existing = LoadObject<UObject>(nullptr, *FullAssetPath);
		if (IsValid(Existing))
		{
			UAssetEditorSubsystem* EditorSub = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			if (EditorSub)
			{
				EditorSub->OpenEditorForAsset(Existing);
			}
		}
		PGX_LOG_INFO(LogPGXTutorialAction, TEXT("Asset already exists, opened: %s"), *FullAssetPath);
		return { true, FText::Format(
			NSLOCTEXT("PGXTutorialAction", "AssetExists", "Asset already exists (opened): {0}"),
			FText::FromString(FullAssetPath)) };
	}

	// EN: Resolve class / ES: Resolver clase
	UClass* TargetClass = FindObject<UClass>(nullptr, *AssetClassName);
	if (!TargetClass)
	{
		TargetClass = LoadObject<UClass>(nullptr, *AssetClassName);
	}
	if (!TargetClass)
	{
		PGX_LOG_ERROR(LogPGXTutorialAction, TEXT("Class not found: %s"), *AssetClassName);
		return { false, FText::Format(
			NSLOCTEXT("PGXTutorialAction", "ClassNotFound", "Class not found: {0}"),
			FText::FromString(AssetClassName)) };
	}

	// EN: Create via IAssetTools / ES: Crear via IAssetTools
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();

	UFactory* Factory = nullptr;
	if (TargetClass->IsChildOf(UDataAsset::StaticClass()))
	{
		UDataAssetFactory* DAFactory = NewObject<UDataAssetFactory>();
		DAFactory->DataAssetClass = TargetClass;
		Factory = DAFactory;
	}

	UObject* CreatedAsset = AssetTools.CreateAsset(AssetName, FolderPath, TargetClass, Factory);
	if (!IsValid(CreatedAsset))
	{
		PGX_LOG_ERROR(LogPGXTutorialAction, TEXT("Failed to create asset: %s at %s"), *AssetName, *FolderPath);
		return { false, FText::Format(
			NSLOCTEXT("PGXTutorialAction", "CreateFailed", "Failed to create: {0}"),
			FText::FromString(AssetName)) };
	}

	CreatedAssetPaths.Add(CreatedAsset->GetPathName());
	PGX_LOG_INFO(LogPGXTutorialAction, TEXT("Created asset: %s"), *CreatedAsset->GetPathName());

	// EN: Save asset to disk so cleanup can properly delete it later
	// ES: Guardar asset a disco para que la limpieza pueda borrarlo despues
	if (AssetSub)
	{
		AssetSub->SaveLoadedAsset(CreatedAsset);
	}

	// EN: Open in editor / ES: Abrir en editor
	UAssetEditorSubsystem* EditorSub = GEditor ? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>() : nullptr;
	if (EditorSub)
	{
		EditorSub->OpenEditorForAsset(CreatedAsset);
	}

	return { true, FText::Format(
		NSLOCTEXT("PGXTutorialAction", "AssetCreated", "Created and opened: {0}"),
		FText::FromString(CreatedAsset->GetPathName())) };
}

FPGXTutorialActionResult FPGXTutorialActionExecutor::OpenAsset(const FString& FolderPath, const FString& AssetName)
{
	const FString FullAssetPath = FolderPath / AssetName;

	UObject* Asset = LoadObject<UObject>(nullptr, *FullAssetPath);
	if (!IsValid(Asset))
	{
		PGX_LOG_WARNING(LogPGXTutorialAction, TEXT("Asset not found: %s"), *FullAssetPath);
		return { false, FText::Format(
			NSLOCTEXT("PGXTutorialAction", "AssetNotFound", "Asset not found: {0}"),
			FText::FromString(FullAssetPath)) };
	}

	UAssetEditorSubsystem* EditorSub = GEditor ? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>() : nullptr;
	if (EditorSub)
	{
		EditorSub->OpenEditorForAsset(Asset);
	}

	PGX_LOG_INFO(LogPGXTutorialAction, TEXT("Opened asset: %s"), *FullAssetPath);
	return { true, FText::Format(
		NSLOCTEXT("PGXTutorialAction", "AssetOpened", "Opened: {0}"),
		FText::FromString(FullAssetPath)) };
}

FPGXTutorialActionResult FPGXTutorialActionExecutor::NavigateContentBrowser(const FString& FullPath)
{
	FContentBrowserModule& CBModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	IContentBrowserSingleton& CB = CBModule.Get();

	TArray<FString> Folders;
	Folders.Add(FullPath);
	CB.SyncBrowserToFolders(Folders, /*bAllowLockedBrowsers=*/ false);

	PGX_LOG_INFO(LogPGXTutorialAction, TEXT("Navigated Content Browser to: %s"), *FullPath);
	return { true, FText::Format(
		NSLOCTEXT("PGXTutorialAction", "CBNavigated", "Content Browser: {0}"),
		FText::FromString(FullPath)) };
}

// ============================================================================
// Cleanup
// ============================================================================

FPGXTutorialActionResult FPGXTutorialActionExecutor::CleanupTutorialAssets()
{
	int32 DeletedAssets = 0;
	const int32 TotalAssets = CreatedAssetPaths.Num();

	UAssetEditorSubsystem* EditorSub = GEditor ? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>() : nullptr;

	// EN: Close editors and delete assets in reverse order
	// ES: Cerrar editores y borrar assets en orden inverso
	for (int32 i = CreatedAssetPaths.Num() - 1; i >= 0; --i)
	{
		const FString& AssetPath = CreatedAssetPaths[i];

		// EN: C3 fix — refuse to delete assets outside the current tutorial root.
		//     Defensive against stale array state (e.g., a previous tutorial's
		//     paths leaking into this run after ResetTracking was missed).
		// ES: Fix C3 — rechazar borrar assets fuera del root del tutorial.
		if (!IsPathUnderTutorialRoot(AssetPath))
		{
			PGX_LOG_WARNING(LogPGXTutorialAction,
				TEXT("[C3 fix] Refused asset delete outside tutorial root: %s (current root=%s)"),
				*AssetPath, *CurrentTutorialRoot);
			continue;
		}

		UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
		if (IsValid(Asset))
		{
			// EN: Close any open editor windows for this asset first
			// ES: Cerrar ventanas de editor abiertas para este asset primero
			if (EditorSub)
			{
				EditorSub->CloseAllEditorsForAsset(Asset);
			}

			TArray<UObject*> ToDelete;
			ToDelete.Add(Asset);
			const int32 Deleted = ObjectTools::DeleteObjects(ToDelete, /*bShowConfirmation=*/ false);
			if (Deleted > 0)
			{
				DeletedAssets++;
			}
		}
	}

	// EN: Delete created folders (deepest first by path length)
	// ES: Borrar carpetas creadas (las mas profundas primero por longitud de ruta)
	int32 DeletedFolders = 0;
	CreatedFolderPaths.Sort([](const FString& A, const FString& B)
	{
		return A.Len() > B.Len();
	});

	for (const FString& FolderPath : CreatedFolderPaths)
	{
		// EN: C3 fix — refuse to delete folders outside the current tutorial root.
		//     CRITICAL: DeleteDirectory with Tree=true is RECURSIVE, so a stale
		//     path would cascade into unexpected subtree deletion. The guard
		//     is the only thing standing between stale data and disk loss.
		// ES: Fix C3 — rechazar borrar carpetas fuera del root. CRITICO:
		//     DeleteDirectory con Tree=true es RECURSIVO.
		if (!IsPathUnderTutorialRoot(FolderPath))
		{
			PGX_LOG_WARNING(LogPGXTutorialAction,
				TEXT("[C3 fix] Refused folder delete outside tutorial root: %s (current root=%s)"),
				*FolderPath, *CurrentTutorialRoot);
			continue;
		}

		FString DiskPath;
		if (FPackageName::TryConvertLongPackageNameToFilename(FolderPath, DiskPath))
		{
			if (IFileManager::Get().DeleteDirectory(*DiskPath, false, true))
			{
				DeletedFolders++;
				PGX_LOG_INFO(LogPGXTutorialAction, TEXT("Deleted folder: %s"), *FolderPath);
			}
		}
	}

	ResetTracking();

	PGX_LOG_INFO(LogPGXTutorialAction, TEXT("Cleanup: deleted %d/%d assets, %d folders"),
		DeletedAssets, TotalAssets, DeletedFolders);

	return { true, FText::Format(
		NSLOCTEXT("PGXTutorialAction", "CleanupDone", "Cleaned up {0} assets, {1} folders"),
		FText::AsNumber(DeletedAssets), FText::AsNumber(DeletedFolders)) };
}
