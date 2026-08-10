// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Templates/PGXBuiltInTemplates.h"
#include "Core/PGXScaffoldTemplateRegistry.h"
#include "Core/PGXScaffoldTypes.h"
#include "PGXScaffoldEditor.h"
#include "Logging/PGXLogMacros.h"

#define LOCTEXT_NAMESPACE "PGXBuiltInTemplates"

// EN: Helper to create a folder item / ES: Helper para crear un item de carpeta
static FPGXScaffoldTemplateItem MakeFolder(FName Id, const FText& Name, const FString& Path, FName Parent = NAME_None, int32 Order = 0)
{
	FPGXScaffoldTemplateItem Item;
	Item.ItemId = Id;
	Item.DisplayName = Name;
	Item.ActionType = EPGXScaffoldActionType::CreateFolder;
	Item.RelativePath = Path;
	Item.ParentItemId = Parent;
	Item.ExecutionOrder = Order;
	return Item;
}

// EN: Helper to create a DataAsset item / ES: Helper para crear un item de DataAsset
static FPGXScaffoldTemplateItem MakeDA(FName Id, const FText& Name, const FString& Path, FName ClassName, FName Parent = NAME_None, TArray<FName> Deps = {}, int32 Order = 0)
{
	FPGXScaffoldTemplateItem Item;
	Item.ItemId = Id;
	Item.DisplayName = Name;
	Item.ActionType = EPGXScaffoldActionType::CreateDataAsset;
	Item.RelativePath = Path;
	Item.AssetClassName = ClassName;
	Item.ParentItemId = Parent;
	Item.DependsOn = Deps;
	Item.ExecutionOrder = Order;
	return Item;
}

// EN: Helper to create a Blueprint item / ES: Helper para crear un item de Blueprint
static FPGXScaffoldTemplateItem MakeBP(FName Id, const FText& Name, const FString& Path, const FString& ParentClass, FName Parent = NAME_None, TArray<FName> Deps = {}, int32 Order = 0)
{
	FPGXScaffoldTemplateItem Item;
	Item.ItemId = Id;
	Item.DisplayName = Name;
	Item.ActionType = EPGXScaffoldActionType::CreateBlueprint;
	Item.RelativePath = Path;
	Item.ParentClassPath = ParentClass;
	Item.ParentItemId = Parent;
	Item.DependsOn = Deps;
	Item.ExecutionOrder = Order;
	return Item;
}

void FPGXBuiltInTemplates::RegisterAll(FPGXScaffoldTemplateRegistry& Registry)
{
	RegisterGameQuickstart(Registry);
	RegisterSystemDataSetup(Registry);
	RegisterBlueprintArchitecture(Registry);
	RegisterContentPack(Registry);

	PGX_LOG_INFO(LogPGXScaffold, TEXT("FPGXBuiltInTemplates: Registered 4 built-in templates"));
}

