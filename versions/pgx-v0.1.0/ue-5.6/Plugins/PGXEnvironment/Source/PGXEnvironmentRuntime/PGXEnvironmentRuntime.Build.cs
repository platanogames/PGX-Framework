// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXEnvironmentRuntime : ModuleRules
{
	public PGXEnvironmentRuntime(ReadOnlyTargetRules Target) : base(Target)
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
