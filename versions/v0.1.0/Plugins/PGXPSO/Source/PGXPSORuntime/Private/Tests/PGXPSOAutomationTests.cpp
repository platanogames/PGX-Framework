// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

// EN: Automation wrappers around UPGXPSOTestUtility helpers. Each wrapper forwards
//     discovered issues and propagates the helper result so failures remain visible.
// ES: Wrappers de automatizacion para UPGXPSOTestUtility. Cada wrapper propaga
//     incidencias y resultado para que los fallos sean visibles.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Misc/AutomationTest.h"
#include "PGXPSOSubsystem.h"
#include "PGXPSOTestUtility.h"
#include "PGXPSOWarmUpConfig.h"
#include "Tags/PGXPSOTags.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PGXPSOAutomationTestsInternal
{
	struct FScopedGameInstanceFixture
	{
		explicit FScopedGameInstanceFixture(FAutomationTestBase& InTest)
			: Test(InTest)
		{
			if (!GEngine)
			{
				Test.AddError(TEXT("PGXPSO automation setup failed: engine is unavailable."));
				return;
			}

			GameInstance = NewObject<UGameInstance>(GEngine, UGameInstance::StaticClass(), NAME_None, RF_Transient);
			if (!GameInstance)
			{
				Test.AddError(TEXT("PGXPSO automation setup failed: could not create transient GameInstance."));
				return;
			}

			GameInstance->AddToRoot();
			GameInstance->InitializeStandalone(TEXT("PGXPSOAutomationWorld"));
			if (!GameInstance->GetWorld())
			{
				Test.AddError(TEXT("PGXPSO automation setup failed: standalone GameInstance has no World."));
				return;
			}

			UPGXPSOSubsystem* PSO = GameInstance->GetSubsystem<UPGXPSOSubsystem>();
			if (!PSO)
			{
				Test.AddError(TEXT("PGXPSO automation setup failed: UPGXPSOSubsystem missing."));
				return;
			}

			Config = NewObject<UPGXPSOWarmUpConfig>(
				GetTransientPackage(),
				UPGXPSOWarmUpConfig::StaticClass(),
				FName(TEXT("PGXPSOAutomationConfig")),
				RF_Transient);
			if (!Config)
			{
				Test.AddError(TEXT("PGXPSO automation setup failed: could not create transient PSO config."));
				return;
			}

			Config->bSaveCacheAfterWarmUp = false;
			Config->BatchSize = 1;
			Config->BatchDelaySeconds = 60.0f;
			FPGXPSOEntry Entry;
			Entry.ContextTag = TAG_PGX_PSO_Context_Global.GetTag();
			Entry.Label = TEXT("PGXPSOAutomationSyntheticEntry");
			Config->Entries.Add(Entry);
			PSO->InjectTestConfigForTesting(Config);
		}

		~FScopedGameInstanceFixture()
		{
			Shutdown();
		}

		void Shutdown()
		{
			if (!GameInstance || bShutdown)
			{
				return;
			}
			GameInstance->Shutdown();
			bShutdown = true;
			if (GameInstance->IsRooted())
			{
				GameInstance->RemoveFromRoot();
			}
		}

		FScopedGameInstanceFixture(const FScopedGameInstanceFixture&) = delete;
		FScopedGameInstanceFixture& operator=(const FScopedGameInstanceFixture&) = delete;

		UWorld* GetWorld() const { return GameInstance ? GameInstance->GetWorld() : nullptr; }

	private:
		FAutomationTestBase& Test;
		UGameInstance* GameInstance = nullptr;
		UPGXPSOWarmUpConfig* Config = nullptr;
		bool bShutdown = false;
	};

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
			if (Issue.StartsWith(TEXT("[FAIL]")) || Issue.Contains(TEXT("FAIL:")))
			{
				Test.AddError(Issue);
			}
			else
			{
				Test.AddInfo(Issue);
			}
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
	PGXPSOAutomationTestsInternal::FScopedGameInstanceFixture Fixture(*this);
	UWorld* World = Fixture.GetWorld();
	if (!World)
	{
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
	PGXPSOAutomationTestsInternal::FScopedGameInstanceFixture Fixture(*this);
	UWorld* World = Fixture.GetWorld();
	if (!World)
	{
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
	PGXPSOAutomationTestsInternal::FScopedGameInstanceFixture Fixture(*this);
	UWorld* World = Fixture.GetWorld();
	if (!World)
	{
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
	PGXPSOAutomationTestsInternal::FScopedGameInstanceFixture Fixture(*this);
	UWorld* World = Fixture.GetWorld();
	if (!World)
	{
		return true;
	}
	const FGameplayTag ContextTag = TAG_PGX_PSO_Context_Menu.GetTag();
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
	PGXPSOAutomationTestsInternal::FScopedGameInstanceFixture Fixture(*this);
	UWorld* World = Fixture.GetWorld();
	if (!World)
	{
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
	PGXPSOAutomationTestsInternal::FScopedGameInstanceFixture Fixture(*this);
	UWorld* World = Fixture.GetWorld();
	if (!World)
	{
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
	PGXPSOAutomationTestsInternal::FScopedGameInstanceFixture Fixture(*this);
	UWorld* World = Fixture.GetWorld();
	if (!World)
	{
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
