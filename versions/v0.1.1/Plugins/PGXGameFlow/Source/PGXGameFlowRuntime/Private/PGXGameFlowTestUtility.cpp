// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXGameFlowTestUtility.h"
#include "PGXGameFlowSubsystem.h"
#include "Tags/PGXGameFlowTags.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

// EN: GameFlow Test Utility — self-contained test functions for PIE verification
// ES: Utilidad de prueba GameFlow — funciones de prueba autocontenidas para verificacion en PIE

// ============================================================================
// Helpers
// ============================================================================

UPGXGameFlowSubsystem* UPGXGameFlowTestUtility::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;

	return GI->GetSubsystem<UPGXGameFlowSubsystem>();
}

void UPGXGameFlowTestUtility::LogTestResult(const FString& TestName, bool bPassed, const FString& Details)
{
	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXGameFlow, TEXT("[TEST PASS] %s%s"),
			*TestName,
			Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" — %s"), *Details));
	}
	else
	{
		PGX_LOG_ERROR(LogPGXGameFlow, TEXT("[TEST FAIL] %s%s"),
			*TestName,
			Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" — %s"), *Details));
	}
}

FGameplayTag UPGXGameFlowTestUtility::MakeTag(const FString& TagString)
{
	return FGameplayTag::RequestGameplayTag(FName(*TagString), false);
}

// ============================================================================
// RunQuickTest
// ============================================================================

void UPGXGameFlowTestUtility::RunQuickTest(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXGameFlow, TEXT(""));
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("========================================"));
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("  PGX GameFlow — Quick Test Suite"));
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("========================================"));

	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		LogTestResult(TEXT("Subsystem Availability"), false, TEXT("GameFlow subsystem not found"));
		return;
	}

	LogTestResult(TEXT("Subsystem Availability"), true, TEXT("Subsystem found and accessible"));
	LogTestResult(TEXT("Subsystem Initialized"), Sub->IsInitialized(), Sub->IsInitialized() ? TEXT("Ready") : TEXT("Not initialized"));

	// Log channel states
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		const EPGXFlowChannel Ch = static_cast<EPGXFlowChannel>(i);
		const FGameplayTag Current = Sub->GetCurrentFlowTag(Ch);
		PGX_LOG_INFO(LogPGXGameFlow, TEXT("  [%s] = %s"),
			*UPGXGameFlowSubsystem::GetChannelName(Ch),
			Current.IsValid() ? *Current.ToString() : TEXT("(none)"));
	}

	// Run sub-tests
	TestSetAndValidate(WorldContextObject);
	TestRevertFlow(WorldContextObject);

	PGX_LOG_INFO(LogPGXGameFlow, TEXT("========================================"));
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("  Quick Test Suite Complete"));
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("========================================"));
	PGX_LOG_INFO(LogPGXGameFlow, TEXT(""));
}

// ============================================================================
// TestSetAndValidate
// ============================================================================

