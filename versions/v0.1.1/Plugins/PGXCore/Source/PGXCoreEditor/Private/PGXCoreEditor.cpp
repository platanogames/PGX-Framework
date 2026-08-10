// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCoreEditor.h"

// Toolbar
#include "Toolbar/PGXMenuCommands.h"
#include "Toolbar/PGXToolbarBuilder.h"

// Asset Tools
#include "AssetTools/PGXAssetTypeActions.h"
#include "AssetTools/PGXAssetCreationRegistry.h"  // EN: For FPGXCreatableAssetEntry / ES: Para FPGXCreatableAssetEntry
#include "AssetTools/PGXContentBrowserExtension.h"

// Details
#include "Details/FPGXDataAssetCustomization.h"
#include "Details/FPGXActorCustomization.h"

// Status Bar
#include "StatusBar/SPGXStatusBarWidget.h"

// Style
#include "Style/PGXEditorStyle.h"

// Workspace Menu
#include "Workspace/PGXWorkspaceMenu.h"

// Inspector NomadTab spawners (PGXMessage + PGXSave Inspectors)
#include "Inspector/FPGXMessageInspectorTabSpawner.h"

// Module APIs
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "Misc/CoreDelegates.h"
#include "Engine/Engine.h"

// Core runtime types
#include "Data/PGXConfigDataAsset.h"
#include "Data/PGXObjectDataAsset.h"
#include "Base/PGXActorBase.h"
#include "Base/PGXCharacterBase.h"

// Core DataAsset types
#include "Profile/PGXProjectProfileConfig.h"

// Construction DataAsset types + startup validator
#include "Construction/PGXConstructionStartupValidator.h"
#include "Construction/PGXGameModeConstruction.h"
#include "Construction/PGXPlayerControllerConstruction.h"
#include "Construction/PGXGameStateConstruction.h"
#include "Construction/PGXPlayerStateConstruction.h"
#include "Construction/PGXCharacterConstruction.h"
#include "Construction/PGXPawnConstruction.h"
#include "Construction/PGXHUDConstruction.h"

DEFINE_LOG_CATEGORY(LogPGXCoreEditor);

#define LOCTEXT_NAMESPACE "FPGXCoreEditorModule"

void FPGXCoreEditorModule::StartupModule()
{
	UE_LOG(LogPGXCoreEditor, Log, TEXT("PGXCoreEditor: Module starting..."));

	// === Register Editor Style (custom icons) ===
	FPGXEditorStyle::Initialize();

	// === Register Workspace Menu Groups (must be after Style, before tab spawners) ===
	FPGXWorkspaceMenu::Initialize();

	// === Register Commands ===
	FPGXMenuCommands::Register();

	// === Register Toolbar ===
	FPGXToolbarBuilder::RegisterToolbar();

	// === Register Quick Access Buttons ===
	FPGXToolbarBuilder::RegisterQuickAccessButtons();

	// === Register Content Browser "Add New" Section ===
	FPGXContentBrowserExtension::Register();

	// === Register PGX Asset Category and Type Actions ===
	// EN: Type actions provide color/name/icon for existing DA assets in Content Browser.
	//     Creation is handled by FPGXContentBrowserExtension's custom UToolMenus section.
	//     Type actions are now derived from PGXAssetCreationRegistry (single source of truth).
	// ES: Las type actions proveen color/nombre/icono para DAs existentes en Content Browser.
	//     La creacion se maneja via la seccion UToolMenus custom de FPGXContentBrowserExtension.
	//     Las type actions ahora se derivan de PGXAssetCreationRegistry (fuente unica de verdad).
	// EN: Provider runtime modules share the Default loading phase with PGXCoreEditor.
	//     Defer resolution until PostEngineInit so /Script/<Provider> classes exist.
	//     Hot reload happens after engine init, so register immediately in that case.
	// ES: Los modulos runtime proveedores comparten fase Default con PGXCoreEditor.
	//     Diferir la resolucion hasta PostEngineInit para que existan las clases /Script/<Provider>.
	//     Hot reload ocurre tras iniciar el engine, por lo que se registra inmediatamente.
	if (GEngine)
	{
		RegisterAssetTypeActions();
	}
	else
	{
		PostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddRaw(this, &FPGXCoreEditorModule::HandlePostEngineInit);
	}

	const TArray<FPGXCreatableAssetEntry>& DataAssetEntries = FPGXAssetCreationRegistry::GetDataAssetEntries();

	// === Register Detail Customizations ===
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout(
		UPGXConfigDataAsset::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FPGXDataAssetCustomization::MakeInstance)
	);
	PropertyModule.RegisterCustomClassLayout(
		UPGXObjectDataAsset::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FPGXDataAssetCustomization::MakeInstance)
	);
	PropertyModule.RegisterCustomClassLayout(
		APGXActorBase::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FPGXActorCustomization::MakeInstance)
	);
	PropertyModule.RegisterCustomClassLayout(
		APGXCharacterBase::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FPGXActorCustomization::MakeInstance)
	);

	// === Register Status Bar ===
	SPGXStatusBarWidget::Register();

	// === Schedule Construction Workflow Validation ===
	FPGXConstructionStartupValidator::ScheduleValidation();

	// EN: Derive counts from the real registry / ES: Derivar conteos del registry real
	const TArray<FPGXCreatableAssetEntry>& BPEntries = FPGXAssetCreationRegistry::GetBlueprintEntries();
	int32 ConstructionCount = 0;
	for (const FPGXCreatableAssetEntry& Entry : DataAssetEntries)
	{
		if (Entry.Category == TEXT("Construction")) { ConstructionCount++; }
	}
	// EN: Inspector NomadTab spawners removed from PGXCore — registered by PGXEditorTools
	//     (single source of truth). PGXCore only owns PGXMessageInspector + PGXSaveInspector
	//     widget implementations used by both. Duplicate registration was causing the second
	//     RegisterNomadTabSpawner to silently succeed but ShutdownModule to unregister the
	//     tab that PGXEditorTools registered, breaking inspector availability after hot-reload.
	// ES: Spawners Inspector NomadTab eliminados de PGXCore — registrados por PGXEditorTools
	//     (fuente unica de verdad). PGXCore solo posee las implementaciones de widget
	//     SPGXMessageInspectorTab usada por ambos. La duplicacion
	//     causaba que el segundo RegisterNomadTabSpawner sobrescribiera silenciosamente pero
	//     ShutdownModule desregistrara el tab de PGXEditorTools, rompiendo inspectors tras
	//     hot-reload.
	UE_LOG(LogPGXCoreEditor, Log, TEXT("PGXCoreEditor: Module started - Toolbar, Quick Access, %d BP Factories, %d DA Types (incl. %d Construction), Inspectors and Details registered"),
		BPEntries.Num(), DataAssetEntries.Num(), ConstructionCount);
}

