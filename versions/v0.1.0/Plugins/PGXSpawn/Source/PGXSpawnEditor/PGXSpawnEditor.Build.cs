// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXSpawnEditor : ModuleRules
{
	public PGXSpawnEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"PGXSpawnRuntime",
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