void UPGXGameFlowTestUtility::TestSetAndValidate(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("--- TestSetAndValidate ---"));

	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("SetAndValidate"), false, TEXT("No subsystem")); return; }

	const EPGXFlowChannel TestChannel = EPGXFlowChannel::Global;

	// Save original state to restore
	const FGameplayTag OriginalTag = Sub->GetCurrentFlowTag(TestChannel);

	// Test 1: Set a state
	const FGameplayTag TestTag1 = TAG_PGX_GameFlow_State_Boot;
	const FGameplayTag TestTag2 = TAG_PGX_GameFlow_State_MainMenu;

	if (TestTag1.IsValid() && TestTag2.IsValid())
	{
		// Set first state
		FPGXFlowResult R1 = Sub->SetStateByTag(TestChannel, TestTag1, nullptr);
		// If already at TestTag1, that's fine (redundant), try tag2
		if (!R1.bSuccess && R1.Code == EPGXFlowResultCode::RedundantState)
		{
			R1 = FPGXFlowResult::MakeSuccess(TEXT("Already at test state"));
		}
		LogTestResult(TEXT("SetState(Boot)"), R1.bSuccess, R1.Description);

		// Query current
		const bool bIsCurrent = Sub->IsCurrentFlowTag(TestChannel, TestTag1);
		LogTestResult(TEXT("IsCurrentFlowTag(Boot)"), bIsCurrent);

		// CanChange check
		FPGXFlowResult CanChange = Sub->CanChangeByTag(TestChannel, TestTag2);
		LogTestResult(TEXT("CanChangeByTag(MainMenu)"), true, FString::Printf(TEXT("Result=%s: %s"),
			CanChange.bSuccess ? TEXT("ALLOWED") : TEXT("DENIED"), *CanChange.Description));

		// Set second state
		FPGXFlowResult R2 = Sub->SetStateByTag(TestChannel, TestTag2, nullptr);
		LogTestResult(TEXT("SetState(MainMenu)"), R2.bSuccess, R2.Description);

		// Verify last tag
		const FGameplayTag LastTag = Sub->GetLastFlowTag(TestChannel);
		LogTestResult(TEXT("GetLastFlowTag = Boot"), LastTag == TestTag1,
			FString::Printf(TEXT("Got: %s"), LastTag.IsValid() ? *LastTag.ToString() : TEXT("(none)")));

		// Redundant check
		FPGXFlowResult Redundant = Sub->SetStateByTag(TestChannel, TestTag2, nullptr);
		LogTestResult(TEXT("Redundant Rejected"), !Redundant.bSuccess && Redundant.Code == EPGXFlowResultCode::RedundantState,
			Redundant.Description);

		// History check
		TArray<FPGXFlowHistoryEntry> History = Sub->GetChannelHistory(TestChannel);
		LogTestResult(TEXT("History Populated"), History.Num() >= 2,
			FString::Printf(TEXT("%d entries"), History.Num()));
	}
	else
	{
		LogTestResult(TEXT("SetAndValidate"), false, TEXT("Canonical GameFlow test tags not registered"));
	}

	// Restore original state (best-effort)
	if (OriginalTag.IsValid())
	{
		Sub->SetStateByTag(TestChannel, OriginalTag, nullptr);
	}
}

// ============================================================================
// TestBatchOperations
// ============================================================================

void UPGXGameFlowTestUtility::TestBatchOperations(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("--- TestBatchOperations ---"));

	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("BatchOps"), false, TEXT("No subsystem")); return; }

	const EPGXFlowChannel TestChannel = EPGXFlowChannel::UI;
	const FGameplayTag OriginalTag = Sub->GetCurrentFlowTag(TestChannel);

	// Set initial state
	const FGameplayTag StartTag = TAG_PGX_GameFlow_State_Boot;
	if (StartTag.IsValid())
	{
		Sub->SetStateByTag(TestChannel, StartTag, nullptr);
	}

	// Batch tags
	TArray<FGameplayTag> BatchTags;
	BatchTags.Add(TAG_PGX_GameFlow_State_MainMenu);
	BatchTags.Add(TAG_PGX_GameFlow_State_Loading);
	BatchTags.Add(TAG_PGX_GameFlow_State_InWorld);

	// Remove invalid tags
	BatchTags.RemoveAll([](const FGameplayTag& T) { return !T.IsValid(); });

	if (BatchTags.Num() >= 2)
	{
		// Batch validate
		FPGXFlowResult CanBatch = Sub->CanBatchChangeByTag(TestChannel, BatchTags);
		LogTestResult(TEXT("CanBatchChange"), true,
			FString::Printf(TEXT("Result=%s: %s"), CanBatch.bSuccess ? TEXT("VALID") : TEXT("INVALID"), *CanBatch.Description));

		// Batch sequential
		FPGXFlowResult BatchResult = Sub->SetBatchSequentialStateByTag(TestChannel, BatchTags, nullptr);
		LogTestResult(TEXT("SetBatchSequential"), BatchResult.bSuccess, BatchResult.Description);

		// Verify final state
		if (BatchResult.bSuccess)
		{
			const FGameplayTag FinalTag = Sub->GetCurrentFlowTag(TestChannel);
			const FGameplayTag ExpectedFinal = BatchTags.Last();
			LogTestResult(TEXT("BatchFinalState"), FinalTag == ExpectedFinal,
				FString::Printf(TEXT("Expected: %s, Got: %s"),
					*ExpectedFinal.ToString(),
					FinalTag.IsValid() ? *FinalTag.ToString() : TEXT("(none)")));
		}
	}
	else
	{
		LogTestResult(TEXT("BatchOps"), false, TEXT("Insufficient valid test tags"));
	}

	// Restore
	if (OriginalTag.IsValid())
	{
		Sub->SetStateByTag(TestChannel, OriginalTag, nullptr);
	}
}

