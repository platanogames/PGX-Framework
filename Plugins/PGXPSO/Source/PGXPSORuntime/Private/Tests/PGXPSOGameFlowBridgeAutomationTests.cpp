// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXPSOSubsystem.h"
#include "PGXPSOWarmUpConfig.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/PGXMessage.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/Tags/PGXBridgeTags.h"
#include "Misc/AutomationTest.h"
#include "Tags/PGXPSOTags.h"
#include "Engine/GameInstance.h"

namespace PGXPSOGameFlowBridgeAutomation
{
	struct FScopedGameInstanceFixture
	{
		explicit FScopedGameInstanceFixture(FAutomationTestBase& InTest)
			: Test(InTest)
		{
			GameInstance = NewObject<UGameInstance>(
				GetTransientPackage(),
				UGameInstance::StaticClass(),
				NAME_None,
				RF_Transient);
			if (!GameInstance)
			{
				Test.AddError(TEXT("PGXPSO GameFlow bridge automation setup failed: could not create transient GameInstance."));
				return;
			}

			GameInstance->AddToRoot();
			GameInstance->Init();
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

		UGameInstance* Get() const { return GameInstance; }

	private:
		FAutomationTestBase& Test;
		UGameInstance* GameInstance = nullptr;
		bool bShutdown = false;
	};

	UPGXPSOWarmUpConfig* MakeGameFlowTriggeredConfig(const TCHAR* Name, FGameplayTag TriggerTag, int32 EntryCount = 1)
	{
		UPGXPSOWarmUpConfig* Config = NewObject<UPGXPSOWarmUpConfig>(
			GetTransientPackage(),
			UPGXPSOWarmUpConfig::StaticClass(),
			FName(Name),
			RF_Transient);
		if (!Config)
		{
			return nullptr;
		}

		Config->ActivationMode = EPGXPSOActivationMode::OnGameFlowTag;
		Config->TriggerGameFlowTag = TriggerTag;
		Config->bSaveCacheAfterWarmUp = false;
		Config->BatchSize = 0;

		for (int32 Index = 0; Index < EntryCount; ++Index)
		{
			FPGXPSOEntry Entry;
			Entry.ContextTag = TAG_PGX_PSO_Context_Global.GetTag();
			Entry.Label = FString::Printf(TEXT("GameFlowBridgeSyntheticEntry_%d"), Index);
			Config->Entries.Add(Entry);
		}

		return Config;
	}

	bool PrepareFixture(FAutomationTestBase& Test, UGameInstance* GameInstance,
		UPGXPSOSubsystem*& OutPSO,
		UPGXMessageSubsystem*& OutMessageSubsystem,
		UPGXPSOWarmUpConfig*& OutConfig)
	{
		if (!GameInstance)
		{
			Test.AddError(TEXT("PGXPSO GameFlow bridge automation setup failed: no transient GameInstance."));
			return false;
		}

		OutPSO = GameInstance->GetSubsystem<UPGXPSOSubsystem>();
		OutMessageSubsystem = GameInstance->GetSubsystem<UPGXMessageSubsystem>();
		if (!OutPSO)
		{
			Test.AddError(TEXT("PGXPSO GameFlow bridge automation setup failed: UPGXPSOSubsystem missing."));
		}
		if (!OutMessageSubsystem)
		{
			Test.AddError(TEXT("PGXPSO GameFlow bridge automation setup failed: UPGXMessageSubsystem missing."));
		}
		if (!OutPSO || !OutMessageSubsystem)
		{
			return false;
		}

		OutConfig = MakeGameFlowTriggeredConfig(
			TEXT("PGXPSO_GameFlowBridge_AutomationConfig"),
			TAG_PGX_PSO_Context_Global.GetTag());
		if (!OutConfig)
		{
			Test.AddError(TEXT("PGXPSO GameFlow bridge automation setup failed: could not create transient PSO config."));
			return false;
		}

		OutPSO->InjectTestConfigForTesting(OutConfig);
		OutPSO->RebindGameFlowBridgeForTesting();
		return true;
	}

