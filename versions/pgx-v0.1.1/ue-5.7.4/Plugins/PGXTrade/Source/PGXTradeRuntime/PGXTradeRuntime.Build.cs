// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXTradeRuntime : ModuleRules
{
	public PGXTradeRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine",
			"GameplayTags",
			"PGXCoreRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