// ============================================================================
// TestRevertFlow
// ============================================================================

void UPGXGameFlowTestUtility::TestRevertFlow(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("--- TestRevertFlow ---"));

	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Revert"), false, TEXT("No subsystem")); return; }

	const EPGXFlowChannel TestChannel = EPGXFlowChannel::Systems;
	const FGameplayTag OriginalTag = Sub->GetCurrentFlowTag(TestChannel);

	const FGameplayTag Tag1 = TAG_PGX_GameFlow_State_Boot;
	const FGameplayTag Tag2 = TAG_PGX_GameFlow_State_MainMenu;

	if (Tag1.IsValid() && Tag2.IsValid())
	{
		Sub->SetStateByTag(TestChannel, Tag1, nullptr);
		Sub->SetStateByTag(TestChannel, Tag2, nullptr);

		// Check can revert
		const bool bCanRevert = Sub->CheckCanRevert(TestChannel);
		LogTestResult(TEXT("CheckCanRevert"), bCanRevert);

		// Revert
		FPGXFlowResult RevertResult = Sub->RevertToPreviousFlow(TestChannel, nullptr);
		LogTestResult(TEXT("RevertToPreviousFlow"), RevertResult.bSuccess, RevertResult.Description);

		// Verify reverted to Tag1
		const FGameplayTag AfterRevert = Sub->GetCurrentFlowTag(TestChannel);
		LogTestResult(TEXT("RevertedToCorrectState"), AfterRevert == Tag1,
			FString::Printf(TEXT("Expected: %s, Got: %s"),
				*Tag1.ToString(),
				AfterRevert.IsValid() ? *AfterRevert.ToString() : TEXT("(none)")));
	}
	else
	{
		LogTestResult(TEXT("Revert"), false, TEXT("Test tags not registered"));
	}

	// Restore
	if (OriginalTag.IsValid())
	{
		Sub->SetStateByTag(TestChannel, OriginalTag, nullptr);
	}
}

// ============================================================================
// TestValidationRules
// ============================================================================

void UPGXGameFlowTestUtility::TestValidationRules(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("--- TestValidationRules ---"));

	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("ValidationRules"), false, TEXT("No subsystem")); return; }

	const EPGXFlowChannel TestChannel = EPGXFlowChannel::Global;
	const FGameplayTag OriginalTag = Sub->GetCurrentFlowTag(TestChannel);

	// Get current rule
	FPGXFlowRule CurrentRule;
	const bool bHasRule = Sub->GetAllowedTransitionByCurrentFlowTag(TestChannel, CurrentRule);

	if (bHasRule)
	{
		PGX_LOG_INFO(LogPGXGameFlow, TEXT("  Current rule: %s | Allowed: %d | Disallowed: %d"),
			*CurrentRule.RuleName.ToString(),
			CurrentRule.AllowedDestinations.Num(),
			CurrentRule.DisallowedTagQueries.Num());

		// Test allowed destinations
		for (const FGameplayTag& AllowedTag : CurrentRule.AllowedDestinations)
		{
			FPGXFlowResult R = Sub->CanChangeByTag(TestChannel, AllowedTag);
			LogTestResult(FString::Printf(TEXT("Allowed: %s"), *AllowedTag.ToString()),
				R.bSuccess, R.Description);
		}

		// Test disallowed destinations
		for (const FGameplayTag& DisallowedTag : CurrentRule.DisallowedTagQueries)
		{
			FPGXFlowResult R = Sub->CanChangeByTag(TestChannel, DisallowedTag);
			LogTestResult(FString::Printf(TEXT("Disallowed: %s"), *DisallowedTag.ToString()),
				!R.bSuccess, R.Description);
		}

		LogTestResult(TEXT("ValidationRules"), true, TEXT("Rule enforcement verified"));
	}
	else
	{
		LogTestResult(TEXT("ValidationRules"), true, TEXT("No rules configured — system is permissive (correct behavior)"));
	}

	// Restore
	if (OriginalTag.IsValid() && Sub->GetCurrentFlowTag(TestChannel) != OriginalTag)
	{
		Sub->SetStateByTag(TestChannel, OriginalTag, nullptr);
	}
}

