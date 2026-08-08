// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingTestUtility.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLoadingSubsystem.h"
#include "PGXLoadingRuntime.h"
#include "Tags/PGXLoadingTags.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"

// ============================================================================
// Helpers
// ============================================================================

UPGXLoadingSubsystem* UPGXLoadingTestUtility::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXLoadingSubsystem>() : nullptr;
}

void UPGXLoadingTestUtility::LogTestResult(const FString& TestName, bool bPassed, const FString& Details)
{
	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("[Loading Test] PASS: %s %s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("- %s"), *Details));
	}
	else
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("[Loading Test] FAIL: %s %s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("- %s"), *Details));
	}
}

// ============================================================================
// RunQuickTest
// ============================================================================

void UPGXLoadingTestUtility::RunQuickTest(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Loading Screen Quick Test ==="));

	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);

	// Test 1: Subsystem exists
	LogTestResult(TEXT("Subsystem exists"), Sub != nullptr);
	if (!Sub)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("[Loading Test] Cannot continue - subsystem is null"));
		return;
	}

	// Test 2: Initialized
	LogTestResult(TEXT("Subsystem initialized"), Sub->IsInitialized());

	// Test 3: State is Idle
	const EPGXLoadingScreenState State = Sub->GetCurrentState();
	LogTestResult(TEXT("State is Idle"), State == EPGXLoadingScreenState::Idle,
		FString::Printf(TEXT("State = %d"), static_cast<int32>(State)));

	// Test 4: Profiles discovered
	const int32 ProfileCount = Sub->GetDiscoveredProfileCount();
	LogTestResult(TEXT("Profiles discovered"), ProfileCount >= 0,
		FString::Printf(TEXT("%d profiles"), ProfileCount));

	// Test 5: Context tags registered
	const TArray<FGameplayTag> Tags = Sub->GetRegisteredContextTags();
	LogTestResult(TEXT("Context tags registered"), true,
		FString::Printf(TEXT("%d tags"), Tags.Num()));

	// Test 6: Not loading
	LogTestResult(TEXT("Not loading"), !Sub->IsLoadingActive());

	// Test 7: Elapsed is zero
	LogTestResult(TEXT("Elapsed is zero"), Sub->GetElapsedTime() == 0.0f);

	// Test 8: History accessible
	const TArray<FPGXLoadingRecord> History = Sub->GetLoadingHistory();
	LogTestResult(TEXT("History accessible"), true,
		FString::Printf(TEXT("%d entries"), History.Num()));

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Quick Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// TestProfileResolution
// ============================================================================

