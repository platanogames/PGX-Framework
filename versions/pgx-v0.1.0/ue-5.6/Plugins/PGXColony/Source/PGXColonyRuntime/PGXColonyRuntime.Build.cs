// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXColonyRuntime : ModuleRules
{
	public PGXColonyRuntime(ReadOnlyTargetRules Target) : base(Target)
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
	}
}
