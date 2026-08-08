// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXRegistryEditor : ModuleRules
{
	public PGXRegistryEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		// EN: Include path only (no link) — circular dep: PGXCoreEditor -> PGXRegistryEditor
		// ES: Solo include path (sin link) — dep circular: PGXCoreEditor -> PGXRegistryEditor
		// Tokens (constexpr/inline) work fine, but SNew(SPGXWidget) is NOT allowed here.
		PrivateIncludePathModuleNames.Add("PGXCoreEditor");

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"PGXCoreRuntime",

			// EN: Editor / ES: Editor
			"Slate",
			"SlateCore",
			"UnrealEd",
			"EditorStyle",
			"ToolMenus",
			"WorkspaceMenuStructure",
			"ContentBrowser",

			// EN: Data / ES: Datos
			"AssetRegistry",
			"GameplayTags",
			"Json",
			"JsonUtilities",
			"DeveloperSettings",
			"InputCore"
		});
	}
}
