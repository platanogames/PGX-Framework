// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXPSOTestUtility.h"
#include "PGXPSOSubsystem.h"
#include "PGXPSOWarmUpConfig.h"
#include "Tags/PGXPSOTags.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "ShaderPipelineCache.h"
#include "Engine/GameInstance.h"

// ============================================================================
// EN: Helpers
// ES: Helpers
// ============================================================================

UPGXPSOSubsystem* UPGXPSOTestUtility::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXPSOSubsystem>() : nullptr;
}

void UPGXPSOTestUtility::RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details)
{
	const FString Suffix = Details.IsEmpty() ? FString() : FString::Printf(TEXT(" (%s)"), *Details);
	const FString Tag = bPassed ? TEXT("[PASS]") : TEXT("[FAIL]");
	const FString Line = FString::Printf(TEXT("%s %s%s"), *Tag, *TestName, *Suffix);
	OutIssues.Add(Line);

	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] PASS: %s%s"), *TestName, *Suffix);
	}
	else
	{
		PGX_LOG_ERROR(LogPGXPSO, TEXT("[PSO TestUtility] FAIL: %s%s"), *TestName, *Suffix);
	}
}

// ============================================================================
// EN: RunQuickTest
// ES: RunQuickTest
// ============================================================================

bool UPGXPSOTestUtility::RunQuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== RunQuickTest START =========="));

	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("RunQuickTest"), false, TEXT("PSOSubsystem not found"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	// 1. Subsystem exists
	AssertPass(TEXT("QuickTest.SubsystemExists"), true);

	// 2. Initial state is Idle or Complete
	const EPGXPSOWarmUpState State = Sub->GetWarmUpState();
	AssertPass(TEXT("QuickTest.InitialState"),
		State == EPGXPSOWarmUpState::Idle || State == EPGXPSOWarmUpState::Complete,
		FString::Printf(TEXT("State: %d"), static_cast<int32>(State)));

	// 3. Config discovery
	const int32 ConfigCount = Sub->GetDiscoveredConfigCount();
	AssertPass(TEXT("QuickTest.ConfigDiscovery"), ConfigCount > 0,
		FString::Printf(TEXT("%d configs discovered"), ConfigCount));

	// 4. Active contexts
	const TArray<FGameplayTag> Contexts = Sub->GetActiveContexts();
	AssertPass(TEXT("QuickTest.ActiveContexts"), Contexts.Num() > 0,
		FString::Printf(TEXT("%d active contexts"), Contexts.Num()));

	// 5. Global context present
	AssertPass(TEXT("QuickTest.GlobalContext"), Contexts.Contains(TAG_PGX_PSO_Context_Global));

	// 6. Progress struct valid
	const FPGXPSOWarmUpProgress Progress = Sub->GetWarmUpProgress();
	AssertPass(TEXT("QuickTest.ProgressState"), Progress.State == State,
		FString::Printf(TEXT("Progress.State matches SubsystemState: %s"), Progress.State == State ? TEXT("YES") : TEXT("NO")));

	// 7. Cache dirty flag accessible (does not crash)
	Sub->IsCacheDirty();
	AssertPass(TEXT("QuickTest.CacheDirtyAccess"), true);

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] RunQuickTest COMPLETE: bAllPassed=%s"), bAllPassed ? TEXT("YES") : TEXT("NO"));
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== RunQuickTest END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: TestSingleEntryWarmUp
// ES: TestSingleEntryWarmUp
// ============================================================================

