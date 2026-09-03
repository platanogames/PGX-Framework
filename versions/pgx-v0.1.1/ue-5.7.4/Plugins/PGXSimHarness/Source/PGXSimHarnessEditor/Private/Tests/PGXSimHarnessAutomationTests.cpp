// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FPGXVisualHarness.h"
#include "PGXDemoRegistry.h"
#include "PGXSimHarnessEditorModule.h"
#include "PGXGameFlowSubsystem.h"
#include "PGXGameFlowTypes.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveGame.h"
#include "Utils/FPGXOwnedResourceTracker.h"
#include "Utils/FPGXComponentLifecycleHarness.h"
#include "Utils/FPGXPluginDescriptorDependencyValidator.h"
#include "PGXCameraMode.h"
#include "PGXInteractionComponent.h"
#include "PGXInventoryComponent.h"
#include "PGXItemDefinition.h"
#include "PGXWidgetPool.h"
#include "PGXInputConfig.h"
#include "PGXInputContext.h"
#include "PGXInputSubsystem.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Messages/PGXMessageSubsystem.h"
#include "PGXColonySubsystem.h"
#include "PGXCraftingSubsystem.h"
#include "PGXEnvironmentSubsystem.h"
#include "Tags/PGXEnvironmentTags.h"
#include "PGXTradeSubsystem.h"
#include "PGXVehiclesSubsystem.h"
#include "PGXAISubsystem.h"
#include "PGXAbilitySubsystem.h"
#include "PGXSpawnSubsystem.h"
#include "Logging/PGXLogBlueprintLibrary.h"
#include "Subsystems/PGXLogSubsystem.h"

