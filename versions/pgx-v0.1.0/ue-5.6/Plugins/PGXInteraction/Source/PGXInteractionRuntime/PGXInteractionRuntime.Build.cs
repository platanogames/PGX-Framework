// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXInteractionRuntime : ModuleRules
{
	public PGXInteractionRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"UMG",
			"PGXCoreRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
