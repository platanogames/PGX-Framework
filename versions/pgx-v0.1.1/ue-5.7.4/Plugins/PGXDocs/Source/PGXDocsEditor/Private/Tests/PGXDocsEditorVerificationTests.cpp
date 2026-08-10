// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: Verification [EDITOR] verification tests for PGXDocs (in PGXDocsEditor per the
//     plugin-local test approach). Checks: DocSystem loads +
//     SearchIndex responds. The CRITICAL tab-registration null-deref was already
//     protected by an existing null guard (PGXDocsTabSpawner.cpp guard).
// ES: Tests [EDITOR] Verification para PGXDocs dentro del propio plugin.
//

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "PGXDocSystem.h"
#include "PGXDocSearchIndex.h"

// [EDITOR] Verification — DocSystem loads (Initialize is idempotent; ensures the
// system is up regardless of the live-reload setting that gates StartupModule).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXDocs_VerificationDocSystemEditorTest,
	"PGX.Docs.Verification.DocSystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXDocs_VerificationDocSystemEditorTest::RunTest(const FString& /*Parameters*/)
{
	FDocSystem& DocSystem = FDocSystem::Get();
	DocSystem.Initialize();
	AddInfo(FString::Printf(TEXT("[EDITOR] Docs FDocSystem IsInitialized=%s"),
		DocSystem.IsInitialized() ? TEXT("true") : TEXT("false")));
	TestTrue(TEXT("[EDITOR] Docs FDocSystem loads (IsInitialized after Initialize)"),
		DocSystem.IsInitialized());
	return DocSystem.IsInitialized();
}

// [EDITOR] Verification — SearchIndex responds (a fresh index has 0 entries; reaching
// GetIndexedCount() without crash is the smoke signal).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXDocs_VerificationSearchIndexEditorTest,
	"PGX.Docs.Verification.SearchIndex",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXDocs_VerificationSearchIndexEditorTest::RunTest(const FString& /*Parameters*/)
{
	FDocSearchIndex Index;
	const int32 Count = Index.GetIndexedCount();
	AddInfo(FString::Printf(TEXT("[EDITOR] Docs FDocSearchIndex GetIndexedCount=%d"), Count));
	TestTrue(TEXT("[EDITOR] Docs FDocSearchIndex responds (count >= 0)"), Count >= 0);
	return Count >= 0;
}

#endif // WITH_DEV_AUTOMATION_TESTS
