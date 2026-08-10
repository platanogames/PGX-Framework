// Copyright PGX Framework. All Rights Reserved.
using UnrealBuildTool;

public class PGXAbilityRuntime : ModuleRules
{
  public PGXAbilityRuntime(ReadOnlyTargetRules Target) : base(Target)
  {
    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    PublicDependencyModuleNames.AddRange(new string[] {
      "Core", "CoreUObject", "Engine", "DeveloperSettings",
      "GameplayAbilities", "GameplayTags", "GameplayTasks",
      "PGXCoreRuntime"
    });
  }
}