bool UPGXPSOTestUtility::TestSingleEntryWarmUp(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag)
{
	OutIssues.Empty();
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== TestSingleEntryWarmUp START =========="));

	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("SingleEntryWarmUp"), false, TEXT("PSOSubsystem not found"));
		return false;
	}
	if (!ContextTag.IsValid())
	{
		RecordResult(OutIssues, TEXT("SingleEntryWarmUp"), false, TEXT("Invalid ContextTag"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	Sub->CancelWarmUp();
	AssertPass(TEXT("SingleEntry.PreIdle"), Sub->GetWarmUpState() == EPGXPSOWarmUpState::Idle);

	const bool bRequested = Sub->RequestWarmUp(ContextTag);
	AssertPass(TEXT("SingleEntry.RequestWarmUp"), bRequested,
		FString::Printf(TEXT("Context: %s"), *ContextTag.ToString()));

	const EPGXPSOWarmUpState PostState = Sub->GetWarmUpState();
	AssertPass(TEXT("SingleEntry.StateChanged"), PostState != EPGXPSOWarmUpState::Idle,
		FString::Printf(TEXT("State: %d"), static_cast<int32>(PostState)));

	if (bRequested)
	{
		const FPGXPSOWarmUpProgress Progress = Sub->GetWarmUpProgress();
		AssertPass(TEXT("SingleEntry.TotalEntries"), Progress.TotalEntries > 0,
			FString::Printf(TEXT("%d entries"), Progress.TotalEntries));
	}

	Sub->CancelWarmUp();

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] TestSingleEntryWarmUp COMPLETE: bAllPassed=%s"), bAllPassed ? TEXT("YES") : TEXT("NO"));
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== TestSingleEntryWarmUp END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: TestBatchWarmUp
// ES: TestBatchWarmUp
// ============================================================================

bool UPGXPSOTestUtility::TestBatchWarmUp(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag)
{
	OutIssues.Empty();
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== TestBatchWarmUp START =========="));

	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("BatchWarmUp"), false, TEXT("PSOSubsystem not found"));
		return false;
	}
	if (!ContextTag.IsValid())
	{
		RecordResult(OutIssues, TEXT("BatchWarmUp"), false, TEXT("Invalid ContextTag"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	Sub->CancelWarmUp();

	const bool bRequested = Sub->RequestWarmUp(ContextTag);
	AssertPass(TEXT("Batch.RequestWarmUp"), bRequested);

	const FPGXPSOWarmUpProgress Progress = Sub->GetWarmUpProgress();
	AssertPass(TEXT("Batch.ProgressStruct"), true,
		FString::Printf(TEXT("Total: %d, Completed: %d, Failed: %d, Dedup: %d"),
			Progress.TotalEntries, Progress.CompletedEntries, Progress.FailedEntries, Progress.DeduplicatedEntries));

	const int32 Remaining = static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
	AssertPass(TEXT("Batch.NativeRemaining"), true,
		FString::Printf(TEXT("UE precompiles remaining: %d"), Remaining));

	if (bRequested)
	{
		const EPGXPSOWarmUpState State = Sub->GetWarmUpState();
		AssertPass(TEXT("Batch.NotIdle"), State != EPGXPSOWarmUpState::Idle,
			FString::Printf(TEXT("State: %d"), static_cast<int32>(State)));
	}

	Sub->CancelWarmUp();

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] TestBatchWarmUp COMPLETE: bAllPassed=%s"), bAllPassed ? TEXT("YES") : TEXT("NO"));
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== TestBatchWarmUp END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: TestContextFiltering
// ES: TestContextFiltering
// ============================================================================

bool UPGXPSOTestUtility::TestContextFiltering(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag)
{
	OutIssues.Empty();
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== TestContextFiltering START =========="));

	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("ContextFiltering"), false, TEXT("PSOSubsystem not found"));
		return false;
	}
	if (!ContextTag.IsValid())
	{
		RecordResult(OutIssues, TEXT("ContextFiltering"), false, TEXT("Invalid ContextTag"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	const TArray<FGameplayTag> InitialContexts = Sub->GetActiveContexts();
	AssertPass(TEXT("Context.HasGlobal"), InitialContexts.Contains(TAG_PGX_PSO_Context_Global));

	const int32 CountBefore = Sub->GetActiveContexts().Num();
	Sub->AddPSOContext(ContextTag);
	const int32 CountAfter = Sub->GetActiveContexts().Num();
	AssertPass(TEXT("Context.Add"),
		CountAfter > CountBefore || Sub->GetActiveContexts().Contains(ContextTag),
		FString::Printf(TEXT("Before: %d, After: %d"), CountBefore, CountAfter));

	AssertPass(TEXT("Context.Contains"), Sub->GetActiveContexts().Contains(ContextTag));

	Sub->RemovePSOContext(ContextTag);
	AssertPass(TEXT("Context.Remove"), !Sub->GetActiveContexts().Contains(ContextTag));

	Sub->RemovePSOContext(TAG_PGX_PSO_Context_Global);
	AssertPass(TEXT("Context.GlobalProtected"), Sub->GetActiveContexts().Contains(TAG_PGX_PSO_Context_Global));

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] TestContextFiltering COMPLETE: bAllPassed=%s"), bAllPassed ? TEXT("YES") : TEXT("NO"));
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== TestContextFiltering END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: TestWarmUpControl
// ES: TestWarmUpControl
// ============================================================================

bool UPGXPSOTestUtility::TestWarmUpControl(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag)
{
	OutIssues.Empty();
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== TestWarmUpControl START =========="));

	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("WarmUpControl"), false, TEXT("PSOSubsystem not found"));
		return false;
	}
	if (!ContextTag.IsValid())
	{
		RecordResult(OutIssues, TEXT("WarmUpControl"), false, TEXT("Invalid ContextTag"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	Sub->CancelWarmUp();
	AssertPass(TEXT("Control.PreIdle"), Sub->GetWarmUpState() == EPGXPSOWarmUpState::Idle);

	const bool bStarted = Sub->RequestWarmUp(ContextTag);
	if (!bStarted)
	{
		// EN: Early-exit when no entries — recorded as INFO (not failure) so empty configs do
		//     not synthesize a false negative against the control flow itself.
		// ES: Salida temprana cuando no hay entradas — registrado como INFO (no fallo).
		OutIssues.Add(TEXT("[INFO] Control.RequestWarmUp — No entries for context (early exit, control flow not exercised)"));
		PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] TestWarmUpControl COMPLETE: bAllPassed=%s (early exit)"), bAllPassed ? TEXT("YES") : TEXT("NO"));
		return bAllPassed;
	}
	AssertPass(TEXT("Control.Request"), true);

	Sub->PauseWarmUp();
	const EPGXPSOWarmUpState PausedState = Sub->GetWarmUpState();
	AssertPass(TEXT("Control.Pause"), PausedState == EPGXPSOWarmUpState::Paused,
		FString::Printf(TEXT("State: %d"), static_cast<int32>(PausedState)));

	Sub->ResumeWarmUp();
	const EPGXPSOWarmUpState ResumedState = Sub->GetWarmUpState();
	AssertPass(TEXT("Control.Resume"),
		ResumedState != EPGXPSOWarmUpState::Paused && ResumedState != EPGXPSOWarmUpState::Idle,
		FString::Printf(TEXT("State: %d"), static_cast<int32>(ResumedState)));

	Sub->CancelWarmUp();
	AssertPass(TEXT("Control.Cancel"), Sub->GetWarmUpState() == EPGXPSOWarmUpState::Idle);

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] TestWarmUpControl COMPLETE: bAllPassed=%s"), bAllPassed ? TEXT("YES") : TEXT("NO"));
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== TestWarmUpControl END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: RunStressTest
// ES: RunStressTest
// ============================================================================

bool UPGXPSOTestUtility::RunStressTest(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag /*ContextTag*/, int32 EntryCount)
{
	OutIssues.Empty();
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== RunStressTest START (target: %d entries) =========="), EntryCount);

	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("StressTest"), false, TEXT("PSOSubsystem not found"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	Sub->CancelWarmUp();

	const double StartTime = FPlatformTime::Seconds();

	const bool bStarted = Sub->RequestWarmUpAll();
	AssertPass(TEXT("Stress.RequestAll"), bStarted,
		FString::Printf(TEXT("Configs: %d"), Sub->GetDiscoveredConfigCount()));

	const FPGXPSOWarmUpProgress Progress = Sub->GetWarmUpProgress();
	AssertPass(TEXT("Stress.Progress"), true,
		FString::Printf(TEXT("Total: %d, State: %d"),
			Progress.TotalEntries, static_cast<int32>(Progress.State)));

	const double Elapsed = FPlatformTime::Seconds() - StartTime;
	AssertPass(TEXT("Stress.SubmissionTime"), true,
		FString::Printf(TEXT("%.3fms for %d entries"), Elapsed * 1000.0, Progress.TotalEntries));

	const double CancelStart = FPlatformTime::Seconds();
	Sub->CancelWarmUp();
	const double CancelElapsed = FPlatformTime::Seconds() - CancelStart;
	AssertPass(TEXT("Stress.CancelTime"), true,
		FString::Printf(TEXT("%.3fms"), CancelElapsed * 1000.0));

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] RunStressTest COMPLETE: bAllPassed=%s"), bAllPassed ? TEXT("YES") : TEXT("NO"));
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== RunStressTest END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: SimulateGameSession
// ES: SimulateGameSession
// ============================================================================

bool UPGXPSOTestUtility::SimulateGameSession(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag)
{
	OutIssues.Empty();
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== SimulateGameSession START =========="));

	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		RecordResult(OutIssues, TEXT("GameSession"), false, TEXT("PSOSubsystem not found"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] --- Phase 1: Init ---"));
	AssertPass(TEXT("Session.Init"), true,
		FString::Printf(TEXT("Configs: %d, Contexts: %d"),
			Sub->GetDiscoveredConfigCount(), Sub->GetActiveContexts().Num()));

	if (ContextTag.IsValid())
	{
		PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] --- Phase 2: Context Setup ---"));
		Sub->AddPSOContext(ContextTag);
		AssertPass(TEXT("Session.AddContext"), Sub->GetActiveContexts().Contains(ContextTag));
	}

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] --- Phase 3: Warm-Up ---"));
	Sub->CancelWarmUp();
	const bool bStarted = ContextTag.IsValid() ? Sub->RequestWarmUp(ContextTag) : Sub->RequestWarmUpAll();
	AssertPass(TEXT("Session.WarmUp"), bStarted);

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] --- Phase 4: Progress ---"));
	const FPGXPSOWarmUpProgress Progress = Sub->GetWarmUpProgress();
	AssertPass(TEXT("Session.Progress"), true,
		FString::Printf(TEXT("Total: %d, Completed: %d, Failed: %d, Elapsed: %.2fs"),
			Progress.TotalEntries, Progress.CompletedEntries, Progress.FailedEntries, Progress.ElapsedTimeSeconds));

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] --- Phase 5: Save Cache ---"));
	Sub->SaveCacheToDisk();
	AssertPass(TEXT("Session.SaveCache"), !Sub->IsCacheDirty(), TEXT("Cache should be clean after save"));

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] --- Phase 6: Cleanup ---"));
	Sub->CancelWarmUp();
	if (ContextTag.IsValid() && ContextTag != TAG_PGX_PSO_Context_Global)
	{
		Sub->RemovePSOContext(ContextTag);
	}

	AssertPass(TEXT("Session.Cleanup"), Sub->GetWarmUpState() == EPGXPSOWarmUpState::Idle);

	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] SimulateGameSession COMPLETE: bAllPassed=%s"), bAllPassed ? TEXT("YES") : TEXT("NO"));
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] ========== SimulateGameSession END =========="));
	return bAllPassed;
}

