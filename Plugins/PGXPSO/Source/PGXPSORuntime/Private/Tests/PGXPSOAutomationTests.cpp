// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

// EN: Automation wrappers around UPGXPSOTestUtility helpers. Each wrapper forwards
//     discovered issues and propagates the helper result so failures remain visible.
// ES: Wrappers de automatizacion para UPGXPSOTestUtility. Cada wrapper propaga
//     incidencias y resultado para que los fallos sean visibles.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Misc/AutomationTest.h"
#include "PGXPSOTestUtility.h"
#include "Tags/PGXPSOTags.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PGXPSOAutomationTestsInternal
{
	// EN: Acquire a UWorld from the engine's contexts (PIE > Game > Editor priority). In
	//     headless commandlet mode without an active map this returns nullptr; the wrapper
	//     surfaces a SKIP via AddInfo (test discovered but not exercised) rather than failing
	//     spuriously, since the empirical pass/fail surface is PIE for these tests.
	// ES: Obtener un UWorld desde los WorldContexts del engine. Sin mapa activo retorna nullptr;
	//     el wrapper publica SKIP via AddInfo en vez de fallar espureamente.
	static UWorld* AcquireTestWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		const auto& WorldContexts = GEngine->GetWorldContexts();
		UWorld* PIEWorld = nullptr;
		UWorld* GameWorld = nullptr;
		UWorld* EditorWorld = nullptr;

		for (const FWorldContext& Ctx : WorldContexts)
		{
			UWorld* W = Ctx.World();
			if (!W)
			{
				continue;
			}
			switch (Ctx.WorldType)
			{
			case EWorldType::PIE:    PIEWorld = W; break;
			case EWorldType::Game:   GameWorld = W; break;
			case EWorldType::Editor: EditorWorld = W; break;
			default: break;
			}
		}

		if (PIEWorld)    return PIEWorld;
		if (GameWorld)   return GameWorld;
		return EditorWorld;
	}

	// EN: Resolve the canonical Global PSO context tag. Native tag handle is registered at
	//     module load (UE_DEFINE_GAMEPLAY_TAG) so the lookup is deterministic from first run
	//     (covers the RequestGameplayTag(name, false) failure mode).
	// ES: Resolver el tag de contexto Global de PSO via native tag handle.
	static FGameplayTag GetGlobalContextTag()
	{
		return TAG_PGX_PSO_Context_Global.GetTag();
	}

	// EN: Forward all OutIssues lines via AddInfo so the Automation report shows the BPL
	//     helper's [PASS]/[FAIL]/[INFO] trace inline with the wrapper.
	// ES: Reenviar las lineas de OutIssues via AddInfo para que el reporte de Automation
	//     muestre la traza del BPL helper.
	static void ForwardIssues(FAutomationTestBase& Test, const TArray<FString>& OutIssues)
	{
		for (const FString& Issue : OutIssues)
		{
			Test.AddInfo(Issue);
		}
	}
}

// ============================================================
// Automation coverage — Automation wrappers around UPGXPSOTestUtility BPL helpers
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXPSO_QuickTestAutomationTest,
	"PGX.PSO.QuickTest",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXPSO_QuickTestAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXPSOAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Empirical pass/fail deferred to PIE / M5 Testing Harness."));
		return true;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXPSOTestUtility::RunQuickTest(World, OutIssues);
	PGXPSOAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXPSO_SingleEntryWarmUpAutomationTest,
	"PGX.PSO.SingleEntryWarmUp",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXPSO_SingleEntryWarmUpAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXPSOAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Empirical pass/fail deferred to PIE / M5 Testing Harness."));
		return true;
	}
	const FGameplayTag ContextTag = PGXPSOAutomationTestsInternal::GetGlobalContextTag();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXPSOTestUtility::TestSingleEntryWarmUp(World, OutIssues, ContextTag);
	PGXPSOAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXPSO_BatchWarmUpAutomationTest,
	"PGX.PSO.BatchWarmUp",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXPSO_BatchWarmUpAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXPSOAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Empirical pass/fail deferred to PIE / M5 Testing Harness."));
		return true;
	}
	const FGameplayTag ContextTag = PGXPSOAutomationTestsInternal::GetGlobalContextTag();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXPSOTestUtility::TestBatchWarmUp(World, OutIssues, ContextTag);
	PGXPSOAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXPSO_ContextFilteringAutomationTest,
	"PGX.PSO.ContextFiltering",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXPSO_ContextFilteringAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXPSOAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Empirical pass/fail deferred to PIE / M5 Testing Harness."));
		return true;
	}
	const FGameplayTag ContextTag = PGXPSOAutomationTestsInternal::GetGlobalContextTag();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXPSOTestUtility::TestContextFiltering(World, OutIssues, ContextTag);
	PGXPSOAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXPSO_WarmUpControlAutomationTest,
	"PGX.PSO.WarmUpControl",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXPSO_WarmUpControlAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXPSOAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Empirical pass/fail deferred to PIE / M5 Testing Harness."));
		return true;
	}
	const FGameplayTag ContextTag = PGXPSOAutomationTestsInternal::GetGlobalContextTag();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXPSOTestUtility::TestWarmUpControl(World, OutIssues, ContextTag);
	PGXPSOAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXPSO_StressTestAutomationTest,
	"PGX.PSO.StressTest",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::PerfFilter)

