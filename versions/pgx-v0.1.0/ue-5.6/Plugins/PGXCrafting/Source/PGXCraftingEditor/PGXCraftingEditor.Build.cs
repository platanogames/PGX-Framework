// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXCraftingEditor : ModuleRules
{
	public PGXCraftingEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"PGXCraftingRuntime",
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