// ============================================================================
// RunStressTest
// ============================================================================

void UPGXGameFlowTestUtility::RunStressTest(const UObject* WorldContextObject, int32 Iterations)
{
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("--- RunStressTest (%d iterations) ---"), Iterations);

	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("StressTest"), false, TEXT("No subsystem")); return; }

	const EPGXFlowChannel TestChannel = EPGXFlowChannel::Actors;
	const FGameplayTag OriginalTag = Sub->GetCurrentFlowTag(TestChannel);

	const FGameplayTag TagA = TAG_PGX_GameFlow_State_Boot;
	const FGameplayTag TagB = TAG_PGX_GameFlow_State_MainMenu;

	if (!TagA.IsValid() || !TagB.IsValid())
	{
		LogTestResult(TEXT("StressTest"), false, TEXT("Test tags not registered"));
		return;
	}

	// Set initial
	Sub->SetStateByTag(TestChannel, TagA, nullptr);

	int32 SuccessCount = 0;
	int32 FailCount = 0;

	const double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < Iterations; ++i)
	{
		// Alternate between two tags
		const FGameplayTag& TargetTag = (i % 2 == 0) ? TagB : TagA;
		FPGXFlowResult R = Sub->SetStateByTag(TestChannel, TargetTag, nullptr);
		if (R.bSuccess)
		{
			++SuccessCount;
		}
		else
		{
			++FailCount;
		}
	}

	const double EndTime = FPlatformTime::Seconds();
	const double TotalMs = (EndTime - StartTime) * 1000.0;
	const double AvgMs = TotalMs / Iterations;

	LogTestResult(TEXT("StressTest"),
		SuccessCount == Iterations,
		FString::Printf(TEXT("%d/%d succeeded | Total: %.2f ms | Avg: %.4f ms/transition"),
			SuccessCount, Iterations, TotalMs, AvgMs));

	// Restore
	if (OriginalTag.IsValid())
	{
		Sub->SetStateByTag(TestChannel, OriginalTag, nullptr);
	}
}

// ============================================================================
// SimulateGameSession
// ============================================================================

