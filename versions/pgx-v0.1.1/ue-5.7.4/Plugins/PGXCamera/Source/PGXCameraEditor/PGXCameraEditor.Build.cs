// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXCameraEditor : ModuleRules
{
	public PGXCameraEditor(ReadOnlyTargetRules Target) : base(Target)
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
			"PGXCameraRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry",
			"EditorStyle",
			"InputCore",
			"WorkspaceMenuStructure"
		});
	}
}
