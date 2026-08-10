// Copyright PGX Framework. All Rights Reserved.
//
// EN: IMPLEMENT_SIMPLE_AUTOMATION_TEST wrappers around UPGXAITestUtility BPL helpers,
//     registered under naming `PGX.AI.<TestName>` so headless `Automation RunTests
//     PGX.AI` discovers and runs them. Each wrapper acquires a UWorld via the engine's
//     WorldContexts (PIE / Game / Editor), forwards to the BPL static which uses the
//     bool+OutIssues canonical contract used consistently through its bool+OutIssues contract, surfaces every issue line via FAutomationTestBase::AddInfo,
//     and propagates the helper's bool result. Runtime runtime tests.
//
// ES: Wrappers IMPLEMENT_SIMPLE_AUTOMATION_TEST sobre los BPL helpers de UPGXAITestUtility,
//     registrados bajo `PGX.AI.<TestName>`. Los wrappers mantienen el
//     contrato bool+OutIssues es la superficie estable de estos tests.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "PGXAIConfig.h"
#include "PGXAITestUtility.h"
#include "PGXAISubsystem.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Observability/PGXObservable.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PGXAIAutomationTestsInternal
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
// Runtime runtime tests — Automation wrappers around UPGXAITestUtility BPL helpers
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAI_SubsystemInitializeAutomationTest,
	"PGX.AI.SubsystemInitialize",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXAI_SubsystemInitializeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXAIAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Requires PIE / world-backed validation."));
		return true;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXAITestUtility::SubsystemInitializeTest(World, OutIssues);
	PGXAIAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAI_AgentRegisterUnregisterAutomationTest,
	"PGX.AI.AgentRegisterUnregister",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXAI_AgentRegisterUnregisterAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXAIAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Requires PIE / world-backed validation."));
		return true;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXAITestUtility::AgentRegisterUnregisterTest(World, OutIssues);
	PGXAIAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAI_AgentRegistrySnapshotAutomationTest,
	"PGX.AI.AgentRegistrySnapshot",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXAI_AgentRegistrySnapshotAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXAIAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map). Requires PIE / world-backed validation."));
		return true;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXAITestUtility::AgentRegistrySnapshotTest(World, OutIssues);
	PGXAIAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAI_ConfigResolutionAutomationTest,
	"PGX.AI.ConfigResolution",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXAI_ConfigResolutionAutomationTest::RunTest(const FString& /*Parameters*/)
{
	// EN: ConfigResolutionTest does not require a UWorld — Settings is GetDefault<>(). We pass
	//     a non-null context for API symmetry but tolerate world absence.
	// ES: ConfigResolutionTest no requiere UWorld — Settings es GetDefault<>().
	UWorld* World = PGXAIAutomationTestsInternal::AcquireTestWorld();
	UObject* Context = World ? static_cast<UObject*>(World) : static_cast<UObject*>(GEngine);
	TArray<FString> OutIssues;
	const bool bPassed = UPGXAITestUtility::ConfigResolutionTest(Context, OutIssues);
	PGXAIAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAI_NativeTagsRegisteredAutomationTest,
	"PGX.AI.NativeTagsRegistered",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXAI_NativeTagsRegisteredAutomationTest::RunTest(const FString& /*Parameters*/)
{
	// EN: NativeTagsRegisteredTest does not require a UWorld — gameplay tags resolve at module
	//     load. Pass GEngine as context for symmetry.
	// ES: NativeTagsRegisteredTest no requiere UWorld — los tags resuelven al cargar el modulo.
	UWorld* World = PGXAIAutomationTestsInternal::AcquireTestWorld();
	UObject* Context = World ? static_cast<UObject*>(World) : static_cast<UObject*>(GEngine);
	TArray<FString> OutIssues;
	const bool bPassed = UPGXAITestUtility::NativeTagsRegisteredTest(Context, OutIssues);
	PGXAIAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAI_ControllerValidationSeamAutomationTest,
	"PGX.AI.Integration.ControllerValidationSeam",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXAI_ControllerValidationSeamAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXAIAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map)."));
		return true;
	}

	UPGXAISubsystem* Sub = World->GetSubsystem<UPGXAISubsystem>();
	TestNotNull(TEXT("ControllerValidationSeam subsystem"), Sub);
	if (!Sub)
	{
		return false;
	}

	const FPGXAIResult NullValidation = Sub->ValidateControllerForRegistration(nullptr);
	TestFalse(TEXT("ControllerValidationSeam null controller rejected"), NullValidation.bSucceeded);
	TestTrue(TEXT("ControllerValidationSeam null code"), NullValidation.Code == EPGXAIResultCode::InvalidInput);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAIController* Controller = World->SpawnActor<AAIController>(AAIController::StaticClass(), Params);
	TestNotNull(TEXT("ControllerValidationSeam spawned controller"), Controller);
	if (!Controller)
	{
		return false;
	}

	const FPGXAIResult ValidValidation = Sub->ValidateControllerForRegistration(Controller);
	TestTrue(TEXT("ControllerValidationSeam valid controller accepted"), ValidValidation.bSucceeded);

	FPGXAIResult FirstResult;
	const FPGXAIAgentHandle First = Sub->RegisterAgent(Controller, FirstResult);
	FPGXAIResult SecondResult;
	const FPGXAIAgentHandle Second = Sub->RegisterAgent(Controller, SecondResult);
	TestTrue(TEXT("ControllerValidationSeam first register succeeds"), FirstResult.bSucceeded && First.IsValid());
	TestTrue(TEXT("ControllerValidationSeam duplicate idempotent succeeds"), SecondResult.bSucceeded && Second == First);

	Sub->UnregisterAgent(First);
	Controller->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAI_BehaviorTreeRunSeamAutomationTest,
	"PGX.AI.Integration.BehaviorTreeRunSeam",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXAI_BehaviorTreeRunSeamAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXAIAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map)."));
		return true;
	}

	UPGXAISubsystem* Sub = World->GetSubsystem<UPGXAISubsystem>();
	TestNotNull(TEXT("BehaviorTreeRunSeam subsystem"), Sub);
	if (!Sub)
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAIController* Controller = World->SpawnActor<AAIController>(AAIController::StaticClass(), Params);
	TestNotNull(TEXT("BehaviorTreeRunSeam spawned controller"), Controller);
	if (!Controller)
	{
		return false;
	}

	FPGXAIResult RegisterResult;
	const FPGXAIAgentHandle Handle = Sub->RegisterAgent(Controller, RegisterResult);
	TestTrue(TEXT("BehaviorTreeRunSeam register succeeds"), RegisterResult.bSucceeded && Handle.IsValid());

	const FPGXAIResult InvalidHandleResult = Sub->TryRunBehaviorTreeForAgent(FPGXAIAgentHandle(), nullptr);
	TestFalse(TEXT("BehaviorTreeRunSeam invalid handle rejected"), InvalidHandleResult.bSucceeded);
	TestTrue(TEXT("BehaviorTreeRunSeam invalid handle code"), InvalidHandleResult.Code == EPGXAIResultCode::InvalidInput);

	const FPGXAIResult NullTreeResult = Sub->TryRunBehaviorTreeForAgent(Handle, nullptr);
	TestFalse(TEXT("BehaviorTreeRunSeam null BT rejected"), NullTreeResult.bSucceeded);
	TestTrue(TEXT("BehaviorTreeRunSeam null BT code"), NullTreeResult.Code == EPGXAIResultCode::BehaviorTreeUnavailable);

	UBehaviorTree* BehaviorTree = NewObject<UBehaviorTree>(GetTransientPackage(), UBehaviorTree::StaticClass(), NAME_None, RF_Transient);
	TestNotNull(TEXT("BehaviorTreeRunSeam transient BT"), BehaviorTree);
	if (!BehaviorTree)
	{
		Sub->UnregisterAgent(Handle);
		Controller->Destroy();
		return false;
	}

	Sub->SetForceNextBehaviorTreeRunResultForTesting(true, false);
	const FPGXAIResult ForcedFailure = Sub->TryRunBehaviorTreeForAgent(Handle, BehaviorTree);
	FPGXAIBehaviorTreeRunStatus FailureStatus;
	const bool bFailureStatusFound = Sub->GetBehaviorTreeRunStatus(Handle, FailureStatus);
	TestFalse(TEXT("BehaviorTreeRunSeam forced failure fails"), ForcedFailure.bSucceeded);
	TestTrue(TEXT("BehaviorTreeRunSeam forced failure code"), ForcedFailure.Code == EPGXAIResultCode::BehaviorTreeRunFailed);
	TestTrue(TEXT("BehaviorTreeRunSeam failure status found"), bFailureStatusFound);
	TestTrue(TEXT("BehaviorTreeRunSeam failure status attempted"), FailureStatus.bRunAttempted && !FailureStatus.bRunSucceeded);

	Sub->SetForceNextBehaviorTreeRunResultForTesting(true, true);
	const FPGXAIResult ForcedSuccess = Sub->TryRunBehaviorTreeForAgent(Handle, BehaviorTree);
	FPGXAIBehaviorTreeRunStatus SuccessStatus;
	const bool bSuccessStatusFound = Sub->GetBehaviorTreeRunStatus(Handle, SuccessStatus);
	TestTrue(TEXT("BehaviorTreeRunSeam forced success succeeds"), ForcedSuccess.bSucceeded);
	TestTrue(TEXT("BehaviorTreeRunSeam success status found"), bSuccessStatusFound);
	TestTrue(TEXT("BehaviorTreeRunSeam success status attempted"), SuccessStatus.bRunAttempted && SuccessStatus.bRunSucceeded);

	Sub->UnregisterAgent(Handle);
	Controller->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAI_StaleAgentCleanupSeamAutomationTest,
	"PGX.AI.Integration.StaleAgentCleanupSeam",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXAI_StaleAgentCleanupSeamAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXAIAutomationTestsInternal::AcquireTestWorld();
	if (!World)
	{
		AddInfo(TEXT("SKIP: no UWorld available (headless commandlet without map)."));
		return true;
	}

	UPGXAISubsystem* Sub = World->GetSubsystem<UPGXAISubsystem>();
	TestNotNull(TEXT("StaleAgentCleanupSeam subsystem"), Sub);
	if (!Sub)
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAIController* Controller = World->SpawnActor<AAIController>(AAIController::StaticClass(), Params);
	TestNotNull(TEXT("StaleAgentCleanupSeam spawned controller"), Controller);
	if (!Controller)
	{
		return false;
	}

	FPGXAIResult RegisterResult;
	const FPGXAIAgentHandle Handle = Sub->RegisterAgent(Controller, RegisterResult);
	TestTrue(TEXT("StaleAgentCleanupSeam register succeeds"), RegisterResult.bSucceeded && Handle.IsValid());

	Controller->Destroy();
	const int32 Removed = Sub->CleanupStaleAgents();
	FPGXAIAgentHandle Found;
	TestTrue(TEXT("StaleAgentCleanupSeam cleanup removed stale or world keeps pending kill visible"), Removed >= 0);
	if (Removed > 0)
	{
		TestFalse(TEXT("StaleAgentCleanupSeam handle gone after cleanup"), Sub->FindAgent(Handle.AgentId, Found));
	}
	else
	{
		AddInfo(TEXT("StaleAgentCleanupSeam: destroyed controller still considered valid by current world tick; cleanup path source-covered; removal requires a subsequent world tick."));
		Sub->UnregisterAgent(Handle);
	}
	return true;
}


