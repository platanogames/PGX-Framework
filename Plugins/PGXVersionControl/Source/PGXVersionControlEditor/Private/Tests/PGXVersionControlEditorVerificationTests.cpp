// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: Verification [EDITOR] verification tests for PGXVersionControl. Lives in the
//     plugin's own module so it can include the private FPGXChangelistStore /
//     FPGXCommitValidator headers. Checks: ChangelistStore operates +
//     CommitValidator responds. Uses a temp save-path override so it never
//     touches the editor's real changelist store.
// ES: Tests [EDITOR] Verification para PGXVersionControl (accede a headers private
//     del propio modulo; usa path temporal para no tocar el store real).
//

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

#include "PGXChangelistStore.h"
#include "PGXCommitValidator.h"

// [EDITOR] Verification — ChangelistStore operates (temp override; EnsureDefault
// yields at least the default changelist).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVersionControl_VerificationChangelistStoreEditorTest,
	"PGX.VersionControl.Verification.ChangelistStore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_VerificationChangelistStoreEditorTest::RunTest(const FString& /*Parameters*/)
{
	const FString TempPath = FPaths::ProjectSavedDir() / TEXT("PGX/Verification_VC_changelists.json");
	FPGXChangelistStore Store(TempPath);
	Store.Load();

	const int32 Count = Store.GetChangelists().Num();
	AddInfo(FString::Printf(TEXT("[EDITOR] VersionControl ChangelistStore changelists=%d"), Count));
	TestTrue(TEXT("[EDITOR] VersionControl ChangelistStore operates (>=1 after EnsureDefault)"), Count >= 1);
	return Count >= 1;
}

// [EDITOR] Verification — CommitValidator responds without crash (empty input path).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXVersionControl_VerificationCommitValidatorEditorTest,
	"PGX.VersionControl.Verification.CommitValidator",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXVersionControl_VerificationCommitValidatorEditorTest::RunTest(const FString& /*Parameters*/)
{
	FPGXCommitValidator Validator;
	const TArray<FString> EmptyPaths;
	const TArray<FPGXValidationIssue> Issues = Validator.Validate(EmptyPaths);

	AddInfo(FString::Printf(TEXT("[EDITOR] VersionControl CommitValidator returned %d issues for empty input"), Issues.Num()));
	TestTrue(TEXT("[EDITOR] VersionControl CommitValidator responds without crash"), true);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