	void BroadcastGameFlowState(UPGXMessageSubsystem* MessageSubsystem, FGameplayTag NewState)
	{
		FPGXBridgeGameFlowChanged Payload;
		Payload.NewState = NewState;
		Payload.Timestamp = 1.0;
		MessageSubsystem->BroadcastMessage<FPGXBridgeGameFlowChanged>(
			TAG_PGX_Bridge_GameFlow_StateChanged.GetTag(),
			Payload);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXPSOGameFlowBridgeMatchRoundTripAutomationTest,
	"PGX.PSO.GameFlowBridge.MatchRoundTrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXPSOGameFlowBridgeMatchRoundTripAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXPSOGameFlowBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXPSOSubsystem* PSO = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	UPGXPSOWarmUpConfig* Config = nullptr;
	if (!PGXPSOGameFlowBridgeAutomation::PrepareFixture(*this, Fixture.Get(), PSO, MessageSubsystem, Config))
	{
		return true;
	}

	PGXPSOGameFlowBridgeAutomation::BroadcastGameFlowState(MessageSubsystem, Config->TriggerGameFlowTag);

	const FPGXPSOWarmUpProgress Progress = PSO->GetWarmUpProgress();
	TestEqual(TEXT("Matching GameFlow bridge NewState triggers exactly the injected config entry"),
		Progress.TotalEntries,
		Config->Entries.Num());
	TestTrue(TEXT("Matching GameFlow bridge NewState advances PSO warm-up out of Idle"),
		PSO->GetWarmUpState() != EPGXPSOWarmUpState::Idle);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXPSOGameFlowBridgeNonMatchNoOpAutomationTest,
	"PGX.PSO.GameFlowBridge.NonMatchNoOp",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXPSOGameFlowBridgeNonMatchNoOpAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXPSOGameFlowBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXPSOSubsystem* PSO = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	UPGXPSOWarmUpConfig* Config = nullptr;
	if (!PGXPSOGameFlowBridgeAutomation::PrepareFixture(*this, Fixture.Get(), PSO, MessageSubsystem, Config))
	{
		return true;
	}

	PGXPSOGameFlowBridgeAutomation::BroadcastGameFlowState(MessageSubsystem, TAG_PGX_PSO_Context_Menu.GetTag());

	TestEqual(TEXT("Non-matching GameFlow bridge NewState leaves PSO warm-up state unchanged"),
		static_cast<int32>(PSO->GetWarmUpState()),
		static_cast<int32>(EPGXPSOWarmUpState::Idle));
	TestEqual(TEXT("Non-matching GameFlow bridge NewState does not enqueue config entries"),
		PSO->GetWarmUpProgress().TotalEntries,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXPSOGameFlowBridgeInvalidPayloadAutomationTest,
	"PGX.PSO.GameFlowBridge.InvalidPayload",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXPSOGameFlowBridgeInvalidPayloadAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXPSOGameFlowBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXPSOSubsystem* PSO = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	UPGXPSOWarmUpConfig* Config = nullptr;
	if (!PGXPSOGameFlowBridgeAutomation::PrepareFixture(*this, Fixture.Get(), PSO, MessageSubsystem, Config))
	{
		return true;
	}

	FPGXBridgeGameFlowChanged MissingNewStatePayload;
	MessageSubsystem->BroadcastMessage<FPGXBridgeGameFlowChanged>(
		TAG_PGX_Bridge_GameFlow_StateChanged.GetTag(),
		MissingNewStatePayload);

	FPGXMessage WrongPayload;
	WrongPayload.MessageTag = TAG_PGX_Bridge_GameFlow_StateChanged.GetTag();
	WrongPayload.Owner = MessageSubsystem;
	MessageSubsystem->BroadcastMessage<FPGXMessage>(
		TAG_PGX_Bridge_GameFlow_StateChanged.GetTag(),
		WrongPayload);

	TestEqual(TEXT("Invalid/mismatched GameFlow bridge payloads leave PSO warm-up state unchanged"),
		static_cast<int32>(PSO->GetWarmUpState()),
		static_cast<int32>(EPGXPSOWarmUpState::Idle));
	TestEqual(TEXT("Invalid/mismatched GameFlow bridge payloads do not enqueue config entries"),
		PSO->GetWarmUpProgress().TotalEntries,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXPSOGameFlowBridgeListenerLifecycleAutomationTest,
	"PGX.PSO.GameFlowBridge.ListenerLifecycle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXPSOGameFlowBridgeListenerLifecycleAutomationTest::RunTest(const FString& /*Parameters*/)
{
	PGXPSOGameFlowBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXPSOSubsystem* PSO = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	UPGXPSOWarmUpConfig* Config = nullptr;
	if (!PGXPSOGameFlowBridgeAutomation::PrepareFixture(*this, Fixture.Get(), PSO, MessageSubsystem, Config))
	{
		return true;
	}

	TestTrue(TEXT("PSO registers GameFlow bridge listener after OnGameFlowTag config injection"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Bridge_GameFlow_StateChanged.GetTag()) >= 1);

	Fixture.Shutdown();

	TestEqual(TEXT("PSO shutdown clears GameFlow bridge listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Bridge_GameFlow_StateChanged.GetTag()),
		0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
