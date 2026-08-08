// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
using UnrealBuildTool;

public class PGXPSORuntime : ModuleRules
{
	public PGXPSORuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"RenderCore",
			"RHI",
			"PGXCoreRuntime",
			"GameplayTags",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry",
			"Json",
			"JsonUtilities"
		});
	}
}
