// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXAbilityEditor : ModuleRules
{
	public PGXAbilityEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// EN: Public deps — runtime types exposed in Editor headers (subsystem refs, facades).
		// ES: Deps Public — tipos Runtime expuestos en headers Editor.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"PGXAbilityRuntime",
			"PGXCoreRuntime"
		});

		// EN: Private deps — Slate widgets, tokens, workspace, only used in .cpp.
		// ES: Deps Private — widgets Slate, tokens, workspace, solo .cpp.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"PGXCoreEditor",
			"Slate",
			"SlateCore",
			"WorkspaceMenuStructure",
			"GameplayTags"
		});
	}
}