// ============================================================================
// T1: Game Project Quickstart
// ============================================================================
void FPGXBuiltInTemplates::RegisterGameQuickstart(FPGXScaffoldTemplateRegistry& Registry)
{
	FPGXScaffoldTemplate T;
	T.TemplateId = FName("pgx.template.game_quickstart");
	T.DisplayName = LOCTEXT("T1Name", "Game Project Quickstart");
	T.Description = LOCTEXT("T1Desc", "Standard folder structure for a new PGX game project with optional Config DataAssets.");
	T.Category = FName("Project Setup");
	T.DefaultVariables.Add(TEXT("ProjectName"), TEXT("MyGame"));
	T.VariableDisplayNames.Add(TEXT("ProjectName"), LOCTEXT("T1VarProject", "Project Name"));

	// EN: Root
	T.Items.Add(MakeFolder("root", LOCTEXT("T1Root", "{ProjectName}"), TEXT("{ProjectName}"), NAME_None, 0));

	// EN: Main folders
	T.Items.Add(MakeFolder("bp", LOCTEXT("T1BP", "Blueprints"), TEXT("{ProjectName}/Blueprints"), "root", 1));
	T.Items.Add(MakeFolder("bp_core", LOCTEXT("T1BPCore", "Core"), TEXT("{ProjectName}/Blueprints/Core"), "bp", 0));
	T.Items.Add(MakeFolder("bp_gf", LOCTEXT("T1BPGF", "GameFlow"), TEXT("{ProjectName}/Blueprints/GameFlow"), "bp", 1));
	T.Items.Add(MakeFolder("bp_char", LOCTEXT("T1BPChar", "Characters"), TEXT("{ProjectName}/Blueprints/Characters"), "bp", 2));
	T.Items.Add(MakeFolder("bp_ui", LOCTEXT("T1BPUI", "UI"), TEXT("{ProjectName}/Blueprints/UI"), "bp", 3));

	T.Items.Add(MakeFolder("data", LOCTEXT("T1Data", "Data"), TEXT("{ProjectName}/Data"), "root", 2));
	T.Items.Add(MakeFolder("data_pgx", LOCTEXT("T1DataPGX", "PGX"), TEXT("{ProjectName}/Data/PGX"), "data", 0));
	T.Items.Add(MakeFolder("data_pgx_gf", LOCTEXT("T1DataGF", "GameFlow"), TEXT("{ProjectName}/Data/PGX/GameFlow"), "data_pgx", 0));
	T.Items.Add(MakeFolder("data_pgx_save", LOCTEXT("T1DataSave", "Save"), TEXT("{ProjectName}/Data/PGX/Save"), "data_pgx", 1));
	T.Items.Add(MakeFolder("data_pgx_audio", LOCTEXT("T1DataAudio", "Audio Config"), TEXT("{ProjectName}/Data/PGX/Audio"), "data_pgx", 2));
	T.Items.Add(MakeFolder("data_pgx_loading", LOCTEXT("T1DataLoad", "Loading"), TEXT("{ProjectName}/Data/PGX/Loading"), "data_pgx", 3));
	T.Items.Add(MakeFolder("data_tables", LOCTEXT("T1DataTables", "Tables"), TEXT("{ProjectName}/Data/Tables"), "data", 1));

	T.Items.Add(MakeFolder("maps", LOCTEXT("T1Maps", "Maps"), TEXT("{ProjectName}/Maps"), "root", 3));
	T.Items.Add(MakeFolder("maps_main", LOCTEXT("T1MapsMain", "Main"), TEXT("{ProjectName}/Maps/Main"), "maps", 0));
	T.Items.Add(MakeFolder("maps_test", LOCTEXT("T1MapsTest", "Test"), TEXT("{ProjectName}/Maps/Test"), "maps", 1));

	T.Items.Add(MakeFolder("mats", LOCTEXT("T1Mats", "Materials"), TEXT("{ProjectName}/Materials"), "root", 4));
	T.Items.Add(MakeFolder("meshes", LOCTEXT("T1Meshes", "Meshes"), TEXT("{ProjectName}/Meshes"), "root", 5));
	T.Items.Add(MakeFolder("tex", LOCTEXT("T1Tex", "Textures"), TEXT("{ProjectName}/Textures"), "root", 6));

	T.Items.Add(MakeFolder("audio", LOCTEXT("T1Audio", "Audio"), TEXT("{ProjectName}/Audio"), "root", 7));
	T.Items.Add(MakeFolder("audio_sfx", LOCTEXT("T1SFX", "SFX"), TEXT("{ProjectName}/Audio/SFX"), "audio", 0));
	T.Items.Add(MakeFolder("audio_music", LOCTEXT("T1Music", "Music"), TEXT("{ProjectName}/Audio/Music"), "audio", 1));
	T.Items.Add(MakeFolder("audio_dlg", LOCTEXT("T1Dialogue", "Dialogue"), TEXT("{ProjectName}/Audio/Dialogue"), "audio", 2));

	// EN: Optional Config DAs
	T.Items.Add(MakeDA("da_profile", LOCTEXT("T1DAProfile", "ProfileConfig DA"), TEXT("{ProjectName}/Data/PGX/DA_{ProjectName}_ProfileConfig"),
		FName("UPGXProjectProfileConfig"), "data_pgx", {"data_pgx"}, 10));
	T.Items.Add(MakeDA("da_gf", LOCTEXT("T1DAGF", "GameFlowChannel DA"), TEXT("{ProjectName}/Data/PGX/GameFlow/DA_{ProjectName}_GameFlowChannel"),
		FName("UPGXGameFlowChannelConfig"), "data_pgx_gf", {"data_pgx_gf"}, 11));
	T.Items.Add(MakeDA("da_save", LOCTEXT("T1DASave", "SaveConfig DA"), TEXT("{ProjectName}/Data/PGX/Save/DA_{ProjectName}_SaveConfig"),
		FName("UPGXSaveConfig"), "data_pgx_save", {"data_pgx_save"}, 12));

	Registry.RegisterTemplate(T);
}

// ============================================================================
// T2: PGX System Data Setup
// ============================================================================
void FPGXBuiltInTemplates::RegisterSystemDataSetup(FPGXScaffoldTemplateRegistry& Registry)
{
	FPGXScaffoldTemplate T;
	T.TemplateId = FName("pgx.template.system_data");
	T.DisplayName = LOCTEXT("T2Name", "PGX System Data Setup");
	T.Description = LOCTEXT("T2Desc", "Config DataAsset structure for a specific PGX system. Incremental — safe to run multiple times.");
	T.Category = FName("PGX Systems");
	T.DefaultVariables.Add(TEXT("ProjectName"), TEXT("MyGame"));
	T.DefaultVariables.Add(TEXT("SystemName"), TEXT("Save"));
	T.VariableDisplayNames.Add(TEXT("ProjectName"), LOCTEXT("T2VarProject", "Project Name"));
	T.VariableDisplayNames.Add(TEXT("SystemName"), LOCTEXT("T2VarSystem", "System Name"));

	T.Items.Add(MakeFolder("sys_root", LOCTEXT("T2Root", "{SystemName} Data"), TEXT("{ProjectName}/Data/PGX/{SystemName}"), NAME_None, 0));
	T.Items.Add(MakeDA("sys_config", LOCTEXT("T2Config", "{SystemName} Config DA"), TEXT("{ProjectName}/Data/PGX/{SystemName}/DA_{ProjectName}_{SystemName}Config"),
		FName("UPrimaryDataAsset"), "sys_root", {"sys_root"}, 1));

	Registry.RegisterTemplate(T);
}

