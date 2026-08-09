// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXLoadingSubsystem.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/PGXMessage.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Tags/PGXLoadingTags.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

namespace PGXLoadingPSOBridgeAutomation
{
	struct FScopedGameInstanceFixture
	{
		explicit FScopedGameInstanceFixture(FAutomationTestBase& InTest)
			: Test(InTest)
		{
			if (!GEngine)
			{
				Test.AddError(TEXT("PGXLoading PSO bridge automation setup failed: engine is unavailable."));
				return;
			}

			GameInstance = NewObject<UGameInstance>(
				GEngine,
				UGameInstance::StaticClass(),
				NAME_None,
				RF_Transient);
			if (!GameInstance)
			{
				Test.AddError(TEXT("PGXLoading PSO bridge automation setup failed: could not create transient GameInstance."));
				return;
			}

			GameInstance->AddToRoot();
			GameInstance->InitializeStandalone(TEXT("PGXLoadingPSOBridgeAutomationWorld"));
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

		UGameInstance* Get() const { return GameInstance; }

	private:
		FAutomationTestBase& Test;
		UGameInstance* GameInstance = nullptr;
		bool bShutdown = false;
	};

	bool PrepareFixture(FAutomationTestBase& Test, UGameInstance* GameInstance,
		UPGXLoadingSubsystem*& OutLoading, UPGXMessageSubsystem*& OutMessageSubsystem)
	{
		if (!GameInstance)
		{
			Test.AddError(TEXT("PGXLoading PSO bridge automation setup failed: no transient GameInstance."));
			return false;
		}

		OutLoading = GameInstance->GetSubsystem<UPGXLoadingSubsystem>();
		OutMessageSubsystem = GameInstance->GetSubsystem<UPGXMessageSubsystem>();
		if (!OutLoading)
		{
			Test.AddError(TEXT("PGXLoading PSO bridge automation setup failed: UPGXLoadingSubsystem missing."));
		}
		if (!OutMessageSubsystem)
		{
			Test.AddError(TEXT("PGXLoading PSO bridge automation setup failed: UPGXMessageSubsystem missing."));
		}
		return OutLoading && OutMessageSubsystem;
	}

	void BroadcastPSOProgress(UPGXMessageSubsystem* MessageSubsystem, float Progress)
	{
		FPGXBridgeLoadingState Payload;
		Payload.bIsLoading = true;
		Payload.Progress = Progress;
		Payload.Timestamp = 1.0;
		MessageSubsystem->BroadcastMessage<FPGXBridgeLoadingState>(
			TAG_PGX_Loading_PSO_Progress.GetTag(),
			Payload);
	}

