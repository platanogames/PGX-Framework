// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXCameraRuntime : ModuleRules
{
	public PGXCameraRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"PGXCoreRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