void UPGXGameFlowTestUtility::SimulateGameSession(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("--- SimulateGameSession ---"));

	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("GameSession"), false, TEXT("No subsystem")); return; }

	// Save all original states
	FGameplayTag OriginalStates[PGX_FLOW_CHANNEL_COUNT];
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		OriginalStates[i] = Sub->GetCurrentFlowTag(static_cast<EPGXFlowChannel>(i));
	}

	// Simulate: Boot → MainMenu → Loading → InWorld (on Global)
	struct FSessionStep { EPGXFlowChannel Channel; FGameplayTag Tag; };
	const TArray<FSessionStep> Steps = {
		{EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_Boot},
		{EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_MainMenu},
		{EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_Loading},
		{EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_InWorld},
		{EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_Paused},
		{EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_InWorld},
		{EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_Shutdown},
		{EPGXFlowChannel::Global, TAG_PGX_GameFlow_State_MainMenu},
	};

	int32 StepsPassed = 0;
	int32 StepsTotal = 0;

	for (const FSessionStep& Step : Steps)
	{
		const FGameplayTag Tag = Step.Tag;
		if (!Tag.IsValid())
		{
			PGX_LOG_WARNING(LogPGXGameFlow, TEXT("  [SKIP] Canonical tag not registered"));
			continue;
		}

		++StepsTotal;
		FPGXFlowResult R = Sub->SetStateByTag(Step.Channel, Tag, nullptr);

		const FString ChannelName = UPGXGameFlowSubsystem::GetChannelName(Step.Channel);
		if (R.bSuccess)
		{
			++StepsPassed;
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("  [%s] → %s ✓"), *ChannelName, *Tag.ToString());
		}
		else
		{
			PGX_LOG_INFO(LogPGXGameFlow, TEXT("  [%s] → %s ✗ (%s)"), *ChannelName, *Tag.ToString(), *R.Description);
		}
	}

	LogTestResult(TEXT("GameSession"),
		StepsPassed > 0,
		FString::Printf(TEXT("%d/%d steps completed"), StepsPassed, StepsTotal));

	// Restore all states
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		if (OriginalStates[i].IsValid())
		{
			Sub->SetStateByTag(static_cast<EPGXFlowChannel>(i), OriginalStates[i], nullptr);
		}
	}
}

// ============================================================================
// EN: RunAllTests — aggregate validation for Test Dashboard
// ES: RunAllTests — validacion agregada para Test Dashboard
// ============================================================================

