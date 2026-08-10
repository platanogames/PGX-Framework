// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXCoreDeveloper : ModuleRules
{
	public PGXCoreDeveloper(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"PGXCoreRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