void FPGXCoreEditorModule::ShutdownModule()
{
	if (PostEngineInitHandle.IsValid())
	{
		FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
		PostEngineInitHandle.Reset();
	}
	// EN: Inspector NomadTab spawners are now owned by PGXEditorTools.
	//     PGXCore only provides the SPGXMessageInspectorTab widget implementation.
	// ES: Spawners Inspector NomadTab ahora pertenecen a PGXEditorTools.
	//     PGXCore solo provee implementaciones de widget.
	// Unregister status bar
	SPGXStatusBarWidget::Unregister();

	// Unregister detail customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UPGXConfigDataAsset::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UPGXObjectDataAsset::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(APGXActorBase::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(APGXCharacterBase::StaticClass()->GetFName());
	}

	// Unregister asset type actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
	}
	RegisteredAssetTypeActions.Empty();

	// Unregister Content Browser section
	FPGXContentBrowserExtension::Unregister();

	// Unregister toolbar and commands
	FPGXToolbarBuilder::UnregisterToolbar();
	FPGXMenuCommands::Unregister();

	// Unregister editor style
	FPGXEditorStyle::Shutdown();

	UE_LOG(LogPGXCoreEditor, Log, TEXT("PGXCoreEditor: Module shut down"));
}

void FPGXCoreEditorModule::HandlePostEngineInit()
{
	FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
	PostEngineInitHandle.Reset();
	RegisterAssetTypeActions();
}

void FPGXCoreEditorModule::RegisterAssetTypeActions()
{
	if (RegisteredAssetTypeActions.Num() > 0)
	{
		return;
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	PGXAssetCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("PGX")),
		LOCTEXT("PGXCategory", "PGX Framework")
	);

	const TArray<FPGXCreatableAssetEntry>& DataAssetEntries = FPGXAssetCreationRegistry::GetDataAssetEntries();
	for (const FPGXCreatableAssetEntry& Entry : DataAssetEntries)
	{
		// EN: A soft class load probes the object before loading its /Script module and
		//     emits a misleading missing-class warning. Load the owning provider first.
		// ES: Una carga soft consulta el objeto antes de cargar su modulo /Script y
		//     emite un warning enganoso de clase ausente. Cargar primero el proveedor.
		constexpr int32 ScriptPrefixLength = 8; // "/Script/"
		int32 ClassSeparatorIndex = INDEX_NONE;
		if (Entry.ClassPath.StartsWith(TEXT("/Script/")) && Entry.ClassPath.FindChar(TEXT('.'), ClassSeparatorIndex))
		{
			const FString ProviderModuleName = Entry.ClassPath.Mid(ScriptPrefixLength, ClassSeparatorIndex - ScriptPrefixLength);
			if (!FModuleManager::Get().IsModuleLoaded(*ProviderModuleName)
				&& !FModuleManager::Get().LoadModule(*ProviderModuleName))
			{
				UE_LOG(LogPGXCoreEditor, Warning, TEXT("PGXCoreEditor: Failed to load provider module for TypeAction: %s"), *ProviderModuleName);
				continue;
			}
		}

		TSoftClassPtr<UObject> SoftClass(Entry.ClassPath);
		UClass* Class = SoftClass.LoadSynchronous();
		if (!Class)
		{
			UE_LOG(LogPGXCoreEditor, Warning, TEXT("PGXCoreEditor: Failed to load class for TypeAction: %s"), *Entry.ClassPath);
			continue;
		}

		TArray<FText> SubMenus;
		if (!Entry.Category.IsEmpty())
		{
			SubMenus.Add(FText::FromString(Entry.Category));
		}

		TSharedPtr<FPGXAssetTypeActions> Actions = MakeShareable(
			new FPGXAssetTypeActions(
				FText::FromString(Entry.DisplayName),
				Entry.TypeActionColor,
				Class,
				PGXAssetCategory,
				MoveTemp(SubMenus)
			)
		);
		AssetTools.RegisterAssetTypeActions(Actions.ToSharedRef());
		RegisteredAssetTypeActions.Add(Actions);
	}

	UE_LOG(LogPGXCoreEditor, Log, TEXT("PGXCoreEditor: Registered %d/%d DataAsset type actions from central registry"),
		RegisteredAssetTypeActions.Num(), DataAssetEntries.Num());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXCoreEditorModule, PGXCoreEditor)
