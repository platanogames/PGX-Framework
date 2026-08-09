// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXInputRuntime : ModuleRules
{
	public PGXInputRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"EnhancedInput",
			"DeveloperSettings",
			"PGXCoreRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
