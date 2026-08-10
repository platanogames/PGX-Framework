// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
//
// EN: IMPLEMENT_SIMPLE_AUTOMATION_TEST wrappers around UPGXMessageTestUtility BPL helpers,
//     registered under naming `PGX.Message.<TestName>` so headless `Automation RunTests
//     PGX.Message` discovers and runs them. Each wrapper acquires a UWorld via the engine's
//     WorldContexts (PIE / Game / Editor) and forwards to the BPL static; OutIssues lines are
//     emitted via FAutomationTestBase::AddInfo for diagnostic visibility. These wrappers make
//     the BPL helpers discoverable by Automation without changing their behavior.
//
// ES: Wrappers IMPLEMENT_SIMPLE_AUTOMATION_TEST sobre los BPL helpers de UPGXMessageTestUtility,
//     registrados bajo naming `PGX.Message.<TestName>` para que `Automation RunTests PGX.Message`
//     headless los descubra y ejecute. Cada wrapper obtiene UWorld desde GEngine->WorldContexts
//     y delega al BPL static; las lineas de OutIssues se publican via AddInfo para postmortem.

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Messages/PGXMessageTestUtility.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PGXMessageAutomationTestsInternal
{
	class FScopedMessageTestContext
	{
	public:
		FScopedMessageTestContext()
		{
			if (!GEngine)
			{
				return;
			}

			GameInstance = NewObject<UGameInstance>(GEngine, UGameInstance::StaticClass(), NAME_None, RF_Transient);
			if (!GameInstance)
			{
				return;
			}

			GameInstance->AddToRoot();
			GameInstance->InitializeStandalone();

			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World)
			{
				World->SetGameInstance(GameInstance);
			}
		}

		~FScopedMessageTestContext()
		{
			if (World)
			{
				World->SetGameInstance(nullptr);
				World->DestroyWorld(false);
				World = nullptr;
			}
			if (GameInstance)
			{
				GameInstance->Shutdown();
				GameInstance->RemoveFromRoot();
				GameInstance = nullptr;
			}
		}

		bool IsValid() const
		{
			return World
				&& GameInstance
				&& World->GetGameInstance() == GameInstance
				&& GameInstance->GetSubsystem<UPGXMessageSubsystem>() != nullptr;
		}

		UWorld* GetWorld() const { return World; }

	private:
		UWorld* World = nullptr;
		UGameInstance* GameInstance = nullptr;
	};

	static void ForwardIssues(FAutomationTestBase& Test, const TArray<FString>& OutIssues)
	{
		for (const FString& Issue : OutIssues)
		{
			if (Issue.StartsWith(TEXT("FAIL:")))
			{
				Test.AddError(Issue);
			}
			else
			{
				Test.AddInfo(Issue);
			}
		}
	}

	using FMessageTestFunction = bool (*)(const UObject*, TArray<FString>&);

	static bool RunIsolated(FAutomationTestBase& Test, FMessageTestFunction TestFunction)
	{
		FScopedMessageTestContext Fixture;
		if (!Fixture.IsValid())
		{
			Test.AddError(TEXT("FAIL: isolated Message World/GameInstance fixture initialization failed"));
			return false;
		}

		TArray<FString> OutIssues;
		const bool bPassed = TestFunction(Fixture.GetWorld(), OutIssues);
		ForwardIssues(Test, OutIssues);
		return bPassed;
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
	AddExpectedError(TEXT("Struct type mismatch on channel"), EAutomationExpectedErrorFlags::Contains, 1);
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::TypeMismatchRejectedTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_DoubleUnregisterSafeAutomationTest,
	"PGX.Message.DoubleUnregisterSafe",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_DoubleUnregisterSafeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::DoubleUnregisterSafeTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_CallbackRemovalSafeAutomationTest,
	"PGX.Message.CallbackRemovalSafe",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_CallbackRemovalSafeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::CallbackRemovalSafeTest);
}

// ============================================================
// Partial matching
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_ExactParentNoChildAutomationTest,
	"PGX.Message.ExactParentNoChild",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_ExactParentNoChildAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::ExactParentNoChildTest);
}

