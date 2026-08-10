// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXVehiclesEditor : ModuleRules
{
	public PGXVehiclesEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"PGXVehiclesRuntime",
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
