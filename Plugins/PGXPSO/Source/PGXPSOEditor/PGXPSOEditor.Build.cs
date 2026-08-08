// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
using UnrealBuildTool;

public class PGXPSOEditor : ModuleRules
{
	public PGXPSOEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"Slate",
			"SlateCore",
			"RenderCore",
			"RHI",
			"PGXCoreRuntime",
			"PGXPSORuntime",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"EditorFramework",
			"PGXCoreEditor"
		});
	}
}
