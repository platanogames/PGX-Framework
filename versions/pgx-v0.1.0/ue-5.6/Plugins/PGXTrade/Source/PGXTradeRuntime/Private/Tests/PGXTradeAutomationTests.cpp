// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "PGXTradeTestUtility.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PGXTradeAutomationTestsInternal
{
	struct FScopedGameInstanceFixture
	{
		explicit FScopedGameInstanceFixture(FAutomationTestBase& InTest)
			: Test(InTest)
		{
			if (!GEngine)
			{
				Test.AddError(TEXT("PGXTrade automation setup failed: engine is unavailable."));
				return;
			}

			GameInstance = NewObject<UGameInstance>(GEngine, UGameInstance::StaticClass(), NAME_None, RF_Transient);
			if (!GameInstance)
			{
				Test.AddError(TEXT("PGXTrade automation setup failed: could not create transient GameInstance."));
				return;
			}

			GameInstance->AddToRoot();
			GameInstance->InitializeStandalone(TEXT("PGXTradeAutomationWorld"));
			if (!GameInstance->GetWorld())
			{
				Test.AddError(TEXT("PGXTrade automation setup failed: standalone GameInstance has no World."));
			}
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
		bool bShutdown = false;
	};

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

	template <typename FuncType>
	static bool RunWorldBackedTest(FAutomationTestBase& Test, FuncType Func)
	{
		FScopedGameInstanceFixture Fixture(Test);
		UWorld* World = Fixture.GetWorld();
		if (!World)
		{
			return true;
		}
		TArray<FString> OutIssues;
		const bool bPassed = Func(World, OutIssues);
		ForwardIssues(Test, OutIssues);
		return bPassed;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTrade_SubsystemInitializeAutomationTest,
	"PGX.Trade.SubsystemInitialize",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXTrade_SubsystemInitializeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXTradeAutomationTestsInternal::RunWorldBackedTest(*this, &UPGXTradeTestUtility::SubsystemInitializeTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTrade_ConfigDefaultsAutomationTest,
	"PGX.Trade.ConfigDefaults",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXTrade_ConfigDefaultsAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	UObject* Context = GEngine;
	const bool bPassed = UPGXTradeTestUtility::ConfigDefaultsTest(Context, OutIssues);
	PGXTradeAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTrade_ActorRegistrationAutomationTest,
	"PGX.Trade.ActorRegistration",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXTrade_ActorRegistrationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXTradeAutomationTestsInternal::RunWorldBackedTest(*this, &UPGXTradeTestUtility::ActorRegistrationTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTrade_OfferLifecycleAutomationTest,
	"PGX.Trade.OfferLifecycle",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXTrade_OfferLifecycleAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXTradeAutomationTestsInternal::RunWorldBackedTest(*this, &UPGXTradeTestUtility::OfferLifecycleTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTrade_ReputationReasonGuardAutomationTest,
	"PGX.Trade.ReputationReasonGuard",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXTrade_ReputationReasonGuardAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXTradeAutomationTestsInternal::RunWorldBackedTest(*this, &UPGXTradeTestUtility::ReputationReasonGuardTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTrade_InformationFreshnessAutomationTest,
	"PGX.Trade.InformationFreshness",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXTrade_InformationFreshnessAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXTradeAutomationTestsInternal::RunWorldBackedTest(*this, &UPGXTradeTestUtility::InformationFreshnessTest);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTrade_NativeTagsRegisteredAutomationTest,
	"PGX.Trade.NativeTagsRegistered",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXTrade_NativeTagsRegisteredAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	UObject* Context = GEngine;
	const bool bPassed = UPGXTradeTestUtility::NativeTagsRegisteredTest(Context, OutIssues);
	PGXTradeAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTrade_ObservableConfigSchemaAutomationTest,
	"PGX.Trade.ObservableConfigSchema",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXTrade_ObservableConfigSchemaAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	UObject* Context = GEngine;
	const bool bPassed = UPGXTradeTestUtility::ObservableConfigSchemaTest(Context, OutIssues);
	PGXTradeAutomationTestsInternal::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXTrade_ValidateAllAutomationTest,
	"PGX.Trade.ValidateAll",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXTrade_ValidateAllAutomationTest::RunTest(const FString& /*Parameters*/)
{
	return PGXTradeAutomationTestsInternal::RunWorldBackedTest(*this, &UPGXTradeTestUtility::ValidateAll);
}

#endif // WITH_DEV_AUTOMATION_TESTS
