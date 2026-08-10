// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "AssetTools/PGXContentBrowserExtension.h"
#include "AssetTools/PGXAssetCreationRegistry.h"
#include "AssetTools/PGXBlueprintCreator.h"
#include "Style/PGXEditorStyle.h"

#include "ToolMenus.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "PGXContentBrowser"

// ═══════════════════════════════════════════
// Register / Unregister
// ═══════════════════════════════════════════

void FPGXContentBrowserExtension::Register()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AddNewContextMenu");

	// EN: Create a static section with its own "PGX FRAMEWORK" header, positioned after CREATE.
	//     The submenu's content delegate (BuildPGXFrameworkMenu) handles dynamic parts
	//     (checking which L2 modules are loaded). No need for AddDynamicSection.
	// ES: Crear una seccion estatica con header propio "PGX FRAMEWORK", posicionada despues de CREATE.
	//     El delegate del submenu (BuildPGXFrameworkMenu) maneja las partes dinamicas
	//     (verificar que modulos L2 estan cargados). No necesita AddDynamicSection.
	FToolMenuSection& Section = Menu->AddSection(
		"PGXFrameworkSection",
		LOCTEXT("PGXSectionHeader", "PGX FRAMEWORK"),
		FToolMenuInsert("ContentBrowserNewAsset", EToolMenuInsertType::After)
	);

	Section.AddSubMenu(
		"PGXFramework",
		LOCTEXT("PGXFramework", "PGX Framework"),
		LOCTEXT("PGXFrameworkTooltip", "Create PGX Framework Blueprints and DataAssets"),
		FNewToolMenuDelegate::CreateStatic(&FPGXContentBrowserExtension::BuildPGXFrameworkMenu),
		false,
		FSlateIcon(FPGXEditorStyle::GetStyleSetName(), "PGXEditor.Icon.Hub")
	);
}

void FPGXContentBrowserExtension::Unregister()
{
	if (UToolMenus* ToolMenus = UToolMenus::TryGet())
	{
		ToolMenus->RemoveSection("ContentBrowser.AddNewContextMenu", "PGXFrameworkSection");
	}
}

// ═══════════════════════════════════════════
// Full Submenu Tree
// ═══════════════════════════════════════════

void FPGXContentBrowserExtension::BuildPGXFrameworkMenu(UToolMenu* SubMenu)
{
	// EN: Get all system categories sorted (Core first, then alphabetical)
	// ES: Obtener todas las categorias de sistema ordenadas (Core primero, luego alfabetico)
	TArray<FString> Systems = FPGXAssetCreationRegistry::GetAllSystemCategories();

	FToolMenuSection& Section = SubMenu->AddSection("PGXSystems");

	for (const FString& System : Systems)
	{
		TArray<FPGXCreatableAssetEntry> BPEntries = FPGXAssetCreationRegistry::GetBlueprintEntriesForCategory(System);
		TArray<FPGXCreatableAssetEntry> DAEntries = FPGXAssetCreationRegistry::GetDataAssetEntriesForCategory(System);

		// EN: Filter BP entries to only show those with loaded modules
		// ES: Filtrar entradas BP para solo mostrar las de modulos cargados
		TArray<FPGXCreatableAssetEntry> VisibleBP;
		for (const FPGXCreatableAssetEntry& Entry : BPEntries)
		{
			if (FindObject<UClass>(nullptr, *Entry.ClassPath))
			{
				VisibleBP.Add(Entry);
			}
		}

		// EN: Skip system entirely if no entries are visible
		// ES: Omitir sistema entero si no hay entradas visibles
		if (VisibleBP.Num() == 0 && DAEntries.Num() == 0)
		{
			continue;
		}

		// EN: Add system submenu with icon
		// ES: Agregar submenu de sistema con icono
		FSlateIcon SystemIcon = GetSystemIcon(System);

		Section.AddSubMenu(
			FName(*FString::Printf(TEXT("PGX_%s"), *System)),
			FText::FromString(System),
			FText::Format(LOCTEXT("SystemTooltip", "PGX {0} assets"), FText::FromString(System)),
			FNewToolMenuDelegate::CreateLambda([VisibleBP, DAEntries](UToolMenu* SystemMenu)
			{
				FToolMenuSection& SubSection = SystemMenu->AddSection("Default");

				// ── Blueprints submenu ──
				if (VisibleBP.Num() > 0)
				{
					SubSection.AddSubMenu(
						"Blueprints",
						LOCTEXT("Blueprints", "Blueprints"),
						LOCTEXT("BlueprintsTooltip", "Create Blueprint subclasses"),
						FNewToolMenuDelegate::CreateLambda([VisibleBP](UToolMenu* BPMenu)
						{
							FToolMenuSection& BPSection = BPMenu->AddSection("Default");
							for (const FPGXCreatableAssetEntry& Entry : VisibleBP)
							{
								const auto& CapturedEntry = Entry;
								BPSection.AddMenuEntry(
									FName(*Entry.DisplayName.Replace(TEXT(" "), TEXT("_"))),
									FText::FromString(Entry.DisplayName),
									FText::Format(LOCTEXT("CreateBPTip", "Create {0} Blueprint"), FText::FromString(Entry.DisplayName)),
									FSlateIcon(FAppStyle::GetAppStyleSetName(), FName(*Entry.IconStyleName)),
									FUIAction(FExecuteAction::CreateLambda([CapturedEntry]()
									{
										FPGXBlueprintCreator::CreateBlueprintFromEntry(CapturedEntry);
									}))
								);
							}
						}),
						false,
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Blueprint")
					);
				}

				// ── DataAssets submenu ──
				if (DAEntries.Num() > 0)
				{
					SubSection.AddSubMenu(
						"DataAssets",
						LOCTEXT("DataAssets", "DataAssets"),
						LOCTEXT("DataAssetsTooltip", "Create DataAsset configurations"),
						FNewToolMenuDelegate::CreateLambda([DAEntries](UToolMenu* DAMenu)
						{
							FToolMenuSection& DASection = DAMenu->AddSection("Default");
							for (const FPGXCreatableAssetEntry& Entry : DAEntries)
							{
								const auto& CapturedEntry = Entry;
								DASection.AddMenuEntry(
									FName(*Entry.DisplayName.Replace(TEXT(" "), TEXT("_"))),
									FText::FromString(Entry.DisplayName),
									FText::Format(LOCTEXT("CreateDATip", "Create {0}"), FText::FromString(Entry.DisplayName)),
									FSlateIcon(FAppStyle::GetAppStyleSetName(), FName(*Entry.IconStyleName)),
									FUIAction(FExecuteAction::CreateLambda([CapturedEntry]()
									{
										FPGXBlueprintCreator::CreateDataAssetFromEntry(CapturedEntry);
									}))
								);
							}
						}),
						false,
						FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DataAsset")
					);
				}
			}),
			false,
			SystemIcon
		);
	}
}

