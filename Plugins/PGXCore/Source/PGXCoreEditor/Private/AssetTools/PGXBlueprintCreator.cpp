// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "AssetTools/PGXBlueprintCreator.h"
#include "AssetTools/PGXAssetCreationRegistry.h"
#include "Notifications/PGXEditorNotification.h"
#include "Settings/PGXEditorSettings.h"

#include "Kismet2/KismetEditorUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "Engine/Blueprint.h"
#include "Engine/DataAsset.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "PGXBlueprintCreator"

UClass* FPGXBlueprintCreator::ResolveClass(const FString& ClassPath, const FString& DisplayName)
{
	// EN: Try FindObject first (already loaded), then LoadObject (force load)
	// ES: Intentar FindObject primero (ya cargado), luego LoadObject (forzar carga)
	UClass* Class = FindObject<UClass>(nullptr, *ClassPath);
	if (!Class)
	{
		Class = LoadObject<UClass>(nullptr, *ClassPath);
	}

	if (!Class)
	{
		UPGXEditorNotification::ShowError(
			FText::Format(LOCTEXT("ClassNotFound",
				"PGX: Cannot create {0} — base class not found.\nThe required plugin may not be loaded.\nPath: {1}"),
				FText::FromString(DisplayName),
				FText::FromString(ClassPath))
		);
	}

	return Class;
}

void FPGXBlueprintCreator::CreateBlueprintFromEntry(const FPGXCreatableAssetEntry& Entry)
{
	UClass* ParentClass = ResolveClass(Entry.ClassPath, Entry.DisplayName);
	if (!ParentClass)
	{
		return;
	}

	// EN: Determine default path from settings / ES: Determinar ruta por defecto desde settings
	const UPGXEditorSettings* Settings = GetDefault<UPGXEditorSettings>();
	FString DefaultPath = Settings->DefaultBlueprintPath.Path;
	if (DefaultPath.IsEmpty())
	{
		DefaultPath = TEXT("/Game/Blueprints/PGX");
	}

	// EN: Build default asset path / ES: Construir ruta de asset por defecto
	FString DefaultAssetPath = DefaultPath / Entry.SuggestedName;

	// EN: Show save dialog / ES: Mostrar dialogo de guardado
	FString PackageName;
	FString AssetName;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FSaveAssetDialogConfig SaveConfig;
	SaveConfig.DialogTitleOverride = FText::Format(LOCTEXT("SaveBPTitle", "Save {0} Blueprint"), FText::FromString(Entry.DisplayName));
	SaveConfig.DefaultPath = DefaultPath;
	SaveConfig.DefaultAssetName = Entry.SuggestedName;
	SaveConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;

	FString SavePath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveConfig);
	if (SavePath.IsEmpty())
	{
		return; // EN: User cancelled / ES: Usuario cancelo
	}

	// EN: Extract package/asset name from save path / ES: Extraer nombre de paquete/asset del path
	FString SavePackageName = FPackageName::ObjectPathToPackageName(SavePath);
	AssetName = FPackageName::GetLongPackageAssetName(SavePackageName);

	// EN: Create the Blueprint / ES: Crear el Blueprint
	UPackage* Package = CreatePackage(*SavePackageName);
	UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
		ParentClass,
		Package,
		FName(*AssetName),
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass()
	);

	if (!NewBP)
	{
		UPGXEditorNotification::ShowError(
			FText::Format(LOCTEXT("BPCreateFailed", "PGX: Failed to create Blueprint {0}"), FText::FromString(AssetName))
		);
		return;
	}

	// EN: Register with AssetRegistry and save / ES: Registrar en AssetRegistry y guardar
	FAssetRegistryModule::AssetCreated(NewBP);
	Package->MarkPackageDirty();

	// EN: Open in Blueprint Editor / ES: Abrir en Blueprint Editor
	if (GEditor)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewBP);
	}

	UPGXEditorNotification::ShowInfo(
		FText::Format(LOCTEXT("BPCreated", "PGX: Created {0}"), FText::FromString(AssetName))
	);
}

void FPGXBlueprintCreator::CreateDataAssetFromEntry(const FPGXCreatableAssetEntry& Entry)
{
	UClass* DataAssetClass = ResolveClass(Entry.ClassPath, Entry.DisplayName);
	if (!DataAssetClass)
	{
		return;
	}

	// EN: Determine default path from settings / ES: Determinar ruta por defecto desde settings
	const UPGXEditorSettings* Settings = GetDefault<UPGXEditorSettings>();
	FString DefaultPath = Settings->DefaultBlueprintPath.Path;
	if (DefaultPath.IsEmpty())
	{
		DefaultPath = TEXT("/Game/Blueprints/PGX");
	}

	// EN: Show save dialog / ES: Mostrar dialogo de guardado
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FSaveAssetDialogConfig SaveConfig;
	SaveConfig.DialogTitleOverride = FText::Format(LOCTEXT("SaveDATitle", "Save {0}"), FText::FromString(Entry.DisplayName));
	SaveConfig.DefaultPath = DefaultPath;
	SaveConfig.DefaultAssetName = Entry.SuggestedName;
	SaveConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;

	FString SavePath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveConfig);
	if (SavePath.IsEmpty())
	{
		return;
	}

	FString SavePackageName = FPackageName::ObjectPathToPackageName(SavePath);
	FString AssetName = FPackageName::GetLongPackageAssetName(SavePackageName);

	// EN: Create the DataAsset / ES: Crear el DataAsset
	UPackage* Package = CreatePackage(*SavePackageName);
	UDataAsset* NewDA = NewObject<UDataAsset>(Package, DataAssetClass, FName(*AssetName), RF_Public | RF_Standalone);

	if (!NewDA)
	{
		UPGXEditorNotification::ShowError(
			FText::Format(LOCTEXT("DACreateFailed", "PGX: Failed to create {0}"), FText::FromString(AssetName))
		);
		return;
	}

	FAssetRegistryModule::AssetCreated(NewDA);
	Package->MarkPackageDirty();

	if (GEditor)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewDA);
	}

	UPGXEditorNotification::ShowInfo(
		FText::Format(LOCTEXT("DACreated", "PGX: Created {0}"), FText::FromString(AssetName))
	);
}

#undef LOCTEXT_NAMESPACE
