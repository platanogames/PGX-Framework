// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXGameFlowRuntime : ModuleRules
{
	public PGXGameFlowRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"PGXCoreRuntime",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry"
		});

		// EN: Automation test infrastructure — editor builds only.
		//     PGXCoreDeveloper hosts the PGX_TEST macro family + FPGXTestUtils.
		// ES: Infraestructura de tests de automation — solo builds editor.
		//     PGXCoreDeveloper aloja macros PGX_TEST + FPGXTestUtils.
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"PGXCoreDeveloper"
			});
		}
	}
}
