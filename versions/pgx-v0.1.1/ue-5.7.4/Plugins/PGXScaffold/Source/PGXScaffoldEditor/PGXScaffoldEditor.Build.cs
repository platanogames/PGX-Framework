// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXScaffoldEditor : ModuleRules
{
	public PGXScaffoldEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"GameplayTags",
			"Json",
			"PGXCoreRuntime",
			"PGXCoreEditor"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// EN: Editor internals — only used by .cpp files
			// ES: Internos de editor — solo usados por archivos .cpp
			"AssetTools",
			"AssetRegistry",
			"ContentBrowser",
			"ToolMenus",
			"InputCore",
			"WorkspaceMenuStructure",
			"Projects",
			"SourceControl"
		});
	}
}