	void BroadcastPSOComplete(UPGXMessageSubsystem* MessageSubsystem)
	{
		FPGXBridgeLoadingState Payload;
		Payload.bIsLoading = false;
		Payload.Progress = 1.0f;
		Payload.Timestamp = 2.0;
		MessageSubsystem->BroadcastMessage<FPGXBridgeLoadingState>(
			TAG_PGX_Loading_PSO_Complete.GetTag(),
			Payload);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoadingPSOBridgeProgressRoundTripAutomationTest,
	"PGX.Loading.PSOBridge.ProgressRoundTrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoadingPSOBridgeProgressRoundTripAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXLoadingPSOBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXLoadingSubsystem* Loading = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	if (!PGXLoadingPSOBridgeAutomation::PrepareFixture(*this, Fixture.Get(), Loading, MessageSubsystem))
	{
		return true;
	}

	PGXLoadingPSOBridgeAutomation::BroadcastPSOProgress(MessageSubsystem, 0.42f);

	TestTrue(TEXT("PSO progress message marks bridge as bound"),
		Loading->IsPSOBridgeBoundForTesting());
	TestTrue(TEXT("PSO progress message marks warm-up active"),
		Loading->IsPSOBridgeWarmUpActiveForTesting());
	TestEqual(TEXT("PSO progress message updates Loading PSO progress cache"),
		Loading->GetPSOProgressValueForTesting(),
		0.42f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoadingPSOBridgeCompleteRoundTripAutomationTest,
	"PGX.Loading.PSOBridge.CompleteRoundTrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoadingPSOBridgeCompleteRoundTripAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXLoadingPSOBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXLoadingSubsystem* Loading = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	if (!PGXLoadingPSOBridgeAutomation::PrepareFixture(*this, Fixture.Get(), Loading, MessageSubsystem))
	{
		return true;
	}

	PGXLoadingPSOBridgeAutomation::BroadcastPSOProgress(MessageSubsystem, 0.25f);
	PGXLoadingPSOBridgeAutomation::BroadcastPSOComplete(MessageSubsystem);

	TestTrue(TEXT("PSO complete message keeps bridge as bound"),
		Loading->IsPSOBridgeBoundForTesting());
	TestFalse(TEXT("PSO complete message clears warm-up active flag"),
		Loading->IsPSOBridgeWarmUpActiveForTesting());
	TestTrue(TEXT("PSO complete message marks Loading PSO ready"),
		Loading->IsPSOReadyForTesting());
	TestEqual(TEXT("PSO complete message sets Loading PSO progress to 1.0"),
		Loading->GetPSOProgressValueForTesting(),
		1.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoadingPSOBridgeInvalidPayloadAutomationTest,
	"PGX.Loading.PSOBridge.InvalidPayload",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoadingPSOBridgeInvalidPayloadAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXLoadingPSOBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXLoadingSubsystem* Loading = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	if (!PGXLoadingPSOBridgeAutomation::PrepareFixture(*this, Fixture.Get(), Loading, MessageSubsystem))
	{
		return true;
	}

	PGXLoadingPSOBridgeAutomation::BroadcastPSOProgress(MessageSubsystem, 0.33f);
	const float ProgressBeforeInvalid = Loading->GetPSOProgressValueForTesting();
	const bool bWarmUpActiveBeforeInvalid = Loading->IsPSOBridgeWarmUpActiveForTesting();

	FPGXMessage WrongPayload;
	WrongPayload.MessageTag = TAG_PGX_Loading_PSO_Progress.GetTag();
	WrongPayload.Owner = MessageSubsystem;
	WrongPayload.Timestamp = 3.0;
	AddExpectedError(
		TEXT("Struct type mismatch on channel PGX.Loading.PSO.Progress"),
		EAutomationExpectedErrorFlags::Contains,
		1);
	MessageSubsystem->BroadcastMessage<FPGXMessage>(
		TAG_PGX_Loading_PSO_Progress.GetTag(),
		WrongPayload);

	TestEqual(TEXT("Mismatched PSO progress payload does not mutate Loading progress"),
		Loading->GetPSOProgressValueForTesting(),
		ProgressBeforeInvalid);
	TestEqual(TEXT("Mismatched PSO progress payload does not mutate warm-up active flag"),
		Loading->IsPSOBridgeWarmUpActiveForTesting(),
		bWarmUpActiveBeforeInvalid);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLoadingPSOBridgeListenerLifecycleAutomationTest,
	"PGX.Loading.PSOBridge.ListenerLifecycle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXLoadingPSOBridgeListenerLifecycleAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXLoadingPSOBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXLoadingSubsystem* Loading = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	if (!PGXLoadingPSOBridgeAutomation::PrepareFixture(*this, Fixture.Get(), Loading, MessageSubsystem))
	{
		return true;
	}

	TestTrue(TEXT("Loading Initialize registers PSO state listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_PSO_State.GetTag()) >= 1);
	TestTrue(TEXT("Loading Initialize registers PSO progress listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_PSO_Progress.GetTag()) >= 1);
	TestTrue(TEXT("Loading Initialize registers PSO complete listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_PSO_Complete.GetTag()) >= 1);

	Fixture.Shutdown();

	TestEqual(TEXT("Loading shutdown clears PSO state listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_PSO_State.GetTag()), 0);
	TestEqual(TEXT("Loading shutdown clears PSO progress listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_PSO_Progress.GetTag()), 0);
	TestEqual(TEXT("Loading shutdown clears PSO complete listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_PSO_Complete.GetTag()), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
