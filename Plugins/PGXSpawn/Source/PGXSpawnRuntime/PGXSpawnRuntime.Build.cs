// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXSpawnRuntime : ModuleRules
{
    public PGXSpawnRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "DeveloperSettings",
            "Engine",
            "GameplayTags",
            "StructUtils",
            "PGXCoreRuntime"
        });
    }
}
