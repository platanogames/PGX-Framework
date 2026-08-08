// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "Base/PGXDataAsset.h"
#include "Data/PGXObjectDataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameplayTagsManager.h"
#include "Misc/AutomationTest.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Registry/PGXRegistryTestUtility.h"
#include "Registry/PGXRegistryTypes.h"
#include "Tables/PGXTableTypes.h"

namespace PGXRegistryAutomation
{
	FGameplayTag AddTestTag(const TCHAR* TagName)
	{
		return UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName), FString(TEXT("PGX Registry automation test tag")));
	}

	UGameInstance* FindGameInstance()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			UWorld* World = WorldContext.World();
			if (World && World->GetGameInstance())
			{
				return World->GetGameInstance();
			}
		}

		return nullptr;
	}

	UPGXDataRegistrySubsystem* FindSubsystem(FAutomationTestBase& Test, UGameInstance*& OutGameInstance)
	{
		OutGameInstance = FindGameInstance();
		if (!OutGameInstance)
		{
			Test.AddError(TEXT("PGXRegistry automation setup failed: no GameInstance available. Run as a game/client automation test."));
			return nullptr;
		}

		UPGXDataRegistrySubsystem* Registry = OutGameInstance->GetSubsystem<UPGXDataRegistrySubsystem>();
		if (!Registry)
		{
			Test.AddError(TEXT("PGXRegistry automation setup failed: UPGXDataRegistrySubsystem missing."));
		}
		return Registry;
	}

	UPGXObjectDataAsset* MakeObjectAsset(const TCHAR* Name, FGameplayTag CategoryTag)
	{
		UPGXObjectDataAsset* Asset = NewObject<UPGXObjectDataAsset>(GetTransientPackage(), FName(Name), RF_Transient);
		Asset->CategoryTag = CategoryTag;
		Asset->AssetId = FName(Name);
		Asset->DisplayName = FText::FromString(FString(Name));
		return Asset;
	}

	FPGXCompositeRegistryKey MakeCompositeKey(FGameplayTag DatabaseTag, FGameplayTag CategoryTag, FGameplayTag ItemTag)
	{
		FPGXCompositeRegistryKey Key;
		Key.DatabaseTag = DatabaseTag;
		Key.CategoryTag = CategoryTag;
		Key.ItemTag = ItemTag;
		return Key;
	}

	void EnsureCleanDatabase(UPGXDataRegistrySubsystem* Registry, FGameplayTag DatabaseTag)
	{
		if (!Registry->HasDatabase(DatabaseTag))
		{
			Registry->CreateDatabase(DatabaseTag, UPGXObjectDataAsset::StaticClass(), false);
			return;
		}

		TArray<FPGXRegistryEntry> Entries = Registry->GetAllEntries(DatabaseTag);
		for (const FPGXRegistryEntry& Entry : Entries)
		{
			Registry->UnregisterAsset(DatabaseTag, Entry.ItemTag);
		}
	}

	UDataTable* MakeRegistryTable(const TCHAR* Name, FGameplayTag CategoryTag, FGameplayTag ItemTag, UPGXDataAsset* Asset)
	{
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage(), FName(Name), RF_Transient);
		Table->RowStruct = FPGXRegistryCategoryRow::StaticStruct();

		FPGXRegistryCategoryRow Row;
		Row.CategoryTag = CategoryTag;
		Row.ItemsByTag.Add(ItemTag, TSoftObjectPtr<UPGXDataAsset>(Asset));
		Table->AddRow(FName(TEXT("Category")), Row);
		return Table;
	}

	void TestConflictPolicy(FAutomationTestBase& Test, UPGXDataRegistrySubsystem* Registry, FGameplayTag DatabaseTag,
		EPGXRegistryConflictPolicy Policy, const TCHAR* PolicyName, FGameplayTag ExpectedCategory, bool bExpectedSecondIngest)
	{
		const FGameplayTag ItemTag = AddTestTag(*FString::Printf(TEXT("PGX.Test.Registry.Automation.Conflict.%s.Item"), PolicyName));
		const FGameplayTag FirstCategory = AddTestTag(*FString::Printf(TEXT("PGX.Test.Registry.Automation.Conflict.%s.First"), PolicyName));
		const FGameplayTag LastCategory = AddTestTag(*FString::Printf(TEXT("PGX.Test.Registry.Automation.Conflict.%s.Last"), PolicyName));

		EnsureCleanDatabase(Registry, DatabaseTag);
		UPGXObjectDataAsset* FirstAsset = MakeObjectAsset(*FString::Printf(TEXT("PGXRegistry_%s_First"), PolicyName), FirstCategory);
		UPGXObjectDataAsset* LastAsset = MakeObjectAsset(*FString::Printf(TEXT("PGXRegistry_%s_Last"), PolicyName), LastCategory);

		UDataTable* FirstTable = MakeRegistryTable(*FString::Printf(TEXT("PGXRegistry_%s_FirstTable"), PolicyName), FirstCategory, ItemTag, FirstAsset);
		UDataTable* LastTable = MakeRegistryTable(*FString::Printf(TEXT("PGXRegistry_%s_LastTable"), PolicyName), LastCategory, ItemTag, LastAsset);

		Test.TestTrue(FString::Printf(TEXT("%s initial ingest succeeds"), PolicyName), Registry->IngestDataTable(DatabaseTag, FirstTable, Policy));
		Test.TestEqual(FString::Printf(TEXT("%s second ingest result"), PolicyName), Registry->IngestDataTable(DatabaseTag, LastTable, Policy), bExpectedSecondIngest);

		const FPGXRegistryEntry* Entry = Registry->FindEntry(DatabaseTag, ItemTag);
		Test.TestNotNull(FString::Printf(TEXT("%s conflict entry remains queryable"), PolicyName), Entry);
		if (Entry)
		{
			Test.TestEqual(FString::Printf(TEXT("%s conflict policy selected category"), PolicyName), Entry->CategoryTag, ExpectedCategory);
			Test.TestNotNull(FString::Printf(TEXT("%s selected composite key remains coherent"), PolicyName),
				Registry->FindByCompositeKey(MakeCompositeKey(DatabaseTag, ExpectedCategory, ItemTag)));

			const FGameplayTag RejectedCategory = (ExpectedCategory == FirstCategory) ? LastCategory : FirstCategory;
			Test.TestNull(FString::Printf(TEXT("%s rejected composite key removed"), PolicyName),
				Registry->FindByCompositeKey(MakeCompositeKey(DatabaseTag, RejectedCategory, ItemTag)));
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXRegistry_IndexCoherenceAutomationTest,
	"PGX.Core.Registry.IndexCoherence",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXRegistry_IndexCoherenceAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UGameInstance* GameInstance = nullptr;
	UPGXDataRegistrySubsystem* Registry = PGXRegistryAutomation::FindSubsystem(*this, GameInstance);
	if (!Registry)
	{
		return true;
	}

	const FGameplayTag DatabaseTag = PGXRegistryAutomation::AddTestTag(TEXT("PGX.Test.Registry.Automation.Index.Database"));
	const FGameplayTag CategoryTag = PGXRegistryAutomation::AddTestTag(TEXT("PGX.Test.Registry.Automation.Index.Category"));
	const FGameplayTag ItemTag = PGXRegistryAutomation::AddTestTag(TEXT("PGX.Test.Registry.Automation.Index.Item"));
	PGXRegistryAutomation::EnsureCleanDatabase(Registry, DatabaseTag);
	const int32 ReverseIndexBaseline = Registry->GetCompositeReverseIndexCount();

	UPGXObjectDataAsset* Asset = PGXRegistryAutomation::MakeObjectAsset(TEXT("PGXRegistry_IndexAsset"), CategoryTag);
	TestTrue(TEXT("RegisterAsset succeeds"), Registry->RegisterAsset(DatabaseTag, Asset, ItemTag));
	TestNotNull(TEXT("FindEntry sees registered item"), Registry->FindEntry(DatabaseTag, ItemTag));
	TestNotNull(TEXT("Composite lookup sees registered item"), Registry->FindByCompositeKey(PGXRegistryAutomation::MakeCompositeKey(DatabaseTag, CategoryTag, ItemTag)));
	TestTrue(TEXT("Composite reverse index records registered item"), Registry->GetCompositeReverseIndexCount() >= ReverseIndexBaseline + 1);
	TestEqual(TEXT("ResolveAsset returns cached asset"), Registry->ResolveAsset(DatabaseTag, ItemTag), Cast<UPGXDataAsset>(Asset));

	bool bRequestLoadInvoked = false;
	UPGXDataAsset* RequestedAsset = nullptr;
	Registry->RequestLoad(DatabaseTag, ItemTag, FOnPGXAssetLoaded::CreateLambda(
		[&bRequestLoadInvoked, &RequestedAsset](const FGameplayTag& /*LoadedItemTag*/, UPGXDataAsset* LoadedAsset)
		{
			bRequestLoadInvoked = true;
			RequestedAsset = LoadedAsset;
		}));
	TestTrue(TEXT("RequestLoad invokes callback for cached asset"), bRequestLoadInvoked);
	TestEqual(TEXT("RequestLoad returns cached asset"), RequestedAsset, Cast<UPGXDataAsset>(Asset));

	TestTrue(TEXT("UnregisterAsset succeeds"), Registry->UnregisterAsset(DatabaseTag, ItemTag));
	TestNull(TEXT("FindEntry removed after unregister"), Registry->FindEntry(DatabaseTag, ItemTag));
	TestNull(TEXT("Composite lookup removed after unregister"), Registry->FindByCompositeKey(PGXRegistryAutomation::MakeCompositeKey(DatabaseTag, CategoryTag, ItemTag)));
	TestEqual(TEXT("Composite reverse index restored after unregister"), Registry->GetCompositeReverseIndexCount(), ReverseIndexBaseline);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXRegistry_ConflictPolicyAutomationTest,
	"PGX.Core.Registry.ConflictPolicies",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXRegistry_ConflictPolicyAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UGameInstance* GameInstance = nullptr;
	UPGXDataRegistrySubsystem* Registry = PGXRegistryAutomation::FindSubsystem(*this, GameInstance);
	if (!Registry)
	{
		return true;
	}

	const FGameplayTag DatabaseTag = PGXRegistryAutomation::AddTestTag(TEXT("PGX.Test.Registry.Automation.Conflict.Database"));
	const FGameplayTag FailExpectedCategory = PGXRegistryAutomation::AddTestTag(TEXT("PGX.Test.Registry.Automation.Conflict.FailOnConflict.First"));
	const FGameplayTag FirstWinsExpectedCategory = PGXRegistryAutomation::AddTestTag(TEXT("PGX.Test.Registry.Automation.Conflict.FirstWins.First"));
	const FGameplayTag LastWinsExpectedCategory = PGXRegistryAutomation::AddTestTag(TEXT("PGX.Test.Registry.Automation.Conflict.LastWins.Last"));

	PGXRegistryAutomation::TestConflictPolicy(*this, Registry, DatabaseTag,
		EPGXRegistryConflictPolicy::FailOnConflict, TEXT("FailOnConflict"), FailExpectedCategory, false);
	PGXRegistryAutomation::TestConflictPolicy(*this, Registry, DatabaseTag,
		EPGXRegistryConflictPolicy::FirstWins, TEXT("FirstWins"), FirstWinsExpectedCategory, false);
	PGXRegistryAutomation::TestConflictPolicy(*this, Registry, DatabaseTag,
		EPGXRegistryConflictPolicy::LastWins, TEXT("LastWins"), LastWinsExpectedCategory, true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXRegistry_BenchmarkEntrypointSmokeAutomationTest,
	"PGX.Core.Registry.BenchmarkEntrypointSmoke",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXRegistry_BenchmarkEntrypointSmokeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UGameInstance* GameInstance = nullptr;
	UPGXDataRegistrySubsystem* Registry = PGXRegistryAutomation::FindSubsystem(*this, GameInstance);
	if (!Registry || !GameInstance)
	{
		return true;
	}

	TArray<FString> Issues;
	UObject* WorldContextObject = GameInstance->GetWorld() ? Cast<UObject>(GameInstance->GetWorld()) : Cast<UObject>(GameInstance);
	const bool bSmokePassed = UPGXRegistryTestUtility::RunLargeRegistryBenchmark(WorldContextObject, 16, Issues);
	for (const FString& Issue : Issues)
	{
		AddInfo(Issue);
	}
	TestTrue(TEXT("Large registry benchmark entrypoint smoke runs without heavy AAA counts"), bSmokePassed);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
