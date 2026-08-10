// SPDX-License-Identifier: Apache-2.0
using UnrealBuildTool;
public class PGXDemoEditor : ModuleRules
{
    public PGXDemoEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "UnrealEd", "AssetRegistry", "Json", "Projects", "GameplayTags",
            "PGXDemo", "PGXCoreRuntime", "PGXGameFlowRuntime", "PGXSaveRuntime", "PGXInputRuntime"
        });
        AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");
    }
}
