// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXFlowRulesConfig.h"
#include "PGXGameFlowConfig.h"
#include "PGXGameFlowSubsystem.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/PGXMessageDelegates.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Tags/PGXGameFlowTags.h"
#include "Testing/PGXTestBase.h"
#include "Engine/GameInstance.h"

namespace PGXGameFlowAutomation
{
	const FName GBridgeGameFlowStateChangedTagName(TEXT("PGX.Message.Bridge.GameFlow.StateChanged"));

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
				Test.AddError(TEXT("PGXGameFlow automation setup failed: could not create transient GameInstance."));
				return;
			}

			GameInstance->AddToRoot();
			GameInstance->Init();
		}

		~FScopedGameInstanceFixture()
		{
			if (!GameInstance)
			{
				return;
			}

			GameInstance->Shutdown();
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
	};

	FGameplayTag GetBridgeGameFlowStateChangedTag(FAutomationTestBase& Test)
	{
		const FGameplayTag BridgeTag = FGameplayTag::RequestGameplayTag(GBridgeGameFlowStateChangedTagName, false);
		if (!BridgeTag.IsValid())
		{
			Test.AddError(TEXT("PGXGameFlow bridge automation setup failed: bridge state-changed tag not registered."));
		}
		return BridgeTag;
	}

	UPGXGameFlowSubsystem* FindGameFlow(FAutomationTestBase& Test, UGameInstance* GameInstance)
	{
		if (!GameInstance)
		{
			Test.AddError(TEXT("PGXGameFlow automation setup failed: no transient GameInstance available."));
			return nullptr;
		}

		UPGXGameFlowSubsystem* GameFlow = GameInstance->GetSubsystem<UPGXGameFlowSubsystem>();
		if (!GameFlow)
		{
			Test.AddError(TEXT("PGXGameFlow automation setup failed: UPGXGameFlowSubsystem missing."));
		}
		return GameFlow;
	}

	UPGXMessageSubsystem* FindMessageSubsystem(FAutomationTestBase& Test, UGameInstance* GameInstance)
	{
		if (!GameInstance)
		{
			Test.AddError(TEXT("PGXGameFlow bridge automation setup failed: no transient GameInstance available."));
			return nullptr;
		}

		UPGXMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UPGXMessageSubsystem>();
		if (!MessageSubsystem)
		{
			Test.AddError(TEXT("PGXGameFlow bridge automation setup failed: UPGXMessageSubsystem missing."));
		}
		return MessageSubsystem;
	}

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

	UPGXFlowRulesConfig* MakeRules(const TCHAR* Name, EPGXFlowChannel Channel, int32 Priority)
	{
		UPGXFlowRulesConfig* Rules = NewObject<UPGXFlowRulesConfig>(
			GetTransientPackage(),
			UPGXFlowRulesConfig::StaticClass(),
			FName(Name),
			RF_Transient);
		Rules->Channel = Channel;
		Rules->ConflictPriority = Priority;
		return Rules;
	}

	bool PreparePermissiveGameFlow(FAutomationTestBase& Test, UGameInstance* GameInstance, UPGXGameFlowSubsystem*& OutGameFlow, int32 MaxHistoryDepth = 8)
	{
		OutGameFlow = FindGameFlow(Test, GameInstance);
		if (!OutGameFlow)
		{
			return false;
		}

		UPGXGameFlowConfig* Config = MakeConfig(TEXT("PGXGameFlow_AutomationConfig"), MaxHistoryDepth);
		OutGameFlow->InjectTestConfig(Config);
		OutGameFlow->InjectTestRulesConfigs({});
		OutGameFlow->ResetChannelStatesForTesting();
		return true;
	}
}

PGX_TEST_GAME(FPGXGameFlow_AllowedBranchAutomationTest)
{
	const FGameplayTag Branch = TAG_PGX_GameFlow_State;
	const FGameplayTag Exact = TAG_PGX_GameFlow_State;
	const FGameplayTag Descendant = TAG_PGX_GameFlow_State_InWorld;
	const FGameplayTag Sibling = TAG_PGX_GameFlow_TransitionSource_Player;

	TestTrue(TEXT("AllowedBranchExact"), UPGXGameFlowSubsystem::DoesTagMatchBranchForTesting(Exact, Branch));
	TestTrue(TEXT("AllowedBranchDescendant"), UPGXGameFlowSubsystem::DoesTagMatchBranchForTesting(Descendant, Branch));
	TestFalse(TEXT("AllowedBranchSiblingRejected"), UPGXGameFlowSubsystem::DoesTagMatchBranchForTesting(Sibling, Branch));
	TestFalse(TEXT("IsInBranchMissingTag"), UPGXGameFlowSubsystem::DoesTagMatchBranchForTesting(FGameplayTag::EmptyTag, Branch));
	return true;
}