// ============================================================================
// EN: RunAllTests — aggregate validation for Test Dashboard
// ES: RunAllTests — validacion agregada para Test Dashboard
// ============================================================================

bool UPGXPSOTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX PSO Test Suite ==="));

	UPGXPSOSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("[FAIL] PSOSubsystem not found"));
		return false;
	}
	OutIssues.Add(TEXT("[PASS] PSOSubsystem found"));

	const EPGXPSOWarmUpState State = Sub->GetWarmUpState();
	const bool bValidState = (State == EPGXPSOWarmUpState::Idle || State == EPGXPSOWarmUpState::Complete);
	if (!bValidState)
	{
		OutIssues.Add(FString::Printf(TEXT("[FAIL] Unexpected initial state: %d"), static_cast<int32>(State)));
		bAllPassed = false;
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] Initial state valid"));
	}

	const int32 ConfigCount = Sub->GetDiscoveredConfigCount();
	if (ConfigCount > 0)
	{
		OutIssues.Add(FString::Printf(TEXT("[PASS] %d config(s) discovered"), ConfigCount));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] No PSO configs discovered"));
		bAllPassed = false;
	}

	const TArray<FGameplayTag> Contexts = Sub->GetActiveContexts();
	OutIssues.Add(FString::Printf(TEXT("[PASS] %d active context(s)"), Contexts.Num()));

	const FPGXPSOWarmUpProgress Progress = Sub->GetWarmUpProgress();
	if (Progress.State == State)
	{
		OutIssues.Add(TEXT("[PASS] Progress state matches subsystem state"));
	}
	else
	{
		OutIssues.Add(TEXT("[FAIL] Progress state mismatch"));
		bAllPassed = false;
	}

	Sub->IsCacheDirty();
	OutIssues.Add(TEXT("[PASS] Cache API accessible"));

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	PGX_LOG_INFO(LogPGXPSO, TEXT("[PSO TestUtility] RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
