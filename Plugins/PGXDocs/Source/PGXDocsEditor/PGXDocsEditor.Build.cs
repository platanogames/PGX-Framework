// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXDocsEditor : ModuleRules
{
	public PGXDocsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"Slate",
			"SlateCore",
			"EditorFramework",
			"PGXDocs"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// EN: Editor internals — only used by .cpp files
			// ES: Internos de editor — solo usados por archivos .cpp
			"ToolMenus",
			"InputCore",
			"DesktopPlatform",
			"DirectoryWatcher",
			"WorkspaceMenuStructure",
			"Json",              // EN: For FJsonObjectConverter / ES: Para FJsonObjectConverter
			"JsonUtilities",     // EN: For UStruct serialization / ES: Para serializacion UStruct
			"PGXCoreRuntime",    // EN: For FPGXLogEntry / ES: Para FPGXLogEntry
			"PGXCoreEditor"      // EN: For SPGXPremiumShell and PGXEditorStyle / ES: Para SPGXPremiumShell y PGXEditorStyle
		});
	}
}
