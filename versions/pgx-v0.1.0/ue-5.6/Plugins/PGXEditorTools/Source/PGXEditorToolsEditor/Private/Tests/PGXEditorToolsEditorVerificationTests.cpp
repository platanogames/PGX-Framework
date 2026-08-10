// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: [EDITOR] verification test for PGXEditorTools. The 4
//     auditor/validator/dashboard classes are unimplemented UObject shells
//     and this verifies they instantiate without crashing. It is a smoke test,
//     not a feature test.
// ES: Test [EDITOR] para PGXEditorTools: los 4 shells UObject sin
//     implementar instancian sin crash.
//
// The editor build is the compile gate for these UObject smoke tests.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "UObject/Package.h"

#include "PGXAssetAuditor.h"
#include "PGXBlueprintAuditor.h"
#include "PGXLevelValidator.h"
#include "PGXDashboard.h"

// [EDITOR] The 4 shell UObjects instantiate without crash.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXEditorTools_ShellInstantiationEditorTest,
	"PGX.EditorTools.ShellInstantiation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXEditorTools_ShellInstantiationEditorTest::RunTest(const FString& /*Parameters*/)
{
	UPGXAssetAuditor* AssetAuditor = NewObject<UPGXAssetAuditor>(GetTransientPackage());
	UPGXBlueprintAuditor* BlueprintAuditor = NewObject<UPGXBlueprintAuditor>(GetTransientPackage());
	UPGXLevelValidator* LevelValidator = NewObject<UPGXLevelValidator>(GetTransientPackage());
	UPGXDashboard* Dashboard = NewObject<UPGXDashboard>(GetTransientPackage());

	const bool bAll = AssetAuditor != nullptr
		&& BlueprintAuditor != nullptr
		&& LevelValidator != nullptr
		&& Dashboard != nullptr;

	AddInfo(TEXT("[EDITOR] EditorTools 4 shell UObjects (AssetAuditor/BlueprintAuditor/LevelValidator/Dashboard) instantiated."));
	TestTrue(TEXT("[EDITOR] EditorTools 4 shell UObjects instantiate without crash"), bAll);
	return bAll;
}

#endif // WITH_DEV_AUTOMATION_TESTS