PGX_TEST_GAME(FPGXGameFlow_DuplicateRulesConflictPolicyAutomationTest)
{
	PGXGameFlowAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXGameFlowSubsystem* GameFlow = nullptr;
	if (!PGXGameFlowAutomation::PreparePermissiveGameFlow(*this, Fixture.Get(), GameFlow))
	{
		return true;
	}

	UPGXFlowRulesConfig* RulesA = PGXGameFlowAutomation::MakeRules(TEXT("PGXGameFlow_AutomationRules_A"), EPGXFlowChannel::Global, 1);
	UPGXFlowRulesConfig* RulesB = PGXGameFlowAutomation::MakeRules(TEXT("PGXGameFlow_AutomationRules_B"), EPGXFlowChannel::Global, 1);
	UPGXFlowRulesConfig* RulesHigh = PGXGameFlowAutomation::MakeRules(TEXT("PGXGameFlow_AutomationRules_High"), EPGXFlowChannel::Global, 9);

	UPGXGameFlowConfig* Config = PGXGameFlowAutomation::MakeConfig(TEXT("PGXGameFlow_DuplicatePolicyConfig"));
	GameFlow->InjectTestConfig(Config);

	Config->DuplicateRulesPolicy = EPGXFlowDuplicateRulesPolicy::FirstWins;
	GameFlow->InjectTestRulesConfigs({RulesA, RulesB});
	TestTrue(TEXT("DuplicateRulesConflictPolicy FirstWins"), GameFlow->GetRulesConfigForTesting(EPGXFlowChannel::Global) == RulesA);

	Config->DuplicateRulesPolicy = EPGXFlowDuplicateRulesPolicy::LastWins;
	GameFlow->InjectTestRulesConfigs({RulesA, RulesB});
	TestTrue(TEXT("DuplicateRulesConflictPolicy LastWins"), GameFlow->GetRulesConfigForTesting(EPGXFlowChannel::Global) == RulesB);

	Config->DuplicateRulesPolicy = EPGXFlowDuplicateRulesPolicy::HighestPriorityWins;
	GameFlow->InjectTestRulesConfigs({RulesA, RulesHigh, RulesB});
	TestTrue(TEXT("DuplicateRulesConflictPolicy HighestPriorityWins"), GameFlow->GetRulesConfigForTesting(EPGXFlowChannel::Global) == RulesHigh);
	return true;
}

PGX_TEST_GAME(FPGXGameFlow_BatchSimulationParityAutomationTest)
{
	PGXGameFlowAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXGameFlowSubsystem* GameFlow = nullptr;
	if (!PGXGameFlowAutomation::PreparePermissiveGameFlow(*this, Fixture.Get(), GameFlow))
	{
		return true;
	}

	TestTrue(TEXT("Seed Boot"), GameFlow->SetStateByTag(EPGXFlowChannel::UI, TAG_PGX_GameFlow_State_Boot).bSuccess);

	const TArray<FGameplayTag> ValidBatch =
	{
		TAG_PGX_GameFlow_State_MainMenu,
		TAG_PGX_GameFlow_State_Loading
	};
	TestTrue(TEXT("CanBatchChangeByTag accepts valid simulated sequence"),
		GameFlow->CanBatchChangeByTag(EPGXFlowChannel::UI, ValidBatch).bSuccess);
	TestTrue(TEXT("SetBatchSequentialStateByTag applies same sequence"),
		GameFlow->SetBatchSequentialStateByTag(EPGXFlowChannel::UI, ValidBatch).bSuccess);
	TestTrue(TEXT("Batch final state matches simulated final state"),
		GameFlow->GetCurrentFlowTag(EPGXFlowChannel::UI) == TAG_PGX_GameFlow_State_Loading.GetTag());

	const TArray<FGameplayTag> RedundantBatch =
	{
		TAG_PGX_GameFlow_State_Loading,
		TAG_PGX_GameFlow_State_InWorld
	};
	const FGameplayTag BeforeRejectedBatch = GameFlow->GetCurrentFlowTag(EPGXFlowChannel::UI);
	const FPGXFlowResult Rejected = GameFlow->SetBatchSequentialStateByTag(EPGXFlowChannel::UI, RedundantBatch);
	TestTrue(TEXT("Redundant batch rejected with matching result code"), Rejected.Code == EPGXFlowResultCode::RedundantState);
	TestTrue(TEXT("Rejected batch does not mutate current state"),
		GameFlow->GetCurrentFlowTag(EPGXFlowChannel::UI) == BeforeRejectedBatch);
	return true;
}