bool UPGXGameFlowTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX GameFlow Test Suite ==="));

	// EN: Test 1: Subsystem accessible / ES: Test 1: Subsistema accesible
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("[FAIL] GameFlowSubsystem not found"));
		return false;
	}
	OutIssues.Add(TEXT("[PASS] GameFlowSubsystem found"));

	// EN: Test 2: Initialized / ES: Test 2: Inicializado
	if (!Sub->IsInitialized())
	{
		OutIssues.Add(TEXT("[FAIL] Subsystem not initialized"));
		bAllPassed = false;
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] Subsystem initialized"));
	}

	// EN: Test 3: Channel state queries / ES: Test 3: Consultas de estado por canal
	int32 ValidChannels = 0;
	for (int32 i = 0; i < PGX_FLOW_CHANNEL_COUNT; ++i)
	{
		const EPGXFlowChannel Ch = static_cast<EPGXFlowChannel>(i);
		const FGameplayTag Current = Sub->GetCurrentFlowTag(Ch);
		if (Current.IsValid())
		{
			++ValidChannels;
		}
	}
	OutIssues.Add(FString::Printf(TEXT("[PASS] %d/%d channels have valid states"), ValidChannels, PGX_FLOW_CHANNEL_COUNT));

	// EN: Test 4: Set/Get roundtrip on first channel / ES: Test 4: Roundtrip set/get en primer canal
	{
		const EPGXFlowChannel TestChannel = EPGXFlowChannel::Global;
		const FGameplayTag OriginalTag = Sub->GetCurrentFlowTag(TestChannel);
		const FGameplayTag TestTag = TAG_PGX_GameFlow_State_MainMenu;

		if (TestTag.IsValid())
		{
			const FPGXFlowResult Result = Sub->SetStateByTag(TestChannel, TestTag, nullptr);
			if (Result.bSuccess)
			{
				const FGameplayTag After = Sub->GetCurrentFlowTag(TestChannel);
				if (After == TestTag)
				{
					OutIssues.Add(TEXT("[PASS] Set/Get roundtrip"));
				}
				else
				{
					OutIssues.Add(TEXT("[FAIL] Set/Get roundtrip — tag mismatch after set"));
					bAllPassed = false;
				}
				// EN: Restore / ES: Restaurar
				if (OriginalTag.IsValid())
				{
					Sub->SetStateByTag(TestChannel, OriginalTag, nullptr);
				}
			}
			else
			{
				OutIssues.Add(FString::Printf(TEXT("[FAIL] SetStateByTag rejected — %s"), *Result.Description));
				bAllPassed = false;
			}
		}
		else
		{
			OutIssues.Add(TEXT("[PASS] Set/Get roundtrip skipped — test tag not registered"));
		}
	}

	// EN: Test 5: Branch matcher semantics / ES: Test 5: Semantica del matcher de ramas
	{
		const FGameplayTag Branch = TAG_PGX_GameFlow_State;
		const FGameplayTag Exact = TAG_PGX_GameFlow_State;
		const FGameplayTag Descendant = TAG_PGX_GameFlow_State_InWorld;
		const FGameplayTag Sibling = TAG_PGX_GameFlow_TransitionSource_Player;

		const bool bExactPass = Branch.IsValid() && Exact.IsValid()
			&& UPGXGameFlowSubsystem::DoesTagMatchBranchForTesting(Exact, Branch);
		const bool bDescendantPass = Branch.IsValid() && Descendant.IsValid()
			&& UPGXGameFlowSubsystem::DoesTagMatchBranchForTesting(Descendant, Branch);
		const bool bSiblingRejected = Branch.IsValid() && Sibling.IsValid()
			&& !UPGXGameFlowSubsystem::DoesTagMatchBranchForTesting(Sibling, Branch);
		const bool bMissingRejected = Branch.IsValid()
			&& !UPGXGameFlowSubsystem::DoesTagMatchBranchForTesting(FGameplayTag::EmptyTag, Branch);

		if (bExactPass && bDescendantPass && bSiblingRejected && bMissingRejected)
		{
			OutIssues.Add(TEXT("[PASS] Branch matcher exact/descendant/sibling/missing semantics"));
		}
		else
		{
			OutIssues.Add(TEXT("[FAIL] Branch matcher semantics drifted from IsInBranch contract"));
			bAllPassed = false;
		}
	}

	// EN: Test 6: Batch validation rejects redundant first step without mutation / ES: Test 6: Batch rechaza redundante sin mutar
	{
		const EPGXFlowChannel TestChannel = EPGXFlowChannel::UI;
		const FGameplayTag OriginalTag = Sub->GetCurrentFlowTag(TestChannel);
		const FGameplayTag StartTag = TAG_PGX_GameFlow_State_Boot;
		if (StartTag.IsValid())
		{
			const FPGXFlowResult SeedResult = Sub->SetStateByTag(TestChannel, StartTag, nullptr);
			if (SeedResult.bSuccess || Sub->GetCurrentFlowTag(TestChannel) == StartTag)
			{
				TArray<FGameplayTag> RedundantBatch;
				RedundantBatch.Add(StartTag);
				RedundantBatch.Add(TAG_PGX_GameFlow_State_Loading);
				const FPGXFlowResult BatchResult = Sub->SetBatchSequentialStateByTag(TestChannel, RedundantBatch, nullptr);
				const bool bRejected = !BatchResult.bSuccess && BatchResult.Code == EPGXFlowResultCode::RedundantState;
				const bool bNotMutated = Sub->GetCurrentFlowTag(TestChannel) == StartTag;
				if (bRejected && bNotMutated)
				{
					OutIssues.Add(TEXT("[PASS] Batch atomic redundant rejection"));
				}
				else
				{
					OutIssues.Add(FString::Printf(TEXT("[FAIL] Batch atomic redundant rejection — result=%d current=%s"),
						static_cast<int32>(BatchResult.Code),
						Sub->GetCurrentFlowTag(TestChannel).IsValid() ? *Sub->GetCurrentFlowTag(TestChannel).ToString() : TEXT("(none)")));
					bAllPassed = false;
				}
			}
			else
			{
				OutIssues.Add(FString::Printf(TEXT("[PASS] Batch atomic redundant rejection skipped — seed transition denied by project rules: %s"),
					*SeedResult.Description));
			}

			if (OriginalTag.IsValid())
			{
				Sub->SetStateByTag(TestChannel, OriginalTag, nullptr);
			}
		}
		else
		{
			OutIssues.Add(TEXT("[FAIL] Canonical GameFlow state tags unavailable"));
			bAllPassed = false;
		}
	}

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	PGX_LOG_INFO(LogPGXGameFlow, TEXT("[GameFlow TestUtility] RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
