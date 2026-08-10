// SPDX-License-Identifier: Apache-2.0
using UnrealBuildTool;
public class PGXDemoEditorTarget : TargetRules
{
    public PGXDemoEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.AddRange(new[] { "PGXDemo", "PGXDemoEditor" });
    }
}
