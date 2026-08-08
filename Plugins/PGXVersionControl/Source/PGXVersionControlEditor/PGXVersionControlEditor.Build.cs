// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXVersionControlEditor : ModuleRules
{
    public PGXVersionControlEditor(ReadOnlyTargetRules Target) : base(Target)
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
            "InputCore",
            "PGXCoreRuntime",
            "PGXCoreEditor",
            "EditorSubsystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // EN: Source control API — used only by subsystem .cpp
            // ES: API de source control — usada solo por subsystem .cpp
            "SourceControl",

            // EN: Settings infrastructure / ES: Infraestructura de settings
            "DeveloperSettings",

            // EN: JSON serialization for changelist persistence
            // ES: Serializacion JSON para persistencia de changelists
            "Json",
            "JsonUtilities",

            // EN: NomadTab registration / ES: Registro de NomadTab
            "ToolMenus",
            "WorkspaceMenuStructure",

            // EN: ISettingsModule for "Open Settings" in inspector
            // ES: ISettingsModule para "Open Settings" en inspector
            "Settings",

            // EN: FPlatformApplicationMisc::ClipboardCopy implementation
            // ES: Implementacion de FPlatformApplicationMisc::ClipboardCopy
            "ApplicationCore"
        });
    }
}
