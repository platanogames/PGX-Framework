// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

using UnrealBuildTool;

public class PGXSaveRuntime : ModuleRules
{
	public PGXSaveRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"PGXCoreRuntime",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry"
		});

		// EN: Automation test infrastructure — pulled in whenever the target builds the editor
		//     OR enables dev automation tests explicitly. PGXCoreDeveloper is a DeveloperTool
		//     module hosting the PGX_TEST macro family + FPGXTestUtils. The compound guard
		//     aligns Build.cs with the test files' WITH_DEV_AUTOMATION_TESTS file-level guard:
		//     any non-editor target that flips WithAutomationTests ON also receives the dep,
		//     ensuring tests compile only when their host module is linked. (Property name on
		//     ReadOnlyTargetRules has no `b` prefix in
		//     UE 5.5+: Target.WithAutomationTests, not Target.bWithAutomationTests.)
		// ES: Infraestructura de tests de automation — incluida cuando el target construye el
		//     editor O habilita dev automation tests explicitamente. PGXCoreDeveloper es un
		//     modulo DeveloperTool que aloja la familia de macros PGX_TEST + FPGXTestUtils. El
		//     guard compuesto alinea Build.cs con el guard file-level WITH_DEV_AUTOMATION_TESTS
		//     de los test files: cualquier target non-editor que active WithAutomationTests
		//     tambien recibe la dep, asegurando que los tests solo compilen cuando su modulo host
		//     esta enlazado. (En ReadOnlyTargetRules el property no
		//     lleva prefix `b` en UE 5.5+: Target.WithAutomationTests, no bWithAutomationTests.)
		if (Target.bBuildEditor || Target.WithAutomationTests)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"PGXCoreDeveloper"
			});
		}
	}
}
