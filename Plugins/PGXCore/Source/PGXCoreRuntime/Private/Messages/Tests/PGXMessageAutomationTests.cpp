// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: IMPLEMENT_SIMPLE_AUTOMATION_TEST wrappers around UPGXMessageTestUtility BPL helpers,
//     registered under naming `PGX.Message.<TestName>` so headless `Automation RunTests
//     PGX.Message` discovers and runs them. Each wrapper acquires a UWorld via the engine's
//     WorldContexts (PIE / Game / Editor) and forwards to the BPL static; OutIssues lines are
//     emitted via FAutomationTestBase::AddInfo for postmortem visibility. compatibility retrofit
//     (post audit AMBER methodological gap: BPL helpers were not Automation-discoverable).
//
// ES: Wrappers IMPLEMENT_SIMPLE_AUTOMATION_TEST sobre los BPL helpers de UPGXMessageTestUtility,
//     registrados bajo naming `PGX.Message.<TestName>` para que `Automation RunTests PGX.Message`
//     headless los descubra y ejecute. Cada wrapper obtiene UWorld desde GEngine->WorldContexts
//     y delega al BPL static; las lineas de OutIssues se publican via AddInfo para postmortem.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Messages/PGXMessageTestUtility.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PGXMessageAutomationTestsInternal
{
	// EN: Acquire a UWorld from the engine's contexts. In headless commandlet mode without an
	//     active map, this may return nullptr; the BPL helpers detect that and report
	//     "MessageSubsystem not available" — the wrapper still runs and records the failure.
	// ES: Obtener un UWorld desde los WorldContexts del engine. En commandlet headless sin mapa
	//     activo, puede devolver nullptr; los BPL helpers reportan "MessageSubsystem not available".
	static UWorld* AcquireTestWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		// Prefer PIE world, then Game, then Editor as last resort.
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
// Listener safety
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_TypeMismatchRejectedAutomationTest,
	"PGX.Message.TypeMismatchRejected",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_TypeMismatchRejectedAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::TypeMismatchRejectedTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_DoubleUnregisterSafeAutomationTest,
	"PGX.Message.DoubleUnregisterSafe",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_DoubleUnregisterSafeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::DoubleUnregisterSafeTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_CallbackRemovalSafeAutomationTest,
	"PGX.Message.CallbackRemovalSafe",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_CallbackRemovalSafeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::CallbackRemovalSafeTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

// ============================================================
// Partial matching
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_ExactParentNoChildAutomationTest,
	"PGX.Message.ExactParentNoChild",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_ExactParentNoChildAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::ExactParentNoChildTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

#if WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_PartialMatchGlobalDisabledAutomationTest,
	"PGX.Message.PartialMatchGlobalDisabled",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_PartialMatchGlobalDisabledAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::PartialMatchGlobalDisabledTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}
#endif // WITH_EDITOR

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_ChannelIsolationAutomationTest,
	"PGX.Message.ChannelIsolation",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_ChannelIsolationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::ChannelIsolationTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

// ============================================================
// Payload backward compatibility
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_PayloadBackwardCompatAutomationTest,
	"PGX.Message.PayloadBackwardCompat",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_PayloadBackwardCompatAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::PayloadBackwardCompatTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

// ============================================================
// Configuration and history — WITH_EDITOR (uses InjectTestConfig)
// ============================================================

#if WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_HistoryBoundedByPolicyAutomationTest,
	"PGX.Message.HistoryBoundedByPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_HistoryBoundedByPolicyAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::HistoryBoundedByPolicyTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_ClearHistoryAdditionalAutomationTest,
	"PGX.Message.ClearHistoryAdditional",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_ClearHistoryAdditionalAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::ClearHistoryAdditionalTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_EmergencyHistoryFallbackAutomationTest,
	"PGX.Message.EmergencyHistoryFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_EmergencyHistoryFallbackAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::EmergencyHistoryFallbackTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}
#endif // WITH_EDITOR

// ============================================================
// Telemetry — telemetry (deferred-dispatch behavior)
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_FanOutTelemetryAutomationTest,
	"PGX.Message.FanOutTelemetry",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_FanOutTelemetryAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::FanOutTelemetryTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_NestedBroadcastOrderingAutomationTest,
	"PGX.Message.NestedBroadcastOrdering",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_NestedBroadcastOrderingAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::NestedBroadcastOrderingTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_TimeAccelBoundaryReadOnlyAutomationTest,
	"PGX.Message.TimeAccelBoundaryReadOnly",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_TimeAccelBoundaryReadOnlyAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::TimeAccelBoundaryReadOnlyTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

// ============================================================
// FPGXBridgeGameFlowChanged appended-field compatibility
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_BridgePayloadExtensionPresenceAutomationTest,
	"PGX.Message.BridgePayloadExtensionPresence",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_BridgePayloadExtensionPresenceAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::BridgePayloadExtensionPresenceTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_BridgePayloadExtensionRoundtripAutomationTest,
	"PGX.Message.BridgePayloadExtensionRoundtrip",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_BridgePayloadExtensionRoundtripAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::BridgePayloadExtensionRoundtripTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_BridgePayloadExtensionBackwardCompatAutomationTest,
	"PGX.Message.BridgePayloadExtensionBackwardCompat",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_BridgePayloadExtensionBackwardCompatAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::BridgePayloadExtensionBackwardCompatTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_BridgePayloadExtensionRequestIdAutomationTest,
	"PGX.Message.BridgePayloadExtensionRequestId",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_BridgePayloadExtensionRequestIdAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UWorld* World = PGXMessageAutomationTestsInternal::AcquireTestWorld();
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::BridgePayloadExtensionRequestIdTest(World, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

#endif // WITH_DEV_AUTOMATION_TESTS