PGX_TEST_GAME(FPGXGameFlow_BridgeMessageEmittedAutomationTest)
{
	PGXGameFlowAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXGameFlowSubsystem* GameFlow = nullptr;
	if (!PGXGameFlowAutomation::PreparePermissiveGameFlow(*this, Fixture.Get(), GameFlow))
	{
		return true;
	}

	UPGXMessageSubsystem* MessageSubsystem = PGXGameFlowAutomation::FindMessageSubsystem(*this, Fixture.Get());
	if (!MessageSubsystem)
	{
		return true;
	}

	const FGameplayTag BridgeStateChangedTag = PGXGameFlowAutomation::GetBridgeGameFlowStateChangedTag(*this);
	if (!BridgeStateChangedTag.IsValid())
	{
		return true;
	}

	bool bBridgeMessageEmitted = false;
	FPGXBridgeGameFlowChanged LastMessage;
	FPGXMessageListenerHandle Handle = MessageSubsystem->RegisterListener<FPGXBridgeGameFlowChanged>(
		BridgeStateChangedTag,
		[&bBridgeMessageEmitted, &LastMessage](FGameplayTag, const FPGXBridgeGameFlowChanged& Message)
		{
			bBridgeMessageEmitted = true;
			LastMessage = Message;
		});

	GameFlow->SetStateByTag(EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_Boot);
	bBridgeMessageEmitted = false;
	GameFlow->SetStateByTag(EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_MainMenu);
	Handle.Unregister();

	TestTrue(TEXT("BridgeMessageEmitted"), bBridgeMessageEmitted);
	TestTrue(TEXT("Bridge payload old state"), LastMessage.OldState == TAG_PGX_GameFlow_State_Boot.GetTag());
	TestTrue(TEXT("Bridge payload new state"), LastMessage.NewState == TAG_PGX_GameFlow_State_MainMenu.GetTag());
	return true;
}

PGX_TEST_GAME(FPGXGameFlow_ConsoleMutationGatedAutomationTest)
{
	PGXGameFlowAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXGameFlowSubsystem* GameFlow = nullptr;
	if (!PGXGameFlowAutomation::PreparePermissiveGameFlow(*this, Fixture.Get(), GameFlow))
	{
		return true;
	}

	UPGXGameFlowConfig* Config = PGXGameFlowAutomation::MakeConfig(TEXT("PGXGameFlow_ConsoleGateConfig"));
	Config->bAllowConsoleMutations = false;
	GameFlow->InjectTestConfig(Config);
	TestFalse(TEXT("Console mutations default disabled"), GameFlow->IsConsoleMutationAllowedForTesting());

	Config->bAllowConsoleMutations = true;
	GameFlow->InjectTestConfig(Config);
	TestTrue(TEXT("Console mutations enabled by config in non-shipping dev automation"), GameFlow->IsConsoleMutationAllowedForTesting());
	return true;
}

PGX_TEST_GAME(FPGXGameFlow_HistoryPolicyAutomationTest)
{
	PGXGameFlowAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXGameFlowSubsystem* GameFlow = nullptr;
	if (!PGXGameFlowAutomation::PreparePermissiveGameFlow(*this, Fixture.Get(), GameFlow, 4))
	{
		return true;
	}

	TestEqual(TEXT("HistoryBoundedByPolicy resolved config budget"),
		GameFlow->GetResolvedMaxHistoryDepthForTesting(), 4);

	GameFlow->SetStateByTag(EPGXFlowChannel::Actors, TAG_PGX_GameFlow_State_Boot);
	GameFlow->SetStateByTag(EPGXFlowChannel::Actors, TAG_PGX_GameFlow_State_MainMenu);
	GameFlow->SetStateByTag(EPGXFlowChannel::Actors, TAG_PGX_GameFlow_State_Loading);
	GameFlow->SetStateByTag(EPGXFlowChannel::Actors, TAG_PGX_GameFlow_State_InWorld);
	GameFlow->SetStateByTag(EPGXFlowChannel::Actors, TAG_PGX_GameFlow_State_Paused);
	TestTrue(TEXT("HistoryBoundedByPolicy trims to config budget"),
		GameFlow->GetChannelHistory(EPGXFlowChannel::Actors).Num() <= 4);

	GameFlow->OverrideResolvedMaxHistoryDepthForTesting(2);
	TestEqual(TEXT("ProfileClampApplied resolved test clamp"),
		GameFlow->GetResolvedMaxHistoryDepthForTesting(), 2);
	TestTrue(TEXT("ProfileClampApplied trims existing history"),
		GameFlow->GetChannelHistory(EPGXFlowChannel::Actors).Num() <= 2);
	return true;
}

