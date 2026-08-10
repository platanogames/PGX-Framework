// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXAudioRuntime : ModuleRules
{
	public PGXAudioRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"AudioModulation",
			"PGXCoreRuntime",
			"DeveloperSettings",
			"AudioMixer"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry"
		});
	}
}
