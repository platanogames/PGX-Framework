// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXFlowRulesConfig.h"
#include "PGXGameFlowConfig.h"
#include "PGXGameFlowSubsystem.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/PGXMessage.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Tags/PGXGameFlowTags.h"
#include "Testing/PGXTestBase.h"
#include "Engine/GameInstance.h"

namespace PGXGameFlowLoadingBridgeAutomation
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
				Test.AddError(TEXT("PGXGameFlow loading bridge automation setup failed: could not create transient GameInstance."));
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

	UPGXGameFlowConfig* MakeConfig(const TCHAR* Name, int32 MaxHistoryDepth = 8)
	{
		UPGXGameFlowConfig* Config = NewObject<UPGXGameFlowConfig>(
			GetTransientPackage(),
			UPGXGameFlowConfig::StaticClass(),
			FName(Name),
			RF_Transient);
		Config->MaxHistoryDepth = MaxHistoryDepth;
		Config->bLogTransitions = false;
		Config->bVerboseDebug = false;
		Config->bAllowConsoleMutations = false;
		Config->DuplicateRulesPolicy = EPGXFlowDuplicateRulesPolicy::HighestPriorityWins;
		return Config;
	}

	UPGXGameFlowSubsystem* FindGameFlow(FAutomationTestBase& Test, UGameInstance* GameInstance)
	{
		if (!GameInstance)
		{
			Test.AddError(TEXT("PGXGameFlow loading bridge automation setup failed: no transient GameInstance available."));
			return nullptr;
		}

		UPGXGameFlowSubsystem* GameFlow = GameInstance->GetSubsystem<UPGXGameFlowSubsystem>();
		if (!GameFlow)
		{
			Test.AddError(TEXT("PGXGameFlow loading bridge automation setup failed: UPGXGameFlowSubsystem missing."));
		}
		return GameFlow;
	}

	UPGXMessageSubsystem* FindMessageSubsystem(FAutomationTestBase& Test, UGameInstance* GameInstance)
	{
		if (!GameInstance)
		{
			Test.AddError(TEXT("PGXGameFlow loading bridge automation setup failed: no transient GameInstance available."));
			return nullptr;
		}

		UPGXMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UPGXMessageSubsystem>();
		if (!MessageSubsystem)
		{
			Test.AddError(TEXT("PGXGameFlow loading bridge automation setup failed: UPGXMessageSubsystem missing."));
		}
		return MessageSubsystem;
	}

	bool PrepareBridgeFixture(FAutomationTestBase& Test, UGameInstance* GameInstance,
		UPGXGameFlowSubsystem*& OutGameFlow,
		UPGXMessageSubsystem*& OutMessageSubsystem)
	{
		OutGameFlow = FindGameFlow(Test, GameInstance);
		OutMessageSubsystem = FindMessageSubsystem(Test, GameInstance);
		if (!OutGameFlow || !OutMessageSubsystem)
		{
			return false;
		}

		UPGXGameFlowConfig* Config = MakeConfig(TEXT("PGXGameFlow_LoadingBridge_AutomationConfig"));
		OutGameFlow->InjectTestConfig(Config);
		OutGameFlow->InjectTestRulesConfigs({});
		OutGameFlow->ResetChannelStatesForTesting();
		return true;
	}

	void BroadcastLoadingSetState(UPGXMessageSubsystem* MessageSubsystem, FGameplayTag TargetState)
	{
		FPGXMessage Message;
		Message.MessageTag = TargetState;
		Message.Owner = MessageSubsystem;
		Message.Timestamp = 1.0;
		MessageSubsystem->BroadcastMessage<FPGXMessage>(TAG_PGX_Loading_GameFlow_SetState.GetTag(), Message);
	}

	void BroadcastLoadingRevert(UPGXMessageSubsystem* MessageSubsystem)
	{
		FPGXMessage Message;
		Message.MessageTag = TAG_PGX_Loading_GameFlow_Revert.GetTag();
		Message.Owner = MessageSubsystem;
		Message.Timestamp = 1.0;
		MessageSubsystem->BroadcastMessage<FPGXMessage>(TAG_PGX_Loading_GameFlow_Revert.GetTag(), Message);
	}
}

