// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXInventoryEditor : ModuleRules
{
	public PGXInventoryEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"PGXCoreRuntime",
			"PGXCoreEditor",
			"PGXInventoryRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry",
			"AssetTools",
			"EditorStyle",
			"InputCore",
			"WorkspaceMenuStructure",
			"GameplayTags"
		});
	}
}
