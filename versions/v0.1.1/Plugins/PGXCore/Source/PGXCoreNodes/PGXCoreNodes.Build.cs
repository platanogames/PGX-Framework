// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXCoreNodes : ModuleRules
{
	public PGXCoreNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"BlueprintGraph"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"SlateCore",
			"KismetCompiler",
			"UnrealEd",
			"AssetRegistry",
			"PGXCoreRuntime"
		});
	}
}
