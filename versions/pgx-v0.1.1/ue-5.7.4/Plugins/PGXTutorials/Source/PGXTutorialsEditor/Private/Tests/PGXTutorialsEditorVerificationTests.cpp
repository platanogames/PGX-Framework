// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: Verification [EDITOR] verification tests for PGXTutorials. Checks the
//     TutorialActionExecutor responds and starts clean after ResetTracking.
// ES: Tests [EDITOR] Verification para PGXTutorials.
//

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "PGXTutorialActionExecutor.h"

// [EDITOR] Verification — ActionExecutor responds; tracking is clean after reset.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTutorials_VerificationActionExecutorEditorTest,
	"PGX.Tutorials.Verification.ActionExecutor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXTutorials_VerificationActionExecutorEditorTest::RunTest(const FString& /*Parameters*/)
{
	FPGXTutorialActionExecutor::ResetTracking();

	const int32 AssetCount = FPGXTutorialActionExecutor::GetCreatedAssetCount();
	const int32 FolderCount = FPGXTutorialActionExecutor::GetCreatedFolderCount();
	const bool bClean = AssetCount == 0 && FolderCount == 0
		&& !FPGXTutorialActionExecutor::HasCreatedAssets();

	AddInfo(FString::Printf(
		TEXT("[EDITOR] Tutorials ActionExecutor after ResetTracking: assets=%d folders=%d"),
		AssetCount, FolderCount));
	TestTrue(TEXT("[EDITOR] Tutorials ActionExecutor responds + clean after ResetTracking"), bClean);
	return bClean;
}

#endif // WITH_DEV_AUTOMATION_TESTS
