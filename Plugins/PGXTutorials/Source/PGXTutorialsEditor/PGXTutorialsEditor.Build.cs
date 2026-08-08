// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXTutorialsEditor : ModuleRules
{
	public PGXTutorialsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Engine",
			"UnrealEd",
			"InputCore",
			"EditorFramework",
			"ToolMenus",
			"WorkspaceMenuStructure",
			"PGXCoreEditor",
			"PGXCoreRuntime",
			"AssetTools",
			"ContentBrowser",
			"EditorScriptingUtilities"
		});
	}
}