#if WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_PartialMatchGlobalDisabledAutomationTest,
	"PGX.Message.PartialMatchGlobalDisabled",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_PartialMatchGlobalDisabledAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXMessageAutomationTestsInternal::FScopedMessageTestContext Fixture;
	if (!Fixture.IsValid())
	{
		AddError(TEXT("FAIL: isolated Message World/GameInstance fixture initialization failed"));
		return false;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::PartialMatchGlobalDisabledTest(Fixture.GetWorld(), OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}
#endif // WITH_EDITOR

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_ChannelIsolationAutomationTest,
	"PGX.Message.ChannelIsolation",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_ChannelIsolationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::ChannelIsolationTest);
}

// ============================================================
// Payload backward compatibility
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_PayloadBackwardCompatAutomationTest,
	"PGX.Message.PayloadBackwardCompat",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_PayloadBackwardCompatAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::PayloadBackwardCompatTest);
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
	PGXMessageAutomationTestsInternal::FScopedMessageTestContext Fixture;
	if (!Fixture.IsValid())
	{
		AddError(TEXT("FAIL: isolated Message World/GameInstance fixture initialization failed"));
		return false;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::HistoryBoundedByPolicyTest(Fixture.GetWorld(), OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_ClearHistoryAdditionalAutomationTest,
	"PGX.Message.ClearHistoryAdditional",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_ClearHistoryAdditionalAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXMessageAutomationTestsInternal::FScopedMessageTestContext Fixture;
	if (!Fixture.IsValid())
	{
		AddError(TEXT("FAIL: isolated Message World/GameInstance fixture initialization failed"));
		return false;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::ClearHistoryAdditionalTest(Fixture.GetWorld(), OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_EmergencyHistoryFallbackAutomationTest,
	"PGX.Message.EmergencyHistoryFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_EmergencyHistoryFallbackAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXMessageAutomationTestsInternal::FScopedMessageTestContext Fixture;
	if (!Fixture.IsValid())
	{
		AddError(TEXT("FAIL: isolated Message World/GameInstance fixture initialization failed"));
		return false;
	}
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::EmergencyHistoryFallbackTest(Fixture.GetWorld(), OutIssues);
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
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::FanOutTelemetryTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_NestedBroadcastOrderingAutomationTest,
	"PGX.Message.NestedBroadcastOrdering",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_NestedBroadcastOrderingAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::NestedBroadcastOrderingTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_TimeAccelBoundaryReadOnlyAutomationTest,
	"PGX.Message.TimeAccelBoundaryReadOnly",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_TimeAccelBoundaryReadOnlyAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::TimeAccelBoundaryReadOnlyTest);
}

// ============================================================
// FPGXBridgeGameFlowChanged appended-field compatibility
// ============================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_BridgePayloadExtensionPresenceAutomationTest,
	"PGX.Message.BridgePayloadExtensionPresence",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_BridgePayloadExtensionPresenceAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = UPGXMessageTestUtility::BridgePayloadExtensionPresenceTest(nullptr, OutIssues);
	PGXMessageAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_BridgePayloadExtensionRoundtripAutomationTest,
	"PGX.Message.BridgePayloadExtensionRoundtrip",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_BridgePayloadExtensionRoundtripAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::BridgePayloadExtensionRoundtripTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_BridgePayloadExtensionBackwardCompatAutomationTest,
	"PGX.Message.BridgePayloadExtensionBackwardCompat",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_BridgePayloadExtensionBackwardCompatAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::BridgePayloadExtensionBackwardCompatTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXMessage_BridgePayloadExtensionRequestIdAutomationTest,
	"PGX.Message.BridgePayloadExtensionRequestId",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXMessage_BridgePayloadExtensionRequestIdAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXMessageAutomationTestsInternal::RunIsolated(
		*this, &UPGXMessageTestUtility::BridgePayloadExtensionRequestIdTest);
}

#endif // WITH_DEV_AUTOMATION_TESTS
