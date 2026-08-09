// Copyright PGX Framework. All Rights Reserved.
//
// EN: IMPLEMENT_SIMPLE_AUTOMATION_TEST wrappers around UPGXColonyTestUtility BPL helpers,
//     registered under naming `PGX.Colony.<TestName>` so headless `Automation RunTests
//     PGX.Colony` discovers and runs them. Each wrapper acquires a UWorld via the engine's
//     WorldContexts (PIE / Game / Editor), forwards to the BPL static which uses the
//     bool+OutIssues canonical contract used consistently through its bool+OutIssues contract, surfaces every issue line via FAutomationTestBase::AddInfo,
//     and propagates the helper's bool result. Behavior runtime tests.
//
// ES: Wrappers IMPLEMENT_SIMPLE_AUTOMATION_TEST sobre los BPL helpers de UPGXColonyTestUtility,
//     registrados bajo `PGX.Colony.<TestName>`. Los wrappers mantienen el
//     contrato bool+OutIssues es la superficie estable de estos tests.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Observability/PGXObservable.h"
#include "PGXColonyConfig.h"
#include "PGXColonyTestUtility.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PGXColonyAutomationTestsInternal
{
	// EN: Acquire a UWorld from engine contexts (PIE > Game > Editor priority). Headless
	//     commandlets without a loaded map yield nullptr; wrapper records SKIP via AddInfo
	//     instead of false-failing — empirical pass/fail surface is PIE per Foundation close
	//     criteria; those checks require world-backed validation.
	// ES: Obtener UWorld desde WorldContexts. Sin mapa activo retorna nullptr.
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

	static void ForwardIssues(FAutomationTestBase& Test, const TArray<FString>& OutIssues)
	{
		for (const FString& Issue : OutIssues)
		{
			Test.AddInfo(Issue);
		}
	}
}

// ============================================================
// Behavior runtime tests — Automation wrappers around UPGXColonyTestUtility BPL helpers
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXColony_SubsystemInitializeAutomationTest,
	"PGX.Colony.SubsystemInitialize",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXColony_SubsystemInitializeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXColonyAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Requires PIE / world-backed validation."));
		return true;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXColonyTestUtility::SubsystemInitializeTest(World, OutIssues);
	PGXColonyAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXColony_SurvivorRegisterUnregisterAutomationTest,
	"PGX.Colony.SurvivorRegisterUnregister",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXColony_SurvivorRegisterUnregisterAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXColonyAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Requires PIE / world-backed validation."));
		return true;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXColonyTestUtility::SurvivorRegisterUnregisterTest(World, OutIssues);
	PGXColonyAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXColony_SurvivorRegistrySnapshotAutomationTest,
	"PGX.Colony.SurvivorRegistrySnapshot",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXColony_SurvivorRegistrySnapshotAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXColonyAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Requires PIE / world-backed validation."));
		return true;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXColonyTestUtility::SurvivorRegistrySnapshotTest(World, OutIssues);
	PGXColonyAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXColony_ConfigResolutionAutomationTest,
	"PGX.Colony.ConfigResolution",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXColony_ConfigResolutionAutomationTest::RunTest(const FString& /*Parameters*/)
{
	// EN: ConfigResolutionTest does not require a UWorld — Settings is GetDefault<>(). Pass
	//     a non-null context for API symmetry but tolerate world absence.
	// ES: ConfigResolutionTest no requiere UWorld — Settings es GetDefault<>().
	UWorld* World = PGXColonyAutomationTestsInternal::AcquireTestWorld();
	UObject* Context = World ? static_cast<UObject*>(World) : static_cast<UObject*>(GEngine);
	TArray<FString> OutIssues;
	const bool bPassed = UPGXColonyTestUtility::ConfigResolutionTest(Context, OutIssues);
	PGXColonyAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXColony_NativeTagsRegisteredAutomationTest,
	"PGX.Colony.NativeTagsRegistered",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXColony_NativeTagsRegisteredAutomationTest::RunTest(const FString& /*Parameters*/)
{
	// EN: NativeTagsRegisteredTest does not require a UWorld — gameplay tags resolve at module
	//     load. Pass GEngine as context for symmetry.
	// ES: NativeTagsRegisteredTest no requiere UWorld — los tags resuelven al cargar el modulo.
	UWorld* World = PGXColonyAutomationTestsInternal::AcquireTestWorld();
	UObject* Context = World ? static_cast<UObject*>(World) : static_cast<UObject*>(GEngine);
	TArray<FString> OutIssues;
	const bool bPassed = UPGXColonyTestUtility::NativeTagsRegisteredTest(Context, OutIssues);
	PGXColonyAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXColony_ObservableConfigContractAutomationTest,
	"PGX.Colony.ObservableConfigContract",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXColony_ObservableConfigContractAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TestTrue(TEXT("UPGXColonyConfig implements IPGXObservable"),
		UPGXColonyConfig::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));

	UPGXColonyConfig* Config = NewObject<UPGXColonyConfig>(GetTransientPackage(), UPGXColonyConfig::StaticClass(), NAME_None, RF_Transient);
	TestNotNull(TEXT("Observable config instance"), Config);
	if (!Config)
	{
		return false;
	}

	const FPGXJsonValue Json = Config->ToJson();
	const FString ExpectedType = FString::Printf(TEXT("\"type\":\"%s\""), *Config->GetClass()->GetName());
	TestFalse(TEXT("Observable ToJson returns non-empty envelope"), Json.IsEmpty());
	TestTrue(TEXT("Observable envelope includes class name"), Json.JsonString.Contains(ExpectedType));
	TestTrue(TEXT("Observable envelope includes concrete data"), Json.JsonString.Contains(TEXT("\"MaxSurvivorsPerSettlement\"")));
	TestEqual(TEXT("Observable schema version baseline"), Config->GetSchemaVersion(), UPGXColonyConfig::SchemaVersion);

	const FPGXSchemaDescriptor Descriptor = Config->GetSchemaDescriptor();
	TestEqual(TEXT("Observable descriptor type"), Descriptor.TypeName, UPGXColonyConfig::StaticClass()->GetFName());
	TestTrue(TEXT("Observable descriptor exposes fields"), Descriptor.Fields.Num() >= 6);

	const FPGXValidationResult EmptyResult = Config->FromJson(FPGXJsonValue());
	TestFalse(TEXT("Observable FromJson empty payload visibly fails"), EmptyResult.bValid);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