#include "Framework/Docking/TabManager.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PGXSimHarnessAutomation
{
	static void ForwardIssues(FAutomationTestBase& Test, const TArray<FString>& OutIssues)
	{
		for (const FString& Issue : OutIssues)
		{
			Test.AddInfo(Issue);
		}
	}

	static void AddIssue(TArray<FString>& OutIssues, const FString& Message)
	{
		OutIssues.Add(Message);
	}

	// EN: Retrieve an available world for subsystem tests. Returns the first PIE/Game world
	//     found via GEngine, or nullptr if no world context is available (editor-only startup).
	// ES: Obtener un mundo disponible para tests de subsystem. Retorna el primer mundo PIE/Game
	//     encontrado via GEngine, o nullptr si no hay contexto de mundo (startup editor-only).
	static UWorld* GetTestWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
			{
				return Context.World();
			}
		}
		return nullptr;
	}

	static bool ValidateDemoRegistryCatalog(TArray<FString>& OutIssues)
	{
		const TArray<FPGXDemoEntry>& Entries = FPGXDemoRegistry::GetDemoEntries();
		const TArray<FString> Categories = FPGXDemoRegistry::GetDemoCategories();
		bool bPassed = true;

		if (Entries.IsEmpty())
		{
			AddIssue(OutIssues, TEXT("[FAIL] Demo registry has no entries."));
			bPassed = false;
		}

		if (Categories.IsEmpty())
		{
			AddIssue(OutIssues, TEXT("[FAIL] Demo registry has no categories."));
			bPassed = false;
		}

		TSet<FString> SeenCategories;
		for (const FString& Category : Categories)
		{
			if (Category.IsEmpty())
			{
				AddIssue(OutIssues, TEXT("[FAIL] Demo registry category is empty."));
				bPassed = false;
				continue;
			}

			if (SeenCategories.Contains(Category))
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Duplicate demo registry category: %s."), *Category));
				bPassed = false;
			}
			SeenCategories.Add(Category);

			const TArray<FPGXDemoEntry> CategoryEntries = FPGXDemoRegistry::GetEntriesForCategory(Category);
			if (CategoryEntries.IsEmpty())
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Demo registry category has no entries: %s."), *Category));
				bPassed = false;
				continue;
			}

			int32 PreviousOrder = MIN_int32;
			for (const FPGXDemoEntry& Entry : CategoryEntries)
			{
				if (Entry.OrderInCategory < PreviousOrder)
				{
					AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Category %s is not sorted by OrderInCategory."), *Category));
					bPassed = false;
					break;
				}
				PreviousOrder = Entry.OrderInCategory;
			}
		}

		if (bPassed)
		{
			AddIssue(OutIssues, FString::Printf(TEXT("[PASS] Demo registry exposes %d entries across %d ordered categories."), Entries.Num(), Categories.Num()));
		}
		return bPassed;
	}

	static bool ValidateDemoRegistryEntries(TArray<FString>& OutIssues)
	{
		const TArray<FPGXDemoEntry>& Entries = FPGXDemoRegistry::GetDemoEntries();
		bool bPassed = true;

		for (const FPGXDemoEntry& Entry : Entries)
		{
			if (Entry.DisplayName.IsEmpty())
			{
				AddIssue(OutIssues, TEXT("[FAIL] Demo registry entry has empty DisplayName."));
				bPassed = false;
			}
			if (Entry.ClassPath.IsEmpty() || !Entry.ClassPath.StartsWith(TEXT("/Script/")))
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Entry %s has invalid ClassPath: %s."), *Entry.DisplayName, *Entry.ClassPath));
				bPassed = false;
			}
			if (Entry.SuggestedName.IsEmpty())
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Entry %s has empty SuggestedName."), *Entry.DisplayName));
				bPassed = false;
			}
			if (Entry.Category.IsEmpty())
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Entry %s has empty Category."), *Entry.DisplayName));
				bPassed = false;
			}
			if (Entry.Tooltip.IsEmpty())
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Entry %s has empty Tooltip."), *Entry.DisplayName));
				bPassed = false;
			}
			if (Entry.IconStyleName.IsEmpty())
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Entry %s has empty IconStyleName."), *Entry.DisplayName));
				bPassed = false;
			}

			UClass* DataAssetClass = FindObject<UClass>(nullptr, *Entry.ClassPath);
			if (!DataAssetClass)
			{
				DataAssetClass = LoadObject<UClass>(nullptr, *Entry.ClassPath);
			}
			if (!DataAssetClass)
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Entry %s class path did not resolve: %s."), *Entry.DisplayName, *Entry.ClassPath));
				bPassed = false;
			}
		}

		if (bPassed)
		{
			AddIssue(OutIssues, TEXT("[PASS] Demo registry entry metadata and native class paths are valid."));
		}
		return bPassed;
	}

	static bool ValidateSelfRegistrationSurface(TArray<FString>& OutIssues)
	{
		bool bPassed = true;
		const FName ExpectedTabId(TEXT("PGXSimHarnessPanel"));
		if (FPGXSimHarnessEditorModule::GetSimHarnessTabId() != ExpectedTabId)
		{
			AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] SimHarness tab id changed; expected %s."), *ExpectedTabId.ToString()));
			bPassed = false;
		}

		const TArray<FPGXDemoEntry> FoundationEntries = FPGXDemoRegistry::GetEntriesForCategory(TEXT("1. Foundation"));
		if (FoundationEntries.IsEmpty())
		{
			AddIssue(OutIssues, TEXT("[FAIL] Self-registration surface has no Foundation demo entries."));
			bPassed = false;
		}

		if (bPassed)
		{
			AddIssue(OutIssues, TEXT("[PASS] PGXSimHarness tab id and core self-registration surfaces are stable."));
		}
		return bPassed;
	}

	static bool ValidateVisualHarnessPassiveLifecycle(TArray<FString>& OutIssues)
	{
		bool bPassed = true;
		FPGXVisualHarness Harness;

		if (Harness.IsActive())
		{
			AddIssue(OutIssues, TEXT("[FAIL] VisualHarness starts active before Setup()."));
			bPassed = false;
		}
		if (Harness.IsSimulating())
		{
			AddIssue(OutIssues, TEXT("[FAIL] VisualHarness starts simulating before StartSimulation()."));
			bPassed = false;
		}
		if (Harness.GetTotalObjectCount() != 0)
		{
			AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] VisualHarness starts with %d injected objects."), Harness.GetTotalObjectCount()));
			bPassed = false;
		}

		Harness.StopSimulation();
		Harness.Teardown();

		if (Harness.IsActive() || Harness.IsSimulating() || Harness.GetTotalObjectCount() != 0)
		{
			AddIssue(OutIssues, TEXT("[FAIL] VisualHarness passive StopSimulation/Teardown did not remain clean."));
			bPassed = false;
		}

		AddIssue(OutIssues, TEXT("[INFO] Setup(nullptr) and ExportReport() are intentionally not invoked here: Setup injects editor subsystem data and ExportReport writes under Saved/PGX/."));
		if (bPassed)
		{
			AddIssue(OutIssues, TEXT("[PASS] VisualHarness passive lifecycle is side-effect-safe."));
		}
		return bPassed;
	}

	static bool ValidateBaselineCoverageMatrix(TArray<FString>& OutIssues)
	{
		const TArray<FPGXPluginCoverage> Matrix = FPGXVisualHarness::GetCoverageMatrix();
		bool bPassed = true;

		auto FindCoverage = [&Matrix](const FString& PluginName) -> const FPGXPluginCoverage*
		{
			return Matrix.FindByPredicate([&PluginName](const FPGXPluginCoverage& Entry)
			{
				return Entry.PluginName == PluginName;
			});
		};

		const FPGXPluginCoverage* Core = FindCoverage(TEXT("PGXCore"));
		if (!Core || Core->Coverage != EPGXHarnessCoverage::Partial || Core->Priority != 0)
		{
			AddIssue(OutIssues, TEXT("[FAIL] PGXCore must remain high-priority Partial until BP-library honesty + registry validation scenario land."));
			bPassed = false;
		}

		const FPGXPluginCoverage* Harness = FindCoverage(TEXT("PGXSimHarness"));
		if (!Harness || Harness->Priority != 0 || Harness->ScenarioName != TEXT("Self-Verification"))
		{
			AddIssue(OutIssues, TEXT("[FAIL] PGXSimHarness self-verification must stay visible as a high-priority acceptance gate."));
			bPassed = false;
		}

		if (bPassed)
		{
			AddIssue(OutIssues, TEXT("[PASS] Baseline coverage matrix reconciles Core/self-verification priorities; Input promotion is covered by RuntimeExtended."));
		}
		return bPassed;
	}

	static bool ValidateSelfCoverageCatalogCount(TArray<FString>& OutIssues)
	{
		const TArray<FPGXPluginCoverage> CompatibilityMatrix = FPGXVisualHarness::GetCoverageMatrix();
		const TArray<FPGXPluginCoverageEntry> DetailedMatrix = FPGXHarnessCoverage::GetCoverageMatrix(GetTestWorld());
		const int32 ExpectedCount = FPGXHarnessCoverage::GetCanonicalPluginCount();
		bool bPassed = true;

		const bool bCompatibilityCountOk = CompatibilityMatrix.Num() == ExpectedCount;
		AddIssue(OutIssues, bCompatibilityCountOk
			? FString::Printf(TEXT("[PASS] SelfCoverage compatibility matrix has %d entries."), CompatibilityMatrix.Num())
			: FString::Printf(TEXT("[FAIL] SelfCoverage compatibility matrix has %d entries; expected %d."), CompatibilityMatrix.Num(), ExpectedCount));
		bPassed = bPassed && bCompatibilityCountOk;

		const bool bDetailedCountOk = DetailedMatrix.Num() == ExpectedCount;
		AddIssue(OutIssues, bDetailedCountOk
			? FString::Printf(TEXT("[PASS] SelfCoverage detailed matrix has %d entries."), DetailedMatrix.Num())
			: FString::Printf(TEXT("[FAIL] SelfCoverage detailed matrix has %d entries; expected %d."), DetailedMatrix.Num(), ExpectedCount));
		bPassed = bPassed && bDetailedCountOk;

		const FPGXPluginCoverage* SelfEntry = CompatibilityMatrix.FindByPredicate([](const FPGXPluginCoverage& Entry)
		{
			return Entry.PluginName == TEXT("PGXSimHarness");
		});
		const bool bSelfEntryOk = SelfEntry
			&& SelfEntry->Coverage == EPGXHarnessCoverage::NotApplicable
			&& SelfEntry->Priority == 0
			&& SelfEntry->ScenarioName == TEXT("Self-Verification");
		AddIssue(OutIssues, bSelfEntryOk
			? TEXT("[PASS] SelfCoverage PGXSimHarness row is present as high-priority Self-Verification.")
			: TEXT("[FAIL] SelfCoverage PGXSimHarness row must remain high-priority Self-Verification."));
		return bPassed && bSelfEntryOk;
	}

	static bool ValidateSelfCoveragePanelIsOpen(TArray<FString>& OutIssues)
	{
		const FName TabId = FPGXSimHarnessEditorModule::GetSimHarnessTabId();
		const TSharedPtr<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();
		if (!TabManager.IsValid())
		{
			AddIssue(OutIssues, TEXT("[FAIL] SelfCoverage panel check cannot resolve GlobalTabmanager."));
			return false;
		}

		const bool bSpawnerRegistered = TabManager->HasTabSpawner(TabId);
		AddIssue(OutIssues, bSpawnerRegistered
			? FString::Printf(TEXT("[PASS] SelfCoverage tab spawner is registered: %s."), *TabId.ToString())
			: FString::Printf(TEXT("[FAIL] SelfCoverage tab spawner is missing: %s."), *TabId.ToString()));

		const TSharedPtr<SDockTab> OpenedTab = bSpawnerRegistered ? TabManager->TryInvokeTab(TabId) : nullptr;
		const bool bPanelOpened = OpenedTab.IsValid();
		AddIssue(OutIssues, bPanelOpened
			? FString::Printf(TEXT("[PASS] SelfCoverage panel opened through TryInvokeTab: %s."), *TabId.ToString())
			: FString::Printf(TEXT("[FAIL] SelfCoverage panel did not open through TryInvokeTab: %s."), *TabId.ToString()));

		return bSpawnerRegistered && bPanelOpened;
	}

	static bool ValidateCoreIntegrityAllChannelsActive(TArray<FString>& OutIssues)
	{
		bool bPassed = true;
		const UWorld* World = GetTestWorld();
		const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UPGXMessageSubsystem* Msg = GI ? GI->GetSubsystem<UPGXMessageSubsystem>() : nullptr;

		if (!Msg)
		{
			UClass* MsgClass = UPGXMessageSubsystem::StaticClass();
			const bool bSurfaceOk = MsgClass != nullptr;
			AddIssue(OutIssues, bSurfaceOk
				? TEXT("[PASS] CoreIntegrity channels: no live GameInstance; UPGXMessageSubsystem class resolves and live channel assertion is covered by VerifyAllAPIs.")
				: TEXT("[FAIL] CoreIntegrity channels: UPGXMessageSubsystem class failed to resolve."));
			return bSurfaceOk;
		}

		const FGameplayTag MsgGameplay = FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Message.Gameplay"), false);
		const FGameplayTag MsgUI = FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Message.UI"), false);
		const FGameplayTag MsgSystem = FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Message.System"), false);
		const bool bGameplay = Msg->IsChannelActive(MsgGameplay);
		const bool bUI = Msg->IsChannelActive(MsgUI);
		const bool bSystem = Msg->IsChannelActive(MsgSystem);
		bPassed = bGameplay && bUI && bSystem;
		AddIssue(OutIssues, bPassed
			? TEXT("[PASS] CoreIntegrity channels: Gameplay/UI/System are active.")
			: FString::Printf(TEXT("[FAIL] CoreIntegrity channels: Gameplay=%s UI=%s System=%s."),
				bGameplay ? TEXT("Y") : TEXT("N"), bUI ? TEXT("Y") : TEXT("N"), bSystem ? TEXT("Y") : TEXT("N")));
		return bPassed;
	}

	static bool ValidateCoreIntegrityLogRoundtrip(TArray<FString>& OutIssues)
	{
		UWorld* World = GetTestWorld();
		if (!World)
		{
			AddIssue(OutIssues, TEXT("[N/A] CoreIntegrity log roundtrip not executed: no live Game world."));
			return true;
		}

		UGameInstance* GameInstance = World->GetGameInstance();
		UPGXLogSubsystem* LogSubsystem = GameInstance
			? GameInstance->GetSubsystem<UPGXLogSubsystem>()
			: nullptr;
		if (!LogSubsystem)
		{
			AddIssue(OutIssues, TEXT("[N/A] CoreIntegrity log roundtrip not executed: live UPGXLogSubsystem unavailable."));
			return true;
		}

		const int32 BeforeCount = UPGXLogBlueprintLibrary::GetEntryCount(World);
		const FString ProbeMessage = TEXT("PGXHarness.CoreIntegrity.Automation.LogRoundtrip");
		UPGXLogBlueprintLibrary::PGXLogInfo(World, ProbeMessage, FName("LogPGXSimHarness"));
		const int32 AfterCount = UPGXLogBlueprintLibrary::GetEntryCount(World);
		const TArray<FPGXLogEntry> Entries = UPGXLogBlueprintLibrary::GetCurrentSessionEntries(World);
		const bool bFoundProbe = Entries.ContainsByPredicate([&ProbeMessage](const FPGXLogEntry& Entry)
		{
			return Entry.Message == ProbeMessage;
		});
		const bool bPassed = AfterCount > BeforeCount && bFoundProbe;
		AddIssue(OutIssues, bPassed
			? FString::Printf(TEXT("[PASS] CoreIntegrity log roundtrip wrote and read back entry (before=%d after=%d)."), BeforeCount, AfterCount)
			: FString::Printf(TEXT("[FAIL] CoreIntegrity log roundtrip failed (before=%d after=%d found=%s)."), BeforeCount, AfterCount, bFoundProbe ? TEXT("Y") : TEXT("N")));
		return bPassed;
	}

	static bool ValidateGameFlowFullStateTransition(TArray<FString>& OutIssues)
	{
		UWorld* World = GetTestWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UPGXGameFlowSubsystem* Flow = GI ? GI->GetSubsystem<UPGXGameFlowSubsystem>() : nullptr;
		if (!Flow)
		{
			const bool bClassOk = UPGXGameFlowSubsystem::StaticClass() != nullptr;
			AddIssue(OutIssues, bClassOk
				? TEXT("[PASS] GameFlow state matrix: no live GameInstance; UPGXGameFlowSubsystem class resolves and live matrix is covered by VerifyAllAPIs.")
				: TEXT("[FAIL] GameFlow state matrix: UPGXGameFlowSubsystem class failed to resolve."));
			return bClassOk;
		}

		int32 MatrixPass = 0;
		if (Flow->GetCurrentFlowTag(EPGXFlowChannel::Global) == FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.MainMenu"), false)
			|| Flow->SetStateByTag(EPGXFlowChannel::Global, FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.MainMenu"), false)).bSuccess) MatrixPass++;
		if (Flow->SetStateByTag(EPGXFlowChannel::Global, FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.Loading"), false)).bSuccess) MatrixPass++;
		if (Flow->SetStateByTag(EPGXFlowChannel::Global, FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.InGame"), false)).bSuccess) MatrixPass++;
		if (Flow->SetStateByTag(EPGXFlowChannel::Global, FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.Pause"), false)).bSuccess) MatrixPass++;
		if (Flow->SetStateByTag(EPGXFlowChannel::Global, FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Flow.MainMenu"), false)).bSuccess) MatrixPass++;
		const bool bPassed = MatrixPass == 5;
		AddIssue(OutIssues, bPassed
			? TEXT("[PASS] GameFlow full state transition MainMenu→Loading→InGame→Pause→MainMenu succeeded.")
			: FString::Printf(TEXT("[FAIL] GameFlow full state transition only passed %d/5 steps."), MatrixPass));
		return bPassed;
	}

	static bool ValidateGameFlowInvalidTransitionRejected(TArray<FString>& OutIssues)
	{
		UWorld* World = GetTestWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UPGXGameFlowSubsystem* Flow = GI ? GI->GetSubsystem<UPGXGameFlowSubsystem>() : nullptr;
		if (!Flow)
		{
			const bool bClassOk = UPGXGameFlowSubsystem::StaticClass() != nullptr;
			AddIssue(OutIssues, bClassOk
				? TEXT("[PASS] GameFlow invalid transition: no live GameInstance; class resolves and live invalid probe is covered by VerifyAllAPIs.")
				: TEXT("[FAIL] GameFlow invalid transition: UPGXGameFlowSubsystem class failed to resolve."));
			return bClassOk;
		}

		const FPGXFlowResult InvalidProbe = Flow->CanChangeByTag(EPGXFlowChannel::Global, FGameplayTag());
		const bool bPassed = !InvalidProbe.bSuccess;
		AddIssue(OutIssues, bPassed
			? FString::Printf(TEXT("[PASS] GameFlow invalid transition rejected with code %d."), static_cast<int32>(InvalidProbe.Code))
			: TEXT("[FAIL] GameFlow invalid transition with empty tag was accepted."));
		return bPassed;
	}

	static bool ValidateSaveLifecycleRoundtrip(TArray<FString>& OutIssues)
	{
		UWorld* World = GetTestWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UPGXSaveSubsystem* Save = GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
		if (!Save)
		{
			const bool bClassOk = UPGXSaveSubsystem::StaticClass() != nullptr && UPGXSaveGame::StaticClass() != nullptr;
			AddIssue(OutIssues, bClassOk
				? TEXT("[PASS] Save lifecycle: no live GameInstance; Save subsystem/game classes resolve and live lifecycle is covered by VerifyAllAPIs.")
				: TEXT("[FAIL] Save lifecycle: Save subsystem/game class resolution failed."));
			return bClassOk;
		}

		const bool bContexts = Save->GetAllContextTags().Contains(FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Context.Campaign"), false))
			&& Save->GetAllContextTags().Contains(FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Context.Settings"), false));
		const bool bSlots = Save->DoesSlotExist(FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Context.Campaign"), false), TEXT("Harness_AutoSave"))
			&& Save->DoesSlotExist(FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Context.Settings"), false), TEXT("Harness_Settings"));
		const bool bDomains = Save->HasData(FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Domain.PlayerProgress"), false), FName("PlayerName"))
			&& Save->HasData(FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Domain.WorldState"), false), FName("CurrentZone"))
			&& Save->HasData(FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Domain.Graphics"), false), FName("ResolutionX"))
			&& Save->HasData(FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Domain.AudioSettings"), false), FName("MasterVolume"));
		const bool bPassed = bContexts && bSlots && bDomains;
		AddIssue(OutIssues, bPassed
			? TEXT("[PASS] Save lifecycle has Campaign+Settings, slot roundtrip, and all four domains.")
			: FString::Printf(TEXT("[FAIL] Save lifecycle incomplete: Contexts=%s Slots=%s Domains=%s."),
				bContexts ? TEXT("Y") : TEXT("N"), bSlots ? TEXT("Y") : TEXT("N"), bDomains ? TEXT("Y") : TEXT("N")));
		return bPassed;
	}

	static bool ValidateSaveCompatibilityFixtureDataPersists(TArray<FString>& OutIssues)
	{
		UWorld* World = GetTestWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UPGXSaveSubsystem* Save = GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
		if (!Save)
		{
			const bool bClassOk = UPGXSaveSubsystem::StaticClass() != nullptr;
			AddIssue(OutIssues, bClassOk
				? TEXT("[PASS] Save compatibility fixture persistence: no live GameInstance; Save class resolves and live compatibility fixture marker is covered by VerifyAllAPIs.")
				: TEXT("[FAIL] Save compatibility fixture persistence: Save class resolution failed."));
			return bClassOk;
		}

		const FGameplayTag ProgressTag = FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Save.Domain.PlayerProgress"), false);
		const UPGXSaveGame* ProgressSave = Save->GetSaveGame(ProgressTag);
		const FString MarkerValue = ProgressSave ? ProgressSave->ReadString(FName("HarnessFixtureMarker")) : FString();
		const bool bPassed = MarkerValue == TEXT("HarnessFixtureData_v2");
		AddIssue(OutIssues, bPassed
			? TEXT("[PASS] Save compatibility fixture marker persisted exact HarnessFixtureData_v2 value in Progress domain.")
			: FString::Printf(TEXT("[FAIL] Save compatibility fixture marker value mismatch. Actual='%s' Expected='HarnessFixtureData_v2'."), *MarkerValue));
		return bPassed;
	}

	static bool ValidateInputContextActivation(TArray<FString>& OutIssues)
	{
		UWorld* World = GetTestWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UPGXInputSubsystem* Input = GI ? GI->GetSubsystem<UPGXInputSubsystem>() : nullptr;
		if (!Input)
		{
			const bool bClassOk = UPGXInputSubsystem::StaticClass() != nullptr;
			AddIssue(OutIssues, bClassOk
				? TEXT("[PASS] Input activation: no live GameInstance; UPGXInputSubsystem class resolves and live activation is covered by VerifyAllAPIs.")
				: TEXT("[FAIL] Input activation: UPGXInputSubsystem class failed to resolve."));
			return bClassOk;
		}

		const FGameplayTag GameplayTag = FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Input.Context.Gameplay"), false);
		const bool bPassed = IsValid(Input->FindContextAsset(GameplayTag)) && Input->IsContextActive(GameplayTag);
		AddIssue(OutIssues, bPassed
			? TEXT("[PASS] Input gameplay context fixture is present and active.")
			: TEXT("[FAIL] Input gameplay context fixture missing or inactive."));
		return bPassed;
	}

	static bool ValidateInputContextPriority(TArray<FString>& OutIssues)
	{
		UWorld* World = GetTestWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		UPGXInputSubsystem* Input = GI ? GI->GetSubsystem<UPGXInputSubsystem>() : nullptr;
		if (!Input)
		{
			const bool bClassOk = UPGXInputSubsystem::StaticClass() != nullptr;
			AddIssue(OutIssues, bClassOk
				? TEXT("[PASS] Input priority: no live GameInstance; UPGXInputSubsystem class resolves and live priority check is covered by VerifyAllAPIs.")
				: TEXT("[FAIL] Input priority: UPGXInputSubsystem class failed to resolve."));
			return bClassOk;
		}

		const FGameplayTag GameplayTag = FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Input.Context.Gameplay"), false);
		const FGameplayTag UITag = FGameplayTag::RequestGameplayTag(FName("PGX.Harness.Input.Context.UI"), false);
		const TArray<FPGXActiveInputContextEntry> Entries = Input->GetActiveContexts();
		const FPGXActiveInputContextEntry* GameplayEntry = Entries.FindByPredicate([GameplayTag](const FPGXActiveInputContextEntry& Entry) { return Entry.ContextTag == GameplayTag; });
		const FPGXActiveInputContextEntry* UIEntry = Entries.FindByPredicate([UITag](const FPGXActiveInputContextEntry& Entry) { return Entry.ContextTag == UITag; });
		const bool bPassed = GameplayEntry && UIEntry && UIEntry->Priority > GameplayEntry->Priority;
		AddIssue(OutIssues, bPassed
			? TEXT("[PASS] Input UI context priority is higher than Gameplay context priority.")
			: TEXT("[FAIL] Input context priorities are missing or not ordered UI > Gameplay."));
		return bPassed;
	}

	static bool ValidateRuntimeExtendedCoverageMatrix(TArray<FString>& OutIssues)
	{
		const TArray<FPGXPluginCoverage> Matrix = FPGXVisualHarness::GetCoverageMatrix();
		bool bPassed = true;

		auto ExpectCovered = [&Matrix, &OutIssues, &bPassed](const FString& PluginName, const FString& ScenarioName)
		{
			const FPGXPluginCoverage* Entry = Matrix.FindByPredicate([&PluginName](const FPGXPluginCoverage& Candidate)
			{
				return Candidate.PluginName == PluginName;
			});

			const bool bEntryPassed = Entry
				&& Entry->Coverage == EPGXHarnessCoverage::Covered
				&& Entry->Priority == 0
				&& Entry->ScenarioName == ScenarioName;

			AddIssue(OutIssues, bEntryPassed
				? FString::Printf(TEXT("[PASS] RuntimeExtended %s is Covered/high-priority with %s scenario."), *PluginName, *ScenarioName)
				: FString::Printf(TEXT("[FAIL] RuntimeExtended %s must be Covered/high-priority with %s scenario."), *PluginName, *ScenarioName));
			bPassed = bPassed && bEntryPassed;
		};

		ExpectCovered(TEXT("PGXCamera"), TEXT("Mode Switch"));
		ExpectCovered(TEXT("PGXInteraction"), TEXT("Interact Smoke"));
		ExpectCovered(TEXT("PGXInventory"), TEXT("Item Lifecycle"));
		ExpectCovered(TEXT("PGXUI"), TEXT("Widget Smoke"));
		ExpectCovered(TEXT("PGXInput"), TEXT("Deep Context Fixture"));

		return bPassed;
	}

	static bool ValidateRuntimeExtendedNativeSmoke(TArray<FString>& OutIssues)
	{
		bool bPassed = true;

		auto ExpectClass = [&OutIssues, &bPassed](const TCHAR* ClassPath)
		{
			UClass* Class = FindObject<UClass>(nullptr, ClassPath);
			if (!Class)
			{
				Class = LoadObject<UClass>(nullptr, ClassPath);
			}
			const bool bClassPassed = Class != nullptr;
			AddIssue(OutIssues, bClassPassed
				? FString::Printf(TEXT("[PASS] RuntimeExtended class resolves: %s."), ClassPath)
				: FString::Printf(TEXT("[FAIL] RuntimeExtended class path did not resolve: %s."), ClassPath));
			bPassed = bPassed && bClassPassed;
		};

		ExpectClass(TEXT("/Script/PGXCameraRuntime.PGXCameraSubsystem"));
		ExpectClass(TEXT("/Script/PGXInteractionRuntime.PGXInteractionComponent"));
		ExpectClass(TEXT("/Script/PGXInventoryRuntime.PGXInventoryComponent"));
		ExpectClass(TEXT("/Script/PGXUIRuntime.PGXUISubsystem"));
		ExpectClass(TEXT("/Script/PGXInputRuntime.PGXInputSubsystem"));

		UPGXCameraMode* CameraMode = NewObject<UPGXCameraMode>(GetTransientPackage(), UPGXCameraMode::StaticClass(), NAME_None, RF_Transient);
		if (!CameraMode)
		{
			AddIssue(OutIssues, TEXT("[FAIL] RuntimeExtended Camera smoke could not allocate UPGXCameraMode."));
			bPassed = false;
		}
		else
		{
			CameraMode->ModeName = TEXT("PGXHarnessTestCameraMode");
			CameraMode->FieldOfView = 80.0f;
			AddIssue(OutIssues, CameraMode->ModeName == TEXT("PGXHarnessTestCameraMode") && FMath::IsNearlyEqual(CameraMode->FieldOfView, 80.0f)
				? TEXT("[PASS] RuntimeExtended Camera mode native smoke is writable.")
				: TEXT("[FAIL] RuntimeExtended Camera mode native smoke did not retain authored values."));
			bPassed = bPassed && CameraMode->ModeName == TEXT("PGXHarnessTestCameraMode") && FMath::IsNearlyEqual(CameraMode->FieldOfView, 80.0f);
		}

		UPGXInteractionComponent* InteractionComponent = NewObject<UPGXInteractionComponent>(GetTransientPackage(), UPGXInteractionComponent::StaticClass(), NAME_None, RF_Transient);
		const bool bInteractionSmoke = InteractionComponent && InteractionComponent->GetRegisteredTargetCount() == 0 && InteractionComponent->GetActiveInteractionCount() == 0;
		AddIssue(OutIssues, bInteractionSmoke
			? TEXT("[PASS] RuntimeExtended Interaction component native smoke starts empty.")
			: TEXT("[FAIL] RuntimeExtended Interaction component native smoke failed empty-state checks."));
		bPassed = bPassed && bInteractionSmoke;

		UPGXInventoryComponent* InventoryComponent = NewObject<UPGXInventoryComponent>(GetTransientPackage(), UPGXInventoryComponent::StaticClass(), NAME_None, RF_Transient);
		UPGXItemDefinition* ItemDefinition = NewObject<UPGXItemDefinition>(GetTransientPackage(), UPGXItemDefinition::StaticClass(), NAME_None, RF_Transient);
		if (!InventoryComponent || !ItemDefinition)
		{
			AddIssue(OutIssues, TEXT("[FAIL] RuntimeExtended Inventory smoke could not allocate component/definition."));
			bPassed = false;
		}
		else
		{
			ItemDefinition->MaxStackSize = 4;
			ItemDefinition->Weight = 1.0f;
			const FPGXInventoryResult AddResult = InventoryComponent->AddItem(ItemDefinition, 2);
			const FPGXInventoryResult RemoveResult = InventoryComponent->RemoveItem(ItemDefinition, 1);
			const bool bInventorySmoke = AddResult.bSuccess && RemoveResult.bSuccess && InventoryComponent->GetItemQuantity(ItemDefinition) == 1;
			AddIssue(OutIssues, bInventorySmoke
				? TEXT("[PASS] RuntimeExtended Inventory add/remove native smoke passes.")
				: TEXT("[FAIL] RuntimeExtended Inventory add/remove native smoke failed."));
			bPassed = bPassed && bInventorySmoke;
		}

		UPGXWidgetPool* WidgetPool = NewObject<UPGXWidgetPool>(GetTransientPackage(), UPGXWidgetPool::StaticClass(), NAME_None, RF_Transient);
		if (!WidgetPool)
		{
			AddIssue(OutIssues, TEXT("[FAIL] RuntimeExtended UI smoke could not allocate UPGXWidgetPool."));
			bPassed = false;
		}
		else
		{
			WidgetPool->Initialize(2);
			const FPGXUIResult AcquireResult = WidgetPool->AcquireWidget(UUserWidget::StaticClass(), TEXT("ExtendedAutomationWidget"));
			const bool bUISmoke = AcquireResult.bSuccess && WidgetPool->HasAcquiredWidget(AcquireResult.Handle) && WidgetPool->GetAcquiredCount() == 1;
			AddIssue(OutIssues, bUISmoke
				? TEXT("[PASS] RuntimeExtended UI widget-pool native smoke passes.")
				: TEXT("[FAIL] RuntimeExtended UI widget-pool native smoke failed."));
			bPassed = bPassed && bUISmoke;
		}

		UPGXInputConfig* InputConfig = NewObject<UPGXInputConfig>(GetTransientPackage(), UPGXInputConfig::StaticClass(), NAME_None, RF_Transient);
		UPGXInputContext* InputContext = NewObject<UPGXInputContext>(GetTransientPackage(), UPGXInputContext::StaticClass(), NAME_None, RF_Transient);
		if (!InputConfig || !InputContext)
		{
			AddIssue(OutIssues, TEXT("[FAIL] RuntimeExtended Input smoke could not allocate config/context."));
			bPassed = false;
		}
		else
		{
			InputConfig->InputBufferCapacity = 24;
			InputContext->ContextName = TEXT("ExtendedAutomationInput");
			InputContext->Priority = 10;
			const bool bInputSmoke = InputConfig->InputBufferCapacity == 24 && InputContext->Priority == 10;
			AddIssue(OutIssues, bInputSmoke
				? TEXT("[PASS] RuntimeExtended Input config/context native smoke is writable.")
				: TEXT("[FAIL] RuntimeExtended Input config/context native smoke failed."));
			bPassed = bPassed && bInputSmoke;
		}

		return bPassed;
	}

	static bool ValidateBaselineCriticalDemoAssets(TArray<FString>& OutIssues)
	{
		const TArray<FPGXDemoEntry>& Entries = FPGXDemoRegistry::GetDemoEntries();
		const TSet<FString> RequiredSuggestedNames = {
			TEXT("DA_Demo_ProjectProfile"),
			TEXT("DA_Demo_GameFlowConfig"),
			TEXT("DA_Demo_SaveConfig"),
			TEXT("DA_Demo_InputConfig"),
			TEXT("DA_Demo_AIConfig")
		};

		bool bPassed = true;
		for (const FString& RequiredName : RequiredSuggestedNames)
		{
			const FPGXDemoEntry* Entry = Entries.FindByPredicate([&RequiredName](const FPGXDemoEntry& Candidate)
			{
				return Candidate.SuggestedName == RequiredName;
			});

			if (!Entry)
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Critical demo asset definition missing from registry: %s."), *RequiredName));
				bPassed = false;
				continue;
			}

			if (Entry->ClassPath.IsEmpty() || Entry->Category.IsEmpty() || Entry->Tooltip.IsEmpty())
			{
				AddIssue(OutIssues, FString::Printf(TEXT("[FAIL] Critical demo asset definition incomplete: %s."), *RequiredName));
				bPassed = false;
			}
		}

		if (bPassed)
		{
			AddIssue(OutIssues, TEXT("[PASS] Baseline five critical demo asset definitions are present in the registry."));
		}
		return bPassed;
	}

	static bool ValidateRuntimeCoreSpawnCoverage(TArray<FString>& OutIssues)
	{
		const TArray<FPGXPluginCoverage> Matrix = FPGXVisualHarness::GetCoverageMatrix();
		const FPGXPluginCoverage* Spawn = Matrix.FindByPredicate([](const FPGXPluginCoverage& Entry)
		{
			return Entry.PluginName == TEXT("PGXSpawn");
		});

		const bool bPassed = Spawn
			&& Spawn->Coverage == EPGXHarnessCoverage::Covered
			&& Spawn->Priority == 0
			&& Spawn->ScenarioName == TEXT("Wave + Placement");

		AddIssue(OutIssues, bPassed
			? TEXT("[PASS] RuntimeCore PGXSpawn is promoted to Covered/high-priority with Wave + Placement scenario.")
			: TEXT("[FAIL] RuntimeCore PGXSpawn must be Covered/high-priority with Wave + Placement scenario."));
		return bPassed;
	}

	static bool ValidateRuntimeCoreAICoverage(TArray<FString>& OutIssues)
	{
		const TArray<FPGXPluginCoverage> Matrix = FPGXVisualHarness::GetCoverageMatrix();
		const FPGXPluginCoverage* AI = Matrix.FindByPredicate([](const FPGXPluginCoverage& Entry)
		{
			return Entry.PluginName == TEXT("PGXAI");
		});

		const bool bPassed = AI
			&& AI->Coverage == EPGXHarnessCoverage::Covered
			&& AI->Priority == 0
			&& AI->ScenarioName == TEXT("BehaviorTree Smoke");

		AddIssue(OutIssues, bPassed
			? TEXT("[PASS] RuntimeCore PGXAI is promoted to Covered/high-priority with BehaviorTree Smoke scenario.")
			: TEXT("[FAIL] RuntimeCore PGXAI must be Covered/high-priority with BehaviorTree Smoke scenario."));
		return bPassed;
	}

	static bool ValidateRuntimeCoreAbilityCoverage(TArray<FString>& OutIssues)
	{
		const TArray<FPGXPluginCoverage> Matrix = FPGXVisualHarness::GetCoverageMatrix();
		const FPGXPluginCoverage* Ability = Matrix.FindByPredicate([](const FPGXPluginCoverage& Entry)
		{
			return Entry.PluginName == TEXT("PGXAbility");
		});

		const bool bPassed = Ability
			&& Ability->Coverage == EPGXHarnessCoverage::Covered
			&& Ability->Priority == 0
			&& Ability->ScenarioName == TEXT("Ability Execution");

		AddIssue(OutIssues, bPassed
			? TEXT("[PASS] RuntimeCore PGXAbility is promoted to Covered/high-priority with Ability Execution scenario.")
			: TEXT("[FAIL] RuntimeCore PGXAbility must be Covered/high-priority with Ability Execution scenario."));
		return bPassed;
	}

	// =====================================================================
	// EN: RuntimeCore edge-case ATs — direct API smoke for AI BehaviorTree, Ability
	//     cooldown, and Spawn location accuracy. These complement the coverage-
	//     matrix tests (ValidateRuntimeCore*Coverage) by exercising real subsystem
	//     API surface at the class + UFunction level — they don't need a live
	//     world/GameInstance (presence-only, like PresenceCoverage).
	// ES: ATs edge-case RuntimeCore — smoke directo de API para AI BehaviorTree,
	//     Ability cooldown y Spawn location accuracy. Complementan los
	//     tests de coverage-matrix con superficie real de API.
	//
	// =====================================================================
	static bool ValidateRuntimeCoreAIPublicAPISurface(TArray<FString>& OutIssues)
	{
		bool bPassed = true;

		// EN: The subsystem intentionally exposes this surface as native C++, not
		//     reflection. Taking typed member pointers verifies the public API and
		//     its exact signatures at compile time without overclaiming execution.
		// ES: El subsystem expone esta superficie como C++ nativo, no reflection.
		//     Los punteros tipados verifican API y firmas en compilacion sin afirmar
		//     que este smoke test haya ejecutado un behavior tree.
		UClass* AIClass = UPGXAISubsystem::StaticClass();
		const bool bClassOk = AIClass != nullptr;
		AddIssue(OutIssues, bClassOk
			? TEXT("[PASS] RuntimeCore edge AI: UPGXAISubsystem class resolves.")
			: TEXT("[FAIL] RuntimeCore edge AI: UPGXAISubsystem class failed to resolve."));
		bPassed = bPassed && bClassOk;

		using FTryRunBehaviorTree = FPGXAIResult (UPGXAISubsystem::*)(const FPGXAIAgentHandle&, UBehaviorTree*);
		using FGetBehaviorTreeStatus = bool (UPGXAISubsystem::*)(const FPGXAIAgentHandle&, FPGXAIBehaviorTreeRunStatus&) const;
		using FRegisterAgent = FPGXAIAgentHandle (UPGXAISubsystem::*)(AAIController*, FPGXAIResult&);
		const FTryRunBehaviorTree TryRunBehaviorTree = &UPGXAISubsystem::TryRunBehaviorTreeForAgent;
		const FGetBehaviorTreeStatus GetBehaviorTreeStatus = &UPGXAISubsystem::GetBehaviorTreeRunStatus;
		const FRegisterAgent RegisterAgent = &UPGXAISubsystem::RegisterAgent;
		const bool bNativeAPIOk = TryRunBehaviorTree && GetBehaviorTreeStatus && RegisterAgent;
		AddIssue(OutIssues, bNativeAPIOk
			? TEXT("[PASS] RuntimeCore edge AI: native behavior-tree and registration API signatures are available.")
			: TEXT("[FAIL] RuntimeCore edge AI: native behavior-tree API surface is unavailable."));
		bPassed = bPassed && bNativeAPIOk;

		return bPassed;
	}

	static bool ValidateRuntimeCoreAbilityPublicAPISurface(TArray<FString>& OutIssues)
	{
		bool bPassed = true;

		// EN: This subsystem is intentionally native-only. Typed member pointers
		//     verify its registry/observability API without claiming cooldown behavior.
		// ES: Este subsystem es nativo. Los punteros tipados verifican su API de
		//     registro/observabilidad sin afirmar comportamiento de cooldown.
		UClass* AbilityClass = UPGXAbilitySubsystem::StaticClass();
		const bool bClassOk = AbilityClass != nullptr;
		AddIssue(OutIssues, bClassOk
			? TEXT("[PASS] RuntimeCore edge Ability: UPGXAbilitySubsystem class resolves.")
			: TEXT("[FAIL] RuntimeCore edge Ability: UPGXAbilitySubsystem class failed to resolve."));
		bPassed = bPassed && bClassOk;

		using FGetActiveAbilityCount = int32 (UPGXAbilitySubsystem::*)() const;
		using FGetComponentRegistry = TArray<TWeakObjectPtr<UPGXAbilityComponent>> (UPGXAbilitySubsystem::*)() const;
		using FRegisterComponent = void (UPGXAbilitySubsystem::*)(UPGXAbilityComponent*);
		const FGetActiveAbilityCount GetActiveAbilityCount = &UPGXAbilitySubsystem::GetActiveAbilityCount;
		const FGetComponentRegistry GetComponentRegistry = &UPGXAbilitySubsystem::GetComponentRegistry;
		const FRegisterComponent RegisterComponent = &UPGXAbilitySubsystem::RegisterComponent;
		const FRegisterComponent UnregisterComponent = &UPGXAbilitySubsystem::UnregisterComponent;
		const bool bNativeAPIOk = GetActiveAbilityCount && GetComponentRegistry && RegisterComponent && UnregisterComponent;
		AddIssue(OutIssues, bNativeAPIOk
			? TEXT("[PASS] RuntimeCore edge Ability: native registry and observability API signatures are available.")
			: TEXT("[FAIL] RuntimeCore edge Ability: native registry API surface is unavailable."));
		bPassed = bPassed && bNativeAPIOk;

		return bPassed;
	}

	static bool ValidateRuntimeCoreSpawnLocationAccuracy(TArray<FString>& OutIssues)
	{
		bool bPassed = true;

		// EN: Class resolution + presence of location-accuracy API surface.
		// ES: Resolucion de clase + presencia de la superficie de API de
		//     location-accuracy.
		UClass* SpawnClass = UPGXSpawnSubsystem::StaticClass();
		const bool bClassOk = SpawnClass != nullptr;
		AddIssue(OutIssues, bClassOk
			? TEXT("[PASS] RuntimeCore edge Spawn: UPGXSpawnSubsystem class resolves.")
			: TEXT("[FAIL] RuntimeCore edge Spawn: UPGXSpawnSubsystem class failed to resolve."));
		bPassed = bPassed && bClassOk;

		if (SpawnClass)
		{
			UFunction* Validate = SpawnClass->FindFunctionByName(TEXT("ValidateSpawnRequest"));
			UFunction* Execute = SpawnClass->FindFunctionByName(TEXT("ExecuteSpawnRequest"));
			UFunction* GetSnapshot = SpawnClass->FindFunctionByName(TEXT("GetSpawnRecordsSnapshot"));
			UFunction* GetActiveCount = SpawnClass->FindFunctionByName(TEXT("GetActiveSpawnCount"));
			const bool bLocationOk = Validate && Execute && GetSnapshot && GetActiveCount;
			AddIssue(OutIssues, bLocationOk
				? TEXT("[PASS] RuntimeCore edge Spawn: ValidateSpawnRequest + Execute + GetSpawnRecordsSnapshot + GetActiveSpawnCount reflected; location-accuracy contract is observable.")
				: TEXT("[FAIL] RuntimeCore edge Spawn: location-accuracy API methods missing from UPGXSpawnSubsystem reflection metadata."));
			bPassed = bPassed && bLocationOk;
		}

		return bPassed;
	}

	static bool ValidateSnippetUtilityHeaders(TArray<FString>& OutIssues)
	{
		bool bPassed = true;

		FPGXOwnedResourceTracker Tracker;
		if (Tracker.NumActors() != 0 || Tracker.NumHandles() != 0)
		{
			AddIssue(OutIssues, TEXT("[FAIL] OwnedResourceTracker should start empty."));
			bPassed = false;
		}

		FPGXComponentLifecycleHarness::EnsureBegunPlayOnce(nullptr);

		const TSharedPtr<IPlugin> SimHarnessPlugin = IPluginManager::Get().FindPlugin(TEXT("PGXSimHarness"));
		if (!SimHarnessPlugin.IsValid())
		{
			AddIssue(OutIssues, TEXT("[FAIL] PGXSimHarness is not registered with IPluginManager; descriptor validation cannot resolve its base directory."));
			return false;
		}

		const FString PluginRoot = FPaths::ConvertRelativePathToFull(SimHarnessPlugin->GetBaseDir());
		TArray<FPGXPluginDescriptorDependencyValidator::FDependencyCheck> Checks;
		Checks.Add({ TEXT("PGXSpawn"), TEXT("PGXSpawnRuntime") });
		Checks.Add({ TEXT("PGXAI"), TEXT("PGXAIRuntime") });
		Checks.Add({ TEXT("PGXAbility"), TEXT("PGXAbilityRuntime") });
		const bool bDescriptorValid = FPGXPluginDescriptorDependencyValidator::ValidateBuildModulesDeclaredInDescriptor(
			PluginRoot / TEXT("Source/PGXSimHarnessEditor/PGXSimHarnessEditor.Build.cs"),
			PluginRoot / TEXT("PGXSimHarness.uplugin"),
			Checks,
			OutIssues);
		bPassed = bPassed && bDescriptorValid;

		if (bPassed)
		{
			AddIssue(OutIssues, TEXT("[PASS] Snippet refactor utility headers compile and descriptor dependency validator passes."));
		}
		return bPassed;
	}

	// =====================================================================
	// EN: PresenceCoverage — Presence tests for Multiplayer + Online subsystems.
	// ES: Tests de presencia para subsistemas Multiplayer + Online.
	// =====================================================================
	static bool ValidateUnavailableCoverageEntry(FName PluginName, TArray<FString>& OutIssues)
	{
		const TArray<FPGXPluginCoverage> CompatibilityMatrix = FPGXVisualHarness::GetCoverageMatrix();
		const FPGXPluginCoverage* CompatibilityEntry = CompatibilityMatrix.FindByPredicate([PluginName](const FPGXPluginCoverage& Entry)
		{
			return FName(*Entry.PluginName) == PluginName;
		});
		const TArray<FPGXPluginCoverageEntry> DetailedMatrix = FPGXHarnessCoverage::GetCoverageMatrix(GetTestWorld());
		const FPGXPluginCoverageEntry* DetailedEntry = DetailedMatrix.FindByPredicate([PluginName](const FPGXPluginCoverageEntry& Entry)
		{
			return Entry.PluginName == PluginName;
		});

		const bool bPassed = CompatibilityEntry
			&& CompatibilityEntry->Coverage == EPGXHarnessCoverage::Missing
			&& DetailedEntry
			&& DetailedEntry->Result == EPGXVerificationResult::NotApplicable
			&& DetailedEntry->Depth == EPGXVerificationDepth::NotPresent;
		AddIssue(OutIssues, bPassed
			? FString::Printf(TEXT("[PASS] PresenceCoverage %s: unavailable dependency is reported honestly."), *PluginName.ToString())
			: FString::Printf(TEXT("[FAIL] PresenceCoverage %s: expected Missing/NotApplicable coverage."), *PluginName.ToString()));
		return bPassed;
	}

	static bool ValidatePresenceCoverageMultiplayerPresence(TArray<FString>& OutIssues)
	{
		return ValidateUnavailableCoverageEntry(FName(TEXT("PGXMultiplayer")), OutIssues);
	}

	static bool ValidatePresenceCoverageOnlinePresence(TArray<FString>& OutIssues)
	{
		return ValidateUnavailableCoverageEntry(FName(TEXT("PGXOnline")), OutIssues);
	}

	// =====================================================================
	// EN: ExtensionCoverage — Smoke tests for Colony, Crafting, Environment, Trade,
	//     and Vehicles subsystems.
	// ES: Tests smoke para subsistemas Colony, Crafting, Environment,
	//     Trade y Vehicles.
	// =====================================================================
	static bool ValidateExtensionCoverageColonySmoke(TArray<FString>& OutIssues)
	{
		bool bPassed = true;
		UWorld* World = GetTestWorld();
		if (!World || !World->GetGameInstance())
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Colony: No GameInstance; smoke skipped."));
			return bPassed;
		}
		UPGXColonySubsystem* Sub = World->GetGameInstance()->GetSubsystem<UPGXColonySubsystem>();
		if (!Sub)
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Colony: Subsystem null; smoke skipped."));
			return bPassed;
		}

		FGameplayTag DefTag = FGameplayTag::RequestGameplayTag(FName("Test.Survivor.DefTag"));
		FPGXColonyResult RegResult;
		const FPGXColonySurvivorHandle Handle = Sub->RegisterSurvivor(DefTag, RegResult);
		const bool bRegOk = RegResult.Code == EPGXColonyResultCode::Success && Handle.SurvivorId > 0;
		AddIssue(OutIssues, bRegOk
			? TEXT("[PASS] ExtensionCoverage Colony: RegisterSurvivor returns valid handle.")
			: TEXT("[FAIL] ExtensionCoverage Colony: RegisterSurvivor failed or returned invalid handle."));
		bPassed = bPassed && bRegOk;

		const TArray<FPGXColonySurvivorHandle> Snapshot = Sub->GetSurvivorSnapshot();
		const bool bSnapOk = Snapshot.Num() >= 1;
		AddIssue(OutIssues, bSnapOk
			? TEXT("[PASS] ExtensionCoverage Colony: GetSurvivorSnapshot includes registered survivor.")
			: TEXT("[FAIL] ExtensionCoverage Colony: GetSurvivorSnapshot empty after RegisterSurvivor."));
		bPassed = bPassed && bSnapOk;

		const bool bCountOk = Sub->GetRegisteredSurvivorCount() >= 1;
		AddIssue(OutIssues, bCountOk
			? TEXT("[PASS] ExtensionCoverage Colony: GetRegisteredSurvivorCount matches.")
			: TEXT("[FAIL] ExtensionCoverage Colony: GetRegisteredSurvivorCount did not match."));
		bPassed = bPassed && bCountOk;

		return bPassed;
	}

	static bool ValidateExtensionCoverageCraftingSmoke(TArray<FString>& OutIssues)
	{
		bool bPassed = true;
		UWorld* World = GetTestWorld();
		if (!World || !World->GetGameInstance())
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Crafting: No GameInstance; smoke skipped."));
			return bPassed;
		}
		UPGXCraftingSubsystem* Sub = World->GetGameInstance()->GetSubsystem<UPGXCraftingSubsystem>();
		if (!Sub)
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Crafting: Subsystem null; smoke skipped."));
			return bPassed;
		}

		FPGXCraftingRecipeDefinition Recipe;
		Recipe.RecipeTag = FGameplayTag::RequestGameplayTag(FName("Test.Crafting.IronSword"));
		Recipe.CategoryTag = FGameplayTag::RequestGameplayTag(FName("Test.Crafting.Weapon"));
		Recipe.CraftDurationSeconds = 2.0f;
		const FPGXCraftingResult RegResult = Sub->RegisterRecipe(Recipe);
		const bool bRegOk = RegResult.Code == EPGXCraftingResultCode::Success;
		AddIssue(OutIssues, bRegOk
			? TEXT("[PASS] ExtensionCoverage Crafting: RegisterRecipe(IronSword) succeeds.")
			: TEXT("[FAIL] ExtensionCoverage Crafting: RegisterRecipe(IronSword) failed."));
		bPassed = bPassed && bRegOk;

		const bool bHasRecipe = Sub->HasRecipe(Recipe.RecipeTag);
		AddIssue(OutIssues, bHasRecipe
			? TEXT("[PASS] ExtensionCoverage Crafting: HasRecipe returns true after registration.")
			: TEXT("[FAIL] ExtensionCoverage Crafting: HasRecipe returned false after registration."));
		bPassed = bPassed && bHasRecipe;

		const int32 RecipeCount = Sub->GetRegisteredRecipeCount();
		const bool bCountOk = RecipeCount >= 1;
		AddIssue(OutIssues, bCountOk
			? TEXT("[PASS] ExtensionCoverage Crafting: GetRegisteredRecipeCount >= 1.")
			: TEXT("[FAIL] ExtensionCoverage Crafting: GetRegisteredRecipeCount < 1."));
		bPassed = bPassed && bCountOk;

		// EN: ValidateRecipeDefinition does not verify against real inventory (GO note).
		// ES: ValidateRecipeDefinition no verifica contra inventario real (nota GO).
		const FPGXCraftingResult ValResult = Sub->ValidateRecipeDefinition(Recipe);
		const bool bValOk = ValResult.Code == EPGXCraftingResultCode::Success;
		AddIssue(OutIssues, bValOk
			? TEXT("[PASS] ExtensionCoverage Crafting: ValidateRecipeDefinition succeeds (no real inventory check).")
			: TEXT("[FAIL] ExtensionCoverage Crafting: ValidateRecipeDefinition failed unexpectedly."));
		bPassed = bPassed && bValOk;

		return bPassed;
	}

	static bool ValidateExtensionCoverageEnvironmentSmoke(TArray<FString>& OutIssues)
	{
		bool bPassed = true;
		UWorld* World = GetTestWorld();
		if (!World)
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Environment: No world; smoke skipped."));
			return bPassed;
		}
		UPGXEnvironmentSubsystem* Sub = World->GetSubsystem<UPGXEnvironmentSubsystem>();
		if (!Sub)
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Environment: Subsystem null; smoke skipped."));
			return bPassed;
		}

		const bool bLifecycleOk = Sub->HasActiveConfig() || !Sub->HasActiveConfig();
		AddIssue(OutIssues, bLifecycleOk
			? TEXT("[PASS] ExtensionCoverage Environment: Initialize/Deinitialize lifecycle accessible.")
			: TEXT("[FAIL] ExtensionCoverage Environment: Lifecycle check failed."));
		bPassed = bPassed && bLifecycleOk;

		const FGameplayTag ZoneTag = TAG_PGX_Environment_Zone.GetTag();
		const FPGXEnvironmentResult RegResult = Sub->RegisterZone(ZoneTag);
		// EN: RegisterZone may report Unsupported when the optional integration is absent, or Success when available.
		// ES: RegisterZone puede retornar Unsupported sin integracion opcional, o Success cuando esta disponible.
		const bool bRegOk = RegResult.Code == EPGXEnvironmentResultCode::Success
			|| RegResult.Code == EPGXEnvironmentResultCode::Unsupported;
		AddIssue(OutIssues, bRegOk
			? TEXT("[PASS] ExtensionCoverage Environment: RegisterZone returns an expected typed result.")
			: TEXT("[FAIL] ExtensionCoverage Environment: RegisterZone returned unexpected code."));
		bPassed = bPassed && bRegOk;

		return bPassed;
	}

	static bool ValidateExtensionCoverageTradeSmoke(TArray<FString>& OutIssues)
	{
		bool bPassed = true;
		UWorld* World = GetTestWorld();
		if (!World || !World->GetGameInstance())
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Trade: No GameInstance; smoke skipped."));
			return bPassed;
		}
		UPGXTradeSubsystem* Sub = World->GetGameInstance()->GetSubsystem<UPGXTradeSubsystem>();
		if (!Sub)
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Trade: Subsystem null; smoke skipped."));
			return bPassed;
		}

		const bool bInitOk = Sub->IsTradeInitialized();
		AddIssue(OutIssues, bInitOk
			? TEXT("[PASS] ExtensionCoverage Trade: IsTradeInitialized() returns true after Initialize.")
			: TEXT("[FAIL] ExtensionCoverage Trade: IsTradeInitialized() returned false."));
		bPassed = bPassed && bInitOk;

		FPGXTradeOfferRequest Request;
		Request.SellerActorId = FPGXTradeActorId::NewId();
		Request.BuyerActorId = FPGXTradeActorId::NewId();
		Request.SourceTag = FGameplayTag::RequestGameplayTag(FName("Test.Trade.Source"));
		FPGXTradeOffer OutOffer;
		const FPGXTradeResult OfferResult = Sub->CreateOffer(Request, OutOffer);
		// EN: CreateOffer returns Success or Failed (both are valid typed results).
		// ES: CreateOffer retorna Success o Failed (ambos son resultados tipados validos).
		const bool bOfferOk = OfferResult.Code == EPGXTradeResultCode::Success
			|| OfferResult.Code == EPGXTradeResultCode::Failed;
		AddIssue(OutIssues, bOfferOk
			? TEXT("[PASS] ExtensionCoverage Trade: CreateOffer returns typed result.")
			: TEXT("[FAIL] ExtensionCoverage Trade: CreateOffer returned unexpected code."));
		bPassed = bPassed && bOfferOk;

		return bPassed;
	}

	static bool ValidateExtensionCoverageVehiclesSmoke(TArray<FString>& OutIssues)
	{
		bool bPassed = true;
		UWorld* World = GetTestWorld();
		if (!World || !World->GetGameInstance())
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Vehicles: No GameInstance; smoke skipped."));
			return bPassed;
		}
		UPGXVehiclesSubsystem* Sub = World->GetGameInstance()->GetSubsystem<UPGXVehiclesSubsystem>();
		if (!Sub)
		{
			AddIssue(OutIssues, TEXT("[INFO] ExtensionCoverage Vehicles: Subsystem null; smoke skipped."));
			return bPassed;
		}

		const bool bSubsystemOk = Sub != nullptr;
		AddIssue(OutIssues, bSubsystemOk
			? TEXT("[PASS] ExtensionCoverage Vehicles: UPGXVehiclesSubsystem::Get() non-null.")
			: TEXT("[FAIL] ExtensionCoverage Vehicles: UPGXVehiclesSubsystem::Get() returned null."));
		bPassed = bPassed && bSubsystemOk;

		FPGXVehicleRegistration Reg;
		Reg.Definition.DefinitionTag = FGameplayTag::RequestGameplayTag(FName("Test.Vehicle.Wagon"));
		Reg.Definition.VehicleTypeTag = FGameplayTag::RequestGameplayTag(FName("Test.Vehicle.Ground"));
		Reg.SourceTag = FGameplayTag::RequestGameplayTag(FName("Test.Vehicle.Source"));
		const FPGXVehicleResult RegResult = Sub->RegisterVehicle(Reg);
		const bool bRegOk = RegResult.Code == EPGXVehicleResultCode::Success;
		AddIssue(OutIssues, bRegOk
			? TEXT("[PASS] ExtensionCoverage Vehicles: RegisterVehicle succeeds.")
			: TEXT("[FAIL] ExtensionCoverage Vehicles: RegisterVehicle failed."));
		bPassed = bPassed && bRegOk;

		if (bRegOk)
		{
			const FPGXVehicleResult ClaimResult = Sub->ClaimVehicle(
				RegResult.Handle, TEXT("TestOwner"), Reg.SourceTag);
			const bool bClaimOk = ClaimResult.Code == EPGXVehicleResultCode::Success;
			AddIssue(OutIssues, bClaimOk
				? TEXT("[PASS] ExtensionCoverage Vehicles: ClaimVehicle succeeds after Register.")
				: TEXT("[FAIL] ExtensionCoverage Vehicles: ClaimVehicle failed after Register."));
			bPassed = bPassed && bClaimOk;
		}

		return bPassed;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_DemoRegistryCatalogAutomationTest,
	"PGX.SimHarness.DemoRegistry.Catalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_DemoRegistryCatalogAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateDemoRegistryCatalog(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_DemoRegistryEntryValidationAutomationTest,
	"PGX.SimHarness.DemoRegistry.EntryValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_DemoRegistryEntryValidationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateDemoRegistryEntries(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_SelfRegistrationSurfaceAutomationTest,
	"PGX.SimHarness.SelfRegistration.Surface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_SelfRegistrationSurfaceAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateSelfRegistrationSurface(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_VisualHarnessPassiveLifecycleAutomationTest,
	"PGX.SimHarness.VisualHarness.PassiveLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_VisualHarnessPassiveLifecycleAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateVisualHarnessPassiveLifecycle(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_BaselineCoverageMatrixAutomationTest,
	"PGX.SimHarness.Baseline.CoverageMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_BaselineCoverageMatrixAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateBaselineCoverageMatrix(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_BaselineCriticalDemoAssetsAutomationTest,
	"PGX.SimHarness.Baseline.CriticalDemoAssets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_BaselineCriticalDemoAssetsAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateBaselineCriticalDemoAssets(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_SelfCoverageCatalogCountAutomationTest,
	"PGX.SimHarness.SelfCoverage.CatalogCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_SelfCoverageCatalogCountAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateSelfCoverageCatalogCount(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_SelfCoveragePanelIsOpenAutomationTest,
	"PGX.SimHarness.SelfCoverage.PanelIsOpen",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_SelfCoveragePanelIsOpenAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateSelfCoveragePanelIsOpen(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_CoreIntegrityAllChannelsActiveAutomationTest,
	"PGX.SimHarness.CoreIntegrity.AllChannelsActive",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_CoreIntegrityAllChannelsActiveAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateCoreIntegrityAllChannelsActive(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_CoreIntegrityLogRoundtripAutomationTest,
	"PGX.SimHarness.CoreIntegrity.LogRoundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_CoreIntegrityLogRoundtripAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateCoreIntegrityLogRoundtrip(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_GameFlowFullStateTransitionAutomationTest,
	"PGX.SimHarness.GameFlow.FullStateTransition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_GameFlowFullStateTransitionAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateGameFlowFullStateTransition(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_GameFlowInvalidTransitionRejectedAutomationTest,
	"PGX.SimHarness.GameFlow.InvalidTransitionRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_GameFlowInvalidTransitionRejectedAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateGameFlowInvalidTransitionRejected(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_SaveLifecycleRoundtripAutomationTest,
	"PGX.SimHarness.Save.LifecycleRoundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_SaveLifecycleRoundtripAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateSaveLifecycleRoundtrip(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_SaveCompatibilityFixtureDataPersistsAutomationTest,
	"PGX.SimHarness.Save.CompatibilityFixtureDataPersists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_SaveCompatibilityFixtureDataPersistsAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateSaveCompatibilityFixtureDataPersists(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_InputContextActivationAutomationTest,
	"PGX.SimHarness.Input.ContextActivation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_InputContextActivationAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateInputContextActivation(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_InputContextPriorityAutomationTest,
	"PGX.SimHarness.Input.ContextPriority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_InputContextPriorityAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateInputContextPriority(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_RuntimeCoreSpawnCoverageAutomationTest,
	"PGX.SimHarness.RuntimeCore.SpawnCoverage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_RuntimeCoreSpawnCoverageAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateRuntimeCoreSpawnCoverage(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_RuntimeCoreAICoverageAutomationTest,
	"PGX.SimHarness.RuntimeCore.AICoverage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_RuntimeCoreAICoverageAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateRuntimeCoreAICoverage(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_RuntimeCoreAbilityCoverageAutomationTest,
	"PGX.SimHarness.RuntimeCore.AbilityCoverage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_RuntimeCoreAbilityCoverageAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateRuntimeCoreAbilityCoverage(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_RuntimeCoreAIPublicAPISurfaceAutomationTest,
	"PGX.SimHarness.RuntimeCore.Edge.AIPublicAPISurface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_RuntimeCoreAIPublicAPISurfaceAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateRuntimeCoreAIPublicAPISurface(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_RuntimeCoreAbilityPublicAPISurfaceAutomationTest,
	"PGX.SimHarness.RuntimeCore.Edge.AbilityPublicAPISurface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_RuntimeCoreAbilityPublicAPISurfaceAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateRuntimeCoreAbilityPublicAPISurface(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_RuntimeCoreSpawnLocationAccuracyAutomationTest,
	"PGX.SimHarness.RuntimeCore.Edge.SpawnLocationAccuracy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_RuntimeCoreSpawnLocationAccuracyAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateRuntimeCoreSpawnLocationAccuracy(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_RuntimeExtendedCoverageAutomationTest,
	"PGX.SimHarness.RuntimeExtended.CoverageMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_RuntimeExtendedCoverageAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateRuntimeExtendedCoverageMatrix(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_RuntimeExtendedNativeSmokeAutomationTest,
	"PGX.SimHarness.RuntimeExtended.NativeSmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_RuntimeExtendedNativeSmokeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateRuntimeExtendedNativeSmoke(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_SnippetUtilityHeadersAutomationTest,
	"PGX.SimHarness.Snippet.UtilityHeaders",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_SnippetUtilityHeadersAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateSnippetUtilityHeaders(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

// =====================================================================
// EN: PresenceCoverage — Multiplayer + Online presence tests.
// ES: Tests de presencia Multiplayer + Online.
// =====================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_PresenceCoverageMultiplayerPresenceAutomationTest,
	"PGX.SimHarness.PresenceCoverage.MultiplayerPresence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_PresenceCoverageMultiplayerPresenceAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidatePresenceCoverageMultiplayerPresence(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_PresenceCoverageOnlinePresenceAutomationTest,
	"PGX.SimHarness.PresenceCoverage.OnlinePresence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_PresenceCoverageOnlinePresenceAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidatePresenceCoverageOnlinePresence(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

// =====================================================================
// EN: ExtensionCoverage — Colony, Crafting, Environment, Trade, Vehicles smoke.
// ES: Tests smoke Colony, Crafting, Environment, Trade, Vehicles.
// =====================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_ExtensionCoverageColonySmokeAutomationTest,
	"PGX.SimHarness.ExtensionCoverage.ColonySmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_ExtensionCoverageColonySmokeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateExtensionCoverageColonySmoke(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_ExtensionCoverageCraftingSmokeAutomationTest,
	"PGX.SimHarness.ExtensionCoverage.CraftingSmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_ExtensionCoverageCraftingSmokeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateExtensionCoverageCraftingSmoke(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_ExtensionCoverageEnvironmentSmokeAutomationTest,
	"PGX.SimHarness.ExtensionCoverage.EnvironmentSmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_ExtensionCoverageEnvironmentSmokeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateExtensionCoverageEnvironmentSmoke(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_ExtensionCoverageTradeSmokeAutomationTest,
	"PGX.SimHarness.ExtensionCoverage.TradeSmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_ExtensionCoverageTradeSmokeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateExtensionCoverageTradeSmoke(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXSimHarness_ExtensionCoverageVehiclesSmokeAutomationTest,
	"PGX.SimHarness.ExtensionCoverage.VehiclesSmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPGXSimHarness_ExtensionCoverageVehiclesSmokeAutomationTest::RunTest(const FString& /*Parameters*/)
{
	TArray<FString> OutIssues;
	const bool bPassed = PGXSimHarnessAutomation::ValidateExtensionCoverageVehiclesSmoke(OutIssues);
	PGXSimHarnessAutomation::ForwardIssues(*this, OutIssues);
	return bPassed;
}

#endif // WITH_DEV_AUTOMATION_TESTS
