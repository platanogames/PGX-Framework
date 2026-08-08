// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXLoadingEditor : ModuleRules
{
	public PGXLoadingEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"PGXLoadingRuntime",
			"PGXCoreRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"PGXCoreEditor",
			"Slate",
			"SlateCore",
			"WorkspaceMenuStructure",
			"AssetRegistry",
			"AssetTools",
			"DeveloperSettings",
			"GameplayTags",
			"UMG"
		});
	}
}
