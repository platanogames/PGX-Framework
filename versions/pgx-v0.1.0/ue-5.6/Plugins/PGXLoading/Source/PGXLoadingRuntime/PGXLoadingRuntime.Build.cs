// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXLoadingRuntime : ModuleRules
{
    public PGXLoadingRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "PGXCoreRuntime",
            "GameplayTags",
            "DeveloperSettings",
            "RenderCore",
            "UMG",
            "Slate",
            "SlateCore",
            "MediaAssets"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "AssetRegistry"
        });
    }
}
