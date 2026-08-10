// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXTradeEditor : ModuleRules
{
	public PGXTradeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"PGXTradeRuntime",
			"PGXCoreRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UnrealEd",
			"EditorFramework",
			"PGXCoreEditor"
		});
	}
}