// ═══════════════════════════════════════════
// System Icons
// ═══════════════════════════════════════════

FSlateIcon FPGXContentBrowserExtension::GetSystemIcon(const FString& System)
{
	// EN: Map system categories to icons. Use PGX custom icons where available, UE built-in otherwise.
	// ES: Mapear categorias de sistema a iconos. Usar iconos PGX custom donde existan, UE built-in si no.
	static const FName PGXStyle = FPGXEditorStyle::GetStyleSetName();
	static const FName UEStyle = FAppStyle::GetAppStyleSetName();

	if (System == TEXT("Core"))          return FSlateIcon(UEStyle, "ClassIcon.Actor");
	if (System == TEXT("AI"))            return FSlateIcon(UEStyle, "ClassIcon.AIController");
	if (System == TEXT("Animation"))     return FSlateIcon(UEStyle, "ClassIcon.AnimBlueprint");
	if (System == TEXT("Camera"))        return FSlateIcon(UEStyle, "ClassIcon.CameraComponent");
	if (System == TEXT("Documentation")) return FSlateIcon(PGXStyle, "PGXEditor.Icon.Docs");
	if (System == TEXT("GameFlow"))      return FSlateIcon(PGXStyle, "PGXEditor.Icon.GameFlow");
	if (System == TEXT("Input"))         return FSlateIcon(UEStyle, "ClassIcon.Object");
	if (System == TEXT("Inventory"))     return FSlateIcon(UEStyle, "ClassIcon.Object");
	if (System == TEXT("LevelFlow"))     return FSlateIcon(PGXStyle, "PGXEditor.Icon.LevelFlow");
	if (System == TEXT("Loading"))       return FSlateIcon(PGXStyle, "PGXEditor.Icon.Loading");
	if (System == TEXT("Log"))           return FSlateIcon(PGXStyle, "PGXEditor.Icon.LogViewer");
	if (System == TEXT("MGOS"))          return FSlateIcon(PGXStyle, "PGXEditor.Icon.MGOS");
	if (System == TEXT("Multiplayer"))   return FSlateIcon(UEStyle, "ClassIcon.Object");
	if (System == TEXT("Profile"))       return FSlateIcon(PGXStyle, "PGXEditor.Icon.Profile");
	if (System == TEXT("PSO"))           return FSlateIcon(PGXStyle, "PGXEditor.Icon.PSO");
	if (System == TEXT("Save"))          return FSlateIcon(PGXStyle, "PGXEditor.Icon.SaveInspector");
	if (System == TEXT("Spawn"))         return FSlateIcon(UEStyle, "ClassIcon.Object");
	if (System == TEXT("Audio"))         return FSlateIcon(PGXStyle, "PGXEditor.Icon.Audio");
	if (System == TEXT("Construction"))  return FSlateIcon(UEStyle, "ClassIcon.Object");
	if (System == TEXT("Registry"))      return FSlateIcon(PGXStyle, "PGXEditor.Icon.DataRegistry");
	if (System == TEXT("UI"))            return FSlateIcon(UEStyle, "ClassIcon.HUD");
	if (System == TEXT("Message"))       return FSlateIcon(PGXStyle, "PGXEditor.Icon.Message");
	if (System == TEXT("EventHandler"))  return FSlateIcon(PGXStyle, "PGXEditor.Icon.EventHandler");

	// EN: Fallback — generic Object icon (same as UE default for classes without a custom icon)
	// ES: Respaldo — icono generico Object (igual que el default de UE para clases sin icono custom)
	return FSlateIcon(UEStyle, "ClassIcon.Object");
}

#undef LOCTEXT_NAMESPACE