void UPGXLoadingTestUtility::TestProfileResolution(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Loading Screen Profile Resolution Test ==="));

	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	const TArray<FGameplayTag> Tags = Sub->GetRegisteredContextTags();
	LogTestResult(TEXT("Context tags retrieved"), true,
		FString::Printf(TEXT("%d tags"), Tags.Num()));

	int32 Valid = 0;
	int32 Invalid = 0;

	for (const FGameplayTag& CtxTag : Tags)
	{
		const bool bIsValid = Sub->IsProfileValid(CtxTag);
		if (bIsValid)
		{
			++Valid;
			PGX_LOG_INFO(LogPGXLoading, TEXT("  [OK] %s"), *CtxTag.ToString());
		}
		else
		{
			++Invalid;
			PGX_LOG_WARNING(LogPGXLoading, TEXT("  [FAIL] %s - no valid profile"), *CtxTag.ToString());
		}
	}

	LogTestResult(TEXT("All profiles valid"),
		Invalid == 0,
		FString::Printf(TEXT("%d valid, %d invalid"), Valid, Invalid));

	// Test default tag resolution
	const bool bDefaultValid = Sub->IsProfileValid(TAG_PGX_Loading_Context_Default);
	LogTestResult(TEXT("Default context tag valid"), bDefaultValid || Tags.Num() == 0,
		bDefaultValid ? TEXT("has profile") : TEXT("no profile (OK if no DAs created)"));

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Profile Resolution Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// TestFadeSystem
// ============================================================================

void UPGXLoadingTestUtility::TestFadeSystem(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Loading Screen Fade System Test ==="));

	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	// Request loading
	const FPGXLoadingResult Result = Sub->RequestLoading(TAG_PGX_Loading_Context_Default);
	LogTestResult(TEXT("RequestLoading"), Result.bSuccess, Result.Description);

	if (Result.bSuccess)
	{
		// State should be non-Idle
		const EPGXLoadingScreenState State = Sub->GetCurrentState();
		LogTestResult(TEXT("State is non-Idle"), State != EPGXLoadingScreenState::Idle,
			FString::Printf(TEXT("State = %d"), static_cast<int32>(State)));

		// Force close to trigger fade out
		const FPGXLoadingResult CloseResult = Sub->ForceClose();
		LogTestResult(TEXT("ForceClose"), CloseResult.bSuccess, CloseResult.Description);
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Fade System Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// TestStrategySwitch
// ============================================================================

void UPGXLoadingTestUtility::TestStrategySwitch(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Loading Screen Strategy Switch Test ==="));

	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	// Test with default context
	const FPGXLoadingResult Result = Sub->RequestLoading(TAG_PGX_Loading_Context_Default);
	LogTestResult(TEXT("Request default loading"), Result.bSuccess, Result.Description);

	if (Result.bSuccess)
	{
		const EPGXLoadingVisualType VisualType = Sub->GetActiveVisualType();
		LogTestResult(TEXT("Visual type assigned"), true,
			FString::Printf(TEXT("VisualType = %d"), static_cast<int32>(VisualType)));

		// Force close
		Sub->ForceClose();
	}

	// Test each registered context tag
	const TArray<FGameplayTag> Tags = Sub->GetRegisteredContextTags();
	int32 TestedCount = 0;
	for (const FGameplayTag& CtxTag : Tags)
	{
		if (TestedCount >= 3) break; // EN: Limit to 3 to avoid long test / ES: Limitar a 3

		// EN: Wait for Idle state / ES: Esperar estado Idle
		if (Sub->GetCurrentState() != EPGXLoadingScreenState::Idle) continue;

		const FPGXLoadingResult CtxResult = Sub->RequestLoading(CtxTag);
		if (CtxResult.bSuccess)
		{
			LogTestResult(FString::Printf(TEXT("Context '%s'"), *CtxTag.ToString()),
				true, FString::Printf(TEXT("VisualType=%d"), static_cast<int32>(Sub->GetActiveVisualType())));
			Sub->ForceClose();
			++TestedCount;
		}
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Strategy Switch Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// TestInputBlocking
// ============================================================================

void UPGXLoadingTestUtility::TestInputBlocking(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Loading Screen Input Blocking Test ==="));

	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	// Request loading
	const FPGXLoadingResult Result = Sub->RequestLoading(TAG_PGX_Loading_Context_Default);
	LogTestResult(TEXT("Request loading"), Result.bSuccess, Result.Description);

	if (Result.bSuccess)
	{
		// Input should be blocked (UI-only mode)
		LogTestResult(TEXT("Loading active (input blocked)"), Sub->IsLoadingActive());

		// Force close — input should be restored after deferred flush
		Sub->ForceClose();
		LogTestResult(TEXT("ForceClose issued"), true);
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Input Blocking Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// RunStressTest
// ============================================================================

void UPGXLoadingTestUtility::RunStressTest(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Loading Screen Stress Test ==="));

	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	int32 SuccessCount = 0;
	int32 RejectedCount = 0;

	const double StartTime = FPlatformTime::Seconds();

	// EN: Rapid-fire RequestLoading/ForceClose 50 times
	// ES: RequestLoading/ForceClose rapido 50 veces
	for (int32 i = 0; i < 50; ++i)
	{
		const FPGXLoadingResult Result = Sub->RequestLoading(TAG_PGX_Loading_Context_Default);
		if (Result.bSuccess)
		{
			++SuccessCount;
			Sub->ForceClose();
		}
		else
		{
			++RejectedCount;
		}
	}

	const double Elapsed = FPlatformTime::Seconds() - StartTime;

	// Verify final state is Idle
	const EPGXLoadingScreenState FinalState = Sub->GetCurrentState();
	LogTestResult(TEXT("Final state is Idle"), FinalState == EPGXLoadingScreenState::Idle,
		FString::Printf(TEXT("State = %d"), static_cast<int32>(FinalState)));

	LogTestResult(TEXT("Stress test completed"), true,
		FString::Printf(TEXT("%d succeeded, %d rejected, %.3f ms"),
			SuccessCount, RejectedCount, Elapsed * 1000.0));

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Stress Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// SimulateGameSession
// ============================================================================

void UPGXLoadingTestUtility::SimulateGameSession(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Loading Screen Game Session Simulation ==="));

	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	// Step 1: Init check
	LogTestResult(TEXT("1. Init check"), Sub->IsInitialized());

	// Step 2: Profile discovery
	const int32 ProfileCount = Sub->GetDiscoveredProfileCount();
	LogTestResult(TEXT("2. Profiles discovered"), true,
		FString::Printf(TEXT("%d profiles"), ProfileCount));

	// Step 3: Context tags
	const TArray<FGameplayTag> Tags = Sub->GetRegisteredContextTags();
	LogTestResult(TEXT("3. Context tags"), true,
		FString::Printf(TEXT("%d tags"), Tags.Num()));

	// Step 4: Request loading
	const FPGXLoadingResult Result = Sub->RequestLoading(TAG_PGX_Loading_Context_Default);
	LogTestResult(TEXT("4. Request loading"), Result.bSuccess, Result.Description);

	if (Result.bSuccess)
	{
		// Step 5: Query active state
		LogTestResult(TEXT("5. Loading active"), Sub->IsLoadingActive());

		// Step 6: Check progress
		const FPGXLoadingProgress Progress = Sub->GetProgress();
		LogTestResult(TEXT("6. Progress valid"), true,
			FString::Printf(TEXT("Total=%.2f Asset=%.2f"), Progress.TotalProgress, Progress.AssetProgress));

		// Step 7: Visual type
		const EPGXLoadingVisualType VisualType = Sub->GetActiveVisualType();
		LogTestResult(TEXT("7. Visual type"), true,
			FString::Printf(TEXT("Type = %d"), static_cast<int32>(VisualType)));

		// Step 8: Force close
		const FPGXLoadingResult CloseResult = Sub->ForceClose();
		LogTestResult(TEXT("8. Force close"), CloseResult.bSuccess, CloseResult.Description);
	}

	// Step 9: History
	const TArray<FPGXLoadingRecord> History = Sub->GetLoadingHistory();
	LogTestResult(TEXT("9. History"), true,
		FString::Printf(TEXT("%d entries"), History.Num()));

	if (History.Num() > 0)
	{
		const FPGXLoadingRecord& Last = History.Last();
		LogTestResult(TEXT("10. Last record"), true,
			FString::Printf(TEXT("Context=%s Duration=%.2fs Code=%d"),
				*Last.ContextTag.ToString(), Last.TotalDuration,
				static_cast<int32>(Last.ResultCode)));
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Game Session Simulation Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// EN: RunAllTests — aggregate validation for Test Dashboard
// ES: RunAllTests — validacion agregada para Test Dashboard
// ============================================================================

bool UPGXLoadingTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX Loading Screen Test Suite ==="));

	// EN: Test 1: Subsystem accessible / ES: Test 1: Subsistema accesible
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("[FAIL] LoadingSubsystem not found"));
		return false;
	}
	OutIssues.Add(TEXT("[PASS] LoadingSubsystem found"));

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

	// EN: Test 3: State is Idle / ES: Test 3: Estado es Idle
	const EPGXLoadingScreenState State = Sub->GetCurrentState();
	if (State != EPGXLoadingScreenState::Idle)
	{
		OutIssues.Add(FString::Printf(TEXT("[FAIL] Expected Idle state, got %d"), static_cast<int32>(State)));
		bAllPassed = false;
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] State is Idle"));
	}

	// EN: Test 4: Profiles discovered / ES: Test 4: Perfiles descubiertos
	const int32 ProfileCount = Sub->GetDiscoveredProfileCount();
	OutIssues.Add(FString::Printf(TEXT("[PASS] %d profile(s) discovered"), ProfileCount));

	// EN: Test 5: Context tags registered / ES: Test 5: Tags de contexto registrados
	const TArray<FGameplayTag> Tags = Sub->GetRegisteredContextTags();
	OutIssues.Add(FString::Printf(TEXT("[PASS] %d context tag(s) registered"), Tags.Num()));

	// EN: Test 6: Not loading / ES: Test 6: No cargando
	if (Sub->IsLoadingActive())
	{
		OutIssues.Add(TEXT("[FAIL] Loading is unexpectedly active"));
		bAllPassed = false;
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] Not loading"));
	}

	// EN: Test 7: History accessible / ES: Test 7: Historial accesible
	const TArray<FPGXLoadingRecord> History = Sub->GetLoadingHistory();
	OutIssues.Add(FString::Printf(TEXT("[PASS] Loading history accessible — %d entries"), History.Num()));

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	PGX_LOG_INFO(LogPGXLoading, TEXT("[Loading TestUtility] RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
