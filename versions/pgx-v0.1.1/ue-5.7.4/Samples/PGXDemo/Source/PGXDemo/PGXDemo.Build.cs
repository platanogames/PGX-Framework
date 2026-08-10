// SPDX-License-Identifier: Apache-2.0
using UnrealBuildTool;
public class PGXDemo : ModuleRules
{
    public PGXDemo(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "GameplayTags",
            "PGXCoreRuntime", "PGXGameFlowRuntime", "PGXSaveRuntime", "PGXInputRuntime"
        });
    }
}
