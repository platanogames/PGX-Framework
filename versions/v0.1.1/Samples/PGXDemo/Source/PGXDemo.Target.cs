// SPDX-License-Identifier: Apache-2.0
using UnrealBuildTool;
public class PGXDemoTarget : TargetRules
{
    public PGXDemoTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("PGXDemo");
    }
}