// ============================================================================
// EN: IPGXObservable adoption schema validation tests
//     for the 2 PGXGameFlow DA classes via shared template.
// ES: tests de validacion IPGXObservable de las 2
//     clases DA PGXGameFlow via template compartido.
// ============================================================================

#include "Observability/PGXObservable.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace PGXGameFlowObservabilityAutomation
{
	template<typename TConfigClass>
	bool ValidateObservableContract(FAutomationTestBase& Test, const TCHAR* ExpectedTypeName)
	{
		TConfigClass* Instance = NewObject<TConfigClass>(
			GetTransientPackage(), TConfigClass::StaticClass(), NAME_None, RF_Transient);
		if (!Test.TestNotNull(*FString::Printf(TEXT("%s instance"), ExpectedTypeName), Instance)) { return false; }

		const FName SchemaVersion = Instance->GetSchemaVersion();
		Test.TestEqual(*FString::Printf(TEXT("%s::GetSchemaVersion is 1.0"), ExpectedTypeName), SchemaVersion, FName(TEXT("1.0")));

		const FPGXSchemaDescriptor Descriptor = Instance->GetSchemaDescriptor();
		Test.TestEqual(*FString::Printf(TEXT("%s schema TypeName matches"), ExpectedTypeName), Descriptor.TypeName, TConfigClass::StaticClass()->GetFName());
		Test.TestEqual(*FString::Printf(TEXT("%s schema SchemaVersion matches"), ExpectedTypeName), Descriptor.SchemaVersion, SchemaVersion);
		Test.TestTrue(*FString::Printf(TEXT("%s schema Fields > 0"), ExpectedTypeName), Descriptor.Fields.Num() > 0);

		const FPGXJsonValue Envelope = Instance->ToJson();
		Test.TestFalse(*FString::Printf(TEXT("%s ToJson envelope non-empty"), ExpectedTypeName), Envelope.IsEmpty());
		Test.TestTrue(*FString::Printf(TEXT("%s envelope contains type"), ExpectedTypeName),
			Envelope.JsonString.Contains(FString::Printf(TEXT("\"type\":\"%s\""), ExpectedTypeName)));
		Test.TestTrue(*FString::Printf(TEXT("%s envelope contains 1.0 version"), ExpectedTypeName),
			Envelope.JsonString.Contains(TEXT("\"version\":\"1.0\"")));

		const FPGXJsonValue EmptyJson;
		Test.TestFalse(*FString::Printf(TEXT("%s FromJson rejects empty"), ExpectedTypeName), Instance->FromJson(EmptyJson).bValid);
		Test.TestTrue(*FString::Printf(TEXT("%s FromJson accepts envelope"), ExpectedTypeName), Instance->FromJson(Envelope).bValid);

		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXGameFlowConfigObservableSchema,
	"PGX.GameFlow.ConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXGameFlowConfigObservableSchema::RunTest(const FString& /*Parameters*/)
{
	return PGXGameFlowObservabilityAutomation::ValidateObservableContract<UPGXGameFlowConfig>(*this, TEXT("PGXGameFlowConfig"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXFlowRulesConfigObservableSchema,
	"PGX.GameFlow.FlowRulesConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPGXFlowRulesConfigObservableSchema::RunTest(const FString& /*Parameters*/)
{
	return PGXGameFlowObservabilityAutomation::ValidateObservableContract<UPGXFlowRulesConfig>(*this, TEXT("PGXFlowRulesConfig"));
}

#endif // WITH_DEV_AUTOMATION_TESTS