// ============================================================================
// T3: Blueprint Architecture
// ============================================================================
void FPGXBuiltInTemplates::RegisterBlueprintArchitecture(FPGXScaffoldTemplateRegistry& Registry)
{
	FPGXScaffoldTemplate T;
	T.TemplateId = FName("pgx.template.bp_architecture");
	T.DisplayName = LOCTEXT("T3Name", "Blueprint Architecture");
	T.Description = LOCTEXT("T3Desc", "Core Blueprint classes: GameMode, GameInstance, PlayerController, Character, HUD.");
	T.Category = FName("Blueprints");
	T.DefaultVariables.Add(TEXT("ProjectName"), TEXT("MyGame"));
	T.VariableDisplayNames.Add(TEXT("ProjectName"), LOCTEXT("T3VarProject", "Project Name"));

	T.Items.Add(MakeFolder("bp_root", LOCTEXT("T3Root", "Core Blueprints"), TEXT("{ProjectName}/Blueprints/Core"), NAME_None, 0));

	T.Items.Add(MakeBP("bp_gm", LOCTEXT("T3GM", "GameMode"), TEXT("{ProjectName}/Blueprints/Core/BP_{ProjectName}_GameMode"),
		TEXT("/Script/Engine.GameModeBase"), "bp_root", {"bp_root"}, 1));
	T.Items.Add(MakeBP("bp_gi", LOCTEXT("T3GI", "GameInstance"), TEXT("{ProjectName}/Blueprints/Core/BP_{ProjectName}_GameInstance"),
		TEXT("/Script/Engine.GameInstance"), "bp_root", {"bp_root"}, 2));
	T.Items.Add(MakeBP("bp_pc", LOCTEXT("T3PC", "PlayerController"), TEXT("{ProjectName}/Blueprints/Core/BP_{ProjectName}_PlayerController"),
		TEXT("/Script/Engine.PlayerController"), "bp_root", {"bp_root"}, 3));
	T.Items.Add(MakeBP("bp_char", LOCTEXT("T3Char", "Character"), TEXT("{ProjectName}/Blueprints/Core/BP_{ProjectName}_Character"),
		TEXT("/Script/Engine.Character"), "bp_root", {"bp_root"}, 4));
	T.Items.Add(MakeBP("bp_hud", LOCTEXT("T3HUD", "HUD"), TEXT("{ProjectName}/Blueprints/Core/BP_{ProjectName}_HUD"),
		TEXT("/Script/Engine.HUD"), "bp_root", {"bp_root"}, 5));

	Registry.RegisterTemplate(T);
}

// ============================================================================
// T4: Content Pack
// ============================================================================
void FPGXBuiltInTemplates::RegisterContentPack(FPGXScaffoldTemplateRegistry& Registry)
{
	FPGXScaffoldTemplate T;
	T.TemplateId = FName("pgx.template.content_pack");
	T.DisplayName = LOCTEXT("T4Name", "Content Pack");
	T.Description = LOCTEXT("T4Desc", "Standard folder structure for a content pack: Maps, Meshes, Materials, Textures, Audio, Data.");
	T.Category = FName("Content");
	T.DefaultVariables.Add(TEXT("PackName"), TEXT("MyPack"));
	T.VariableDisplayNames.Add(TEXT("PackName"), LOCTEXT("T4VarPack", "Pack Name"));

	T.Items.Add(MakeFolder("pack", LOCTEXT("T4Root", "{PackName}"), TEXT("{PackName}"), NAME_None, 0));
	T.Items.Add(MakeFolder("pack_maps", LOCTEXT("T4Maps", "Maps"), TEXT("{PackName}/Maps"), "pack", 1));
	T.Items.Add(MakeFolder("pack_meshes", LOCTEXT("T4Meshes", "Meshes"), TEXT("{PackName}/Meshes"), "pack", 2));
	T.Items.Add(MakeFolder("pack_mats", LOCTEXT("T4Mats", "Materials"), TEXT("{PackName}/Materials"), "pack", 3));
	T.Items.Add(MakeFolder("pack_tex", LOCTEXT("T4Tex", "Textures"), TEXT("{PackName}/Textures"), "pack", 4));
	T.Items.Add(MakeFolder("pack_audio", LOCTEXT("T4Audio", "Audio"), TEXT("{PackName}/Audio"), "pack", 5));
	T.Items.Add(MakeFolder("pack_data", LOCTEXT("T4Data", "Data"), TEXT("{PackName}/Data"), "pack", 6));

	Registry.RegisterTemplate(T);
}

#undef LOCTEXT_NAMESPACE
