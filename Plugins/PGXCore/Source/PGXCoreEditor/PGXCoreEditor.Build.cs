// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXCoreEditor : ModuleRules
{
	public PGXCoreEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"BlueprintGraph",
			"PGXCoreRuntime",
			"Slate",
			"SlateCore",
			"AssetTools",
			"EditorFramework",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Kismet",
			"KismetCompiler",
			"DeveloperSettings",
			"Projects",

			// EN: Editor internals — only used by .cpp and Private/ widgets
			// ES: Internos de editor — solo usados por .cpp y widgets en Private/
			"ToolMenus",
			"PropertyEditor",
			"ContentBrowser",
			"Settings",
			"InputCore",
			"WorkspaceMenuStructure",

			"PGXRegistryEditor",

			// EN: Immediate-mode UI for telemetry graphs and debug panels
			// ES: UI modo inmediato para graficos de telemetria y paneles debug
			"SlateIM"
		});
	}
}
