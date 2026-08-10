// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXEditorToolsEditor : ModuleRules
{
    public PGXEditorToolsEditor(ReadOnlyTargetRules Target) : base(Target)
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
            "ToolMenus",
            "PGXCoreRuntime",
            "GameplayTags",
            "PGXCoreEditor"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // EN: Editor internals — only used by .cpp and Private/ widgets
            // ES: Internos de editor — solo usados por .cpp y widgets en Private/
            "AssetTools",
            "PropertyEditor",
            "InputCore",
            "ContentBrowser",
            "DesktopPlatform",
            "RenderCore",
            "WorkspaceMenuStructure",

            // EN: Per-system runtime modules — used by inspector tabs (internal)
            // ES: Modulos runtime por sistema — usados por tabs de inspector (interno)
            "PGXSaveRuntime",
            "PGXGameFlowRuntime",
            "PGXPSORuntime",
            "PGXLoadingRuntime",
            "PGXMGOSRuntime",
            "PGXAudioRuntime",
            "PGXRegistryEditor",

            // EN: PGX Version Control — for System Observer snapshot
            // ES: PGX Version Control — para snapshot del System Observer
            "PGXVersionControlEditor",
            "Settings"
        });
    }
}
