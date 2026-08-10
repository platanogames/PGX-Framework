// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXUIRuntime : ModuleRules
{
	public PGXUIRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"UMG",
			"Slate",
			"SlateCore",
			"PGXCoreRuntime",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
