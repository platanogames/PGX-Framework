// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: [EDITOR] verification tests for PGXScaffold. The automation tests live
//     in this plugin so they can access
//     the plugin's own public API without cross-module private access.
//     Checks: TemplateRegistry has built-in templates + ProjectAnalyzer responds.
// ES: Tests de verificacion [EDITOR] para PGXScaffold, dentro del propio
//     plugin para acceder a su API sin cross-module private access.
//
// UBT and CI are the compile gates for these editor-facing API checks.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Core/PGXProjectAnalyzer.h"
#include "Core/PGXScaffoldTemplateRegistry.h"
#include "Core/PGXScaffoldTypes.h"
#include "Templates/PGXBuiltInTemplates.h"

// [EDITOR] Built-in TemplateRegistry is populated (StartupModule
// registers the built-in templates; RegisterAll is called defensively in case
// this test runs before StartupModule populated the singleton).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXScaffold_TemplateRegistryEditorTest,
	"PGX.Scaffold.TemplateRegistry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXScaffold_TemplateRegistryEditorTest::RunTest(const FString& /*Parameters*/)
{
	FPGXScaffoldTemplateRegistry& Registry = FPGXScaffoldTemplateRegistry::Get();
	if (Registry.GetAllTemplates().Num() < 4)
	{
		AddInfo(FString::Printf(
			TEXT("[EDITOR] Registry had %d templates before RegisterAll (StartupModule may not have run in this context)."),
			Registry.GetAllTemplates().Num()));
		FPGXBuiltInTemplates::RegisterAll(Registry);
	}

	const int32 Count = Registry.GetAllTemplates().Num();
	AddInfo(FString::Printf(TEXT("[EDITOR] Scaffold TemplateRegistry template count = %d"), Count));
	TestTrue(TEXT("[EDITOR] Scaffold TemplateRegistry has >= 4 built-in templates"), Count >= 4);
	return Count >= 4;
}

// [EDITOR] ProjectAnalyzer responds without crashing. Reaching the
// assertion after Analyze() is itself the no-crash smoke signal.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXScaffold_ProjectAnalyzerEditorTest,
	"PGX.Scaffold.ProjectAnalyzer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXScaffold_ProjectAnalyzerEditorTest::RunTest(const FString& /*Parameters*/)
{
	FPGXProjectAnalyzer Analyzer;
	const FPGXScaffoldProjectInfo Info = Analyzer.Analyze();
	(void)Info;

	AddInfo(TEXT("[EDITOR] Scaffold ProjectAnalyzer.Analyze() completed without crash."));
	TestTrue(TEXT("[EDITOR] Scaffold ProjectAnalyzer responded"), true);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
