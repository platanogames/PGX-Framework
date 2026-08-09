// Copyright PGX Framework. All Rights Reserved.

using UnrealBuildTool;

public class PGXAIRuntime : ModuleRules
{
    public PGXAIRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "AIModule",
            "GameplayTags",
            "NavigationSystem",
            "DeveloperSettings",
            "PGXCoreRuntime"
        });
    }
}