PGX_TEST_GAME(FPGXGameFlow_LoadingBridgeSetStateRoundTripAutomationTest)
{
	PGXGameFlowLoadingBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXGameFlowSubsystem* GameFlow = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	if (!PGXGameFlowLoadingBridgeAutomation::PrepareBridgeFixture(*this, Fixture.Get(), GameFlow, MessageSubsystem))
	{
		return true;
	}

	PGXGameFlowLoadingBridgeAutomation::BroadcastLoadingSetState(MessageSubsystem, TAG_PGX_GameFlow_State_Loading.GetTag());

	TestTrue(TEXT("Loading bridge set-state message mutates GameFlow Global state"),
		GameFlow->GetCurrentFlowTag(EPGXFlowChannel::Global) == TAG_PGX_GameFlow_State_Loading.GetTag());
	return true;
}

PGX_TEST_GAME(FPGXGameFlow_LoadingBridgeRevertRoundTripAutomationTest)
{
	PGXGameFlowLoadingBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXGameFlowSubsystem* GameFlow = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	if (!PGXGameFlowLoadingBridgeAutomation::PrepareBridgeFixture(*this, Fixture.Get(), GameFlow, MessageSubsystem))
	{
		return true;
	}

	PGXGameFlowLoadingBridgeAutomation::BroadcastLoadingSetState(MessageSubsystem, TAG_PGX_GameFlow_State_Boot.GetTag());
	PGXGameFlowLoadingBridgeAutomation::BroadcastLoadingSetState(MessageSubsystem, TAG_PGX_GameFlow_State_MainMenu.GetTag());
	TestTrue(TEXT("Loading bridge set-state seeded current state before revert"),
		GameFlow->GetCurrentFlowTag(EPGXFlowChannel::Global) == TAG_PGX_GameFlow_State_MainMenu.GetTag());

	PGXGameFlowLoadingBridgeAutomation::BroadcastLoadingRevert(MessageSubsystem);

	TestTrue(TEXT("Loading bridge revert message restores previous GameFlow Global state"),
		GameFlow->GetCurrentFlowTag(EPGXFlowChannel::Global) == TAG_PGX_GameFlow_State_Boot.GetTag());
	return true;
}

PGX_TEST_GAME(FPGXGameFlow_LoadingBridgeInvalidPayloadAutomationTest)
{
	PGXGameFlowLoadingBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXGameFlowSubsystem* GameFlow = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	if (!PGXGameFlowLoadingBridgeAutomation::PrepareBridgeFixture(*this, Fixture.Get(), GameFlow, MessageSubsystem))
	{
		return true;
	}

	PGXGameFlowLoadingBridgeAutomation::BroadcastLoadingSetState(MessageSubsystem, TAG_PGX_GameFlow_State_Boot.GetTag());
	const FGameplayTag StateBeforeInvalidMessages = GameFlow->GetCurrentFlowTag(EPGXFlowChannel::Global);

	FPGXMessage MissingTargetMessage;
	MissingTargetMessage.Owner = MessageSubsystem;
	MessageSubsystem->BroadcastMessage<FPGXMessage>(TAG_PGX_Loading_GameFlow_SetState.GetTag(), MissingTargetMessage);

	FPGXBridgeGameFlowChanged WrongPayload;
	WrongPayload.NewState = TAG_PGX_GameFlow_State_InWorld.GetTag();
	MessageSubsystem->BroadcastMessage<FPGXBridgeGameFlowChanged>(TAG_PGX_Loading_GameFlow_SetState.GetTag(), WrongPayload);

	TestTrue(TEXT("Loading bridge invalid/mismatched payloads do not mutate GameFlow Global state"),
		GameFlow->GetCurrentFlowTag(EPGXFlowChannel::Global) == StateBeforeInvalidMessages);
	return true;
}

PGX_TEST_GAME(FPGXGameFlow_LoadingBridgeListenerLifecycleAutomationTest)
{
	PGXGameFlowLoadingBridgeAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXGameFlowSubsystem* GameFlow = nullptr;
	UPGXMessageSubsystem* MessageSubsystem = nullptr;
	if (!PGXGameFlowLoadingBridgeAutomation::PrepareBridgeFixture(*this, Fixture.Get(), GameFlow, MessageSubsystem))
	{
		return true;
	}

	TestTrue(TEXT("GameFlow Initialize registers Loading set-state listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_GameFlow_SetState.GetTag()) >= 1);
	TestTrue(TEXT("GameFlow Initialize registers Loading revert listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_GameFlow_Revert.GetTag()) >= 1);

	Fixture.Shutdown();

	TestEqual(TEXT("GameFlow/Message shutdown clears Loading set-state listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_GameFlow_SetState.GetTag()), 0);
	TestEqual(TEXT("GameFlow/Message shutdown clears Loading revert listener"),
		MessageSubsystem->GetListenerCount(TAG_PGX_Loading_GameFlow_Revert.GetTag()), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