bool FPGXPSO_StressTestAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXPSOAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Empirical pass/fail deferred to PIE / M5 Testing Harness."));
		return true;
	}
	const FGameplayTag ContextTag = PGXPSOAutomationTestsInternal::GetGlobalContextTag();
	const int32 EntryCount = 100;
	TArray<FString> OutIssues;
	const bool bPassed = UPGXPSOTestUtility::RunStressTest(World, OutIssues, ContextTag, EntryCount);
	PGXPSOAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXPSO_SimulateGameSessionAutomationTest,
	"PGX.PSO.SimulateGameSession",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXPSO_SimulateGameSessionAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXPSOAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Empirical pass/fail deferred to PIE / M5 Testing Harness."));
		return true;
	}
	const FGameplayTag ContextTag = PGXPSOAutomationTestsInternal::GetGlobalContextTag();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXPSOTestUtility::SimulateGameSession(World, OutIssues, ContextTag);
	PGXPSOAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

// ============================================================================
// EN: IPGXObservable adoption schema validation test
//     for UPGXPSOWarmUpConfig (single class — direct test, no template needed).
// ES: test de validacion IPGXObservable de
//     UPGXPSOWarmUpConfig (clase unica — test directo, sin template).
// ============================================================================

#include "Observability/PGXObservable.h"
#include "PGXPSOWarmUpConfig.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXPSOWarmUpConfigObservableSchema,
	"PGX.PSO.WarmUpConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXPSOWarmUpConfigObservableSchema::RunTest(const FString& /*Parameters*/)
{
	UPGXPSOWarmUpConfig* Config = NewObject<UPGXPSOWarmUpConfig>(
		GetTransientPackage(), UPGXPSOWarmUpConfig::StaticClass(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("UPGXPSOWarmUpConfig instance"), Config))
	{
		return false;
	}

	const FName SchemaVersion = Config->GetSchemaVersion();
	TestEqual(TEXT("UPGXPSOWarmUpConfig::GetSchemaVersion is 1.0"), SchemaVersion, FName(TEXT("1.0")));

	const FPGXSchemaDescriptor Descriptor = Config->GetSchemaDescriptor();
	TestEqual(TEXT("Schema TypeName matches class"), Descriptor.TypeName, UPGXPSOWarmUpConfig::StaticClass()->GetFName());
	TestEqual(TEXT("Schema SchemaVersion matches"), Descriptor.SchemaVersion, SchemaVersion);
	TestTrue(TEXT("Schema Fields enumerated (>0)"), Descriptor.Fields.Num() > 0);

	const FPGXJsonValue Envelope = Config->ToJson();
	TestFalse(TEXT("ToJson envelope non-empty"), Envelope.IsEmpty());
	TestTrue(TEXT("ToJson envelope contains type field"),
		Envelope.JsonString.Contains(TEXT("\"type\":\"PGXPSOWarmUpConfig\"")));
	TestTrue(TEXT("ToJson envelope contains 1.0 version"),
		Envelope.JsonString.Contains(TEXT("\"version\":\"1.0\"")));

	const FPGXJsonValue EmptyJson;
	const FPGXValidationResult EmptyResult = Config->FromJson(EmptyJson);
	TestFalse(TEXT("FromJson rejects empty payload"), EmptyResult.bValid);

	const FPGXValidationResult OkResult = Config->FromJson(Envelope);
	TestTrue(TEXT("FromJson accepts non-empty envelope"), OkResult.bValid);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