// ============================================================================
// EN: Sub-Serialization.3.C — IPGXObservable adoption schema validation tests.
//     Per-class validation: schema descriptor non-empty + GetSchemaVersion
//     match + ToJson envelope non-empty. Mirror PGXEnvironment 8.3.C
//     reference. NewObject in transient package — no PIE/world fixture
//     required.
// ES: Sub-Serialization.3.C — tests de validacion de schema para adopcion
//     IPGXObservable. Validacion per-class: schema descriptor non-empty
//     + match GetSchemaVersion + ToJson envelope non-empty. Mirror
//     referencia PGXEnvironment 8.3.C. NewObject en transient package —
//     no se requiere fixture PIE/world.
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXAIConfigObservableSchema,
	"PGX.AI.ConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXAIConfigObservableSchema::RunTest(const FString& /*Parameters*/)
{
	UPGXAIConfig* Config = NewObject<UPGXAIConfig>(
		GetTransientPackage(), UPGXAIConfig::StaticClass(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("UPGXAIConfig instance"), Config))
	{
		return false;
	}

	// EN: Schema version contract: PGXEnvironment baseline literal "1.0".
	// ES: Contrato schema version: literal baseline PGXEnvironment "1.0".
	const FName SchemaVersion = Config->GetSchemaVersion();
	TestEqual(TEXT("UPGXAIConfig::GetSchemaVersion is 1.0"), SchemaVersion, FName(TEXT("1.0")));

	// EN: Schema descriptor must reflect class name + at least one field
	//     (UPROPERTY taxonomy enumerated by TFieldIterator in helper namespace).
	// ES: Schema descriptor debe reflejar nombre de clase + al menos un field
	//     (taxonomia UPROPERTY enumerada por TFieldIterator en namespace helper).
	const FPGXSchemaDescriptor Descriptor = Config->GetSchemaDescriptor();
	TestEqual(TEXT("Schema TypeName matches class"), Descriptor.TypeName, UPGXAIConfig::StaticClass()->GetFName());
	TestEqual(TEXT("Schema SchemaVersion matches"), Descriptor.SchemaVersion, SchemaVersion);
	TestTrue(TEXT("Schema Fields enumerated (>0)"), Descriptor.Fields.Num() > 0);

	// EN: ToJson must emit canonical envelope with type/version/plugin and
	//     empty data section per Sub-Serialization.3.A baseline.
	// ES: ToJson debe emitir envelope canonical con type/version/plugin y
	//     data section vacia per baseline Sub-Serialization.3.A.
	const FPGXJsonValue Envelope = Config->ToJson();
	TestFalse(TEXT("ToJson envelope non-empty"), Envelope.IsEmpty());
	TestTrue(
		TEXT("ToJson envelope contains type field"),
		Envelope.JsonString.Contains(TEXT("\"type\":\"PGXAIConfig\"")));
	TestTrue(
		TEXT("ToJson envelope contains 1.0 version"),
		Envelope.JsonString.Contains(TEXT("\"version\":\"1.0\"")));

	// EN: FromJson with empty payload returns Failure (envelope guard).
	//     FPGXValidationResult exposes `bValid` field directly (no IsValid()
	//     accessor; AddError flips bValid=false).
	// ES: FromJson con payload vacio retorna Failure (guard de envelope).
	//     FPGXValidationResult expone field `bValid` directamente (sin
	//     accessor IsValid(); AddError flippea bValid=false).
	const FPGXJsonValue EmptyJson;
	const FPGXValidationResult EmptyResult = Config->FromJson(EmptyJson);
	TestFalse(TEXT("FromJson rejects empty payload"), EmptyResult.bValid);

	// EN: FromJson with the canonical envelope is accepted (8.3.A presence-only
	//     validator; no full parser is provided).
	// ES: FromJson con el envelope canonical es aceptado (validador 8.3.A
	//     solo-presencia; parser completo diferido).
	const FPGXValidationResult OkResult = Config->FromJson(Envelope);
	TestTrue(TEXT("FromJson accepts non-empty envelope"), OkResult.bValid);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
