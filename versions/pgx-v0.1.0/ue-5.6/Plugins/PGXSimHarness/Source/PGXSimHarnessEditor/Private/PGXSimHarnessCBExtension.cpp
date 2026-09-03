// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

// ============================================================================
// EN: Registry-driven Content Browser extension. Categories, entries, icons,
//     tooltips and class paths come from FPGXDemoRegistry::GetDemoEntries().
//     A new demo asset needs one registry entry and, when applicable, a
//     matching FPGXDemoPopulator method for educational defaults.
//
// ES: Extension de Content Browser dirigida por registro. Categorias, entradas,
//     iconos, tooltips y paths de clase proceden de
//     FPGXDemoRegistry::GetDemoEntries(). Un nuevo asset demo necesita una
//     entrada de registro y, cuando aplique, un metodo de FPGXDemoPopulator.
// ============================================================================

#include "PGXSimHarnessCBExtension.h"
#include "PGXDemoRegistry.h"
#include "PGXDemoPopulator.h"
#include "PGXSimHarnessEditorModule.h"
#include "Style/PGXEditorStyle.h"
#include "Notifications/PGXEditorNotification.h"

#include "ToolMenus.h"
#include "Styling/AppStyle.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "PGXSimHarnessCB"

void FPGXSimHarnessCBExtension::Register()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AddNewContextMenu");

	// EN: Create "PGX DEMO" section positioned AFTER the existing "PGX FRAMEWORK" section.
	// ES: Crear seccion "PGX DEMO" posicionada DESPUES de la seccion "PGX FRAMEWORK" existente.
	FToolMenuSection& Section = Menu->AddSection(
		"PGXDemoSection",
		LOCTEXT("PGXDemoHeader", "PGX DEMO"),
		FToolMenuInsert("PGXFrameworkSection", EToolMenuInsertType::After)
	);

	Section.AddSubMenu(
		"PGXDemo",
		LOCTEXT("PGXDemo", "PGX Demo"),
		LOCTEXT("PGXDemoTooltip", "Create pre-configured PGX DataAssets with educational example values"),
		FNewToolMenuDelegate::CreateStatic(&FPGXSimHarnessCBExtension::BuildDemoMenu),
		false,
		FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.SimHarness")
	);
}

void FPGXSimHarnessCBExtension::Unregister()
{
	if (UToolMenus* ToolMenus = UToolMenus::TryGet())
	{
		ToolMenus->RemoveSection("ContentBrowser.AddNewContextMenu", "PGXDemoSection");
	}
}

void FPGXSimHarnessCBExtension::BuildDemoMenu(UToolMenu* SubMenu)
{
	// EN: Build 7 numbered category submenus, each with its demo entries.
	// ES: Construir 7 submenus de categorias numeradas, cada uno con sus entradas demo.
	TArray<FString> Categories = FPGXDemoRegistry::GetDemoCategories();

	for (const FString& Category : Categories)
	{
		TArray<FPGXDemoEntry> Entries = FPGXDemoRegistry::GetEntriesForCategory(Category);
		if (Entries.Num() == 0)
		{
			continue;
		}

		FName SectionName = FName(*Category.Replace(TEXT(" "), TEXT("_")).Replace(TEXT("."), TEXT("_")));
		FToolMenuSection& Section = SubMenu->AddSection(SectionName);
		Section.Label = FText::FromString(Category);

		for (const FPGXDemoEntry& Entry : Entries)
		{
			FPGXDemoEntry CapturedEntry = Entry;
			FName EntryName = FName(*Entry.SuggestedName);

			// EN: Determine icon — PGX custom or UE built-in
			// ES: Determinar icono — PGX custom o UE built-in
			FSlateIcon EntryIcon;
			if (Entry.IconStyleName.StartsWith(TEXT("PGXEditor.")))
			{
				EntryIcon = FSlateIcon(FPGXEditorStyle::GetStyleSetName(), FName(*Entry.IconStyleName));
			}
			else
			{
				EntryIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), FName(*Entry.IconStyleName));
			}

			Section.AddMenuEntry(
				EntryName,
				FText::FromString(Entry.DisplayName),
				FText::FromString(Entry.Tooltip),
				EntryIcon,
				FUIAction(FExecuteAction::CreateLambda([CapturedEntry]()
				{
					FPGXSimHarnessCBExtension::CreateDemoAsset(CapturedEntry);
				}))
			);
		}
	}
}

void FPGXSimHarnessCBExtension::CreateDemoAsset(const FPGXDemoEntry& Entry)
{
	// EN: Resolve the DA class / ES: Resolver la clase DA
	UClass* DataAssetClass = FindObject<UClass>(nullptr, *Entry.ClassPath);
	if (!DataAssetClass)
	{
		DataAssetClass = LoadObject<UClass>(nullptr, *Entry.ClassPath);
	}

	if (!DataAssetClass)
	{
		UPGXEditorNotification::ShowError(
			FText::Format(LOCTEXT("ClassNotFound",
				"PGX Demo: Cannot create {0} — class not found. The required plugin may not be loaded.\nPath: {1}"),
				FText::FromString(Entry.DisplayName),
				FText::FromString(Entry.ClassPath))
		);
		return;
	}

	// EN: Show save dialog with demo-friendly defaults
	// ES: Mostrar dialogo de guardado con defaults amigables para demo
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FSaveAssetDialogConfig SaveConfig;
	SaveConfig.DialogTitleOverride = FText::Format(
		LOCTEXT("SaveDemoTitle", "Save Demo: {0}"), FText::FromString(Entry.DisplayName));
	SaveConfig.DefaultPath = TEXT("/Game/PGX_Demo");
	SaveConfig.DefaultAssetName = Entry.SuggestedName;
	SaveConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;

	FString SavePath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveConfig);
	if (SavePath.IsEmpty())
	{
		return; // EN: User cancelled / ES: Usuario cancelo
	}

	FString SavePackageName = FPackageName::ObjectPathToPackageName(SavePath);
	FString AssetName = FPackageName::GetLongPackageAssetName(SavePackageName);

	// EN: Create the DataAsset / ES: Crear el DataAsset
	UPackage* Package = CreatePackage(*SavePackageName);
	UDataAsset* NewDA = NewObject<UDataAsset>(Package, DataAssetClass, FName(*AssetName), RF_Public | RF_Standalone);

	if (!IsValid(NewDA))
	{
		UPGXEditorNotification::ShowError(
			FText::Format(LOCTEXT("DACreateFailed", "PGX Demo: Failed to create {0}"), FText::FromString(AssetName))
		);
		return;
	}

	// EN: Populate with demo values / ES: Poblar con valores demo
	bool bPopulated = FPGXDemoPopulator::PopulateDemo(NewDA);

	// EN: Register and mark dirty / ES: Registrar y marcar sucio
	FAssetRegistryModule::AssetCreated(NewDA);
	Package->MarkPackageDirty();

	// EN: Open in editor / ES: Abrir en editor
	if (GEditor)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewDA);
	}

	FString StatusText = bPopulated
		? FString::Printf(TEXT("PGX Demo: Created and populated %s"), *AssetName)
		: FString::Printf(TEXT("PGX Demo: Created %s (no population available for this type)"), *AssetName);

	UPGXEditorNotification::ShowInfo(FText::FromString(StatusText));
}

#undef LOCTEXT_NAMESPACE
