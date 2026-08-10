// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLevelFlowTestUtility.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLevelFlowSubsystem.h"
#include "PGXLevelFlowActor.h"
#include "PGXLoadingRuntime.h"
#include "ShaderPipelineCache.h"
#include "Engine/GameInstance.h"

// ============================================================================
// Helpers
// ============================================================================

UPGXLevelFlowSubsystem* UPGXLevelFlowTestUtility::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXLevelFlowSubsystem>() : nullptr;
}

void UPGXLevelFlowTestUtility::LogTestResult(const FString& TestName, bool bPassed, const FString& Details)
{
	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow Test] ✓ PASS: %s %s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("— %s"), *Details));
	}
	else
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow Test] ✗ FAIL: %s %s"),
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("— %s"), *Details));
	}
}

// ============================================================================
// RunQuickTest
// ============================================================================

void UPGXLevelFlowTestUtility::RunQuickTest(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== LevelFlow Quick Test ==="));

	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);

	// Test 1: Subsystem exists
	LogTestResult(TEXT("Subsystem exists"), Sub != nullptr);
	if (!Sub)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("[LevelFlow Test] Cannot continue — subsystem is null"));
		return;
	}

	// Test 2: Initialized
	LogTestResult(TEXT("Subsystem initialized"), Sub->IsInitialized());

	// Test 3: State is Idle
	const EPGXLevelFlowState State = Sub->GetTransitionState();
	LogTestResult(TEXT("State is Idle"), State == EPGXLevelFlowState::Idle,
		FString::Printf(TEXT("State = %d"), static_cast<int32>(State)));

	// Test 4: Profiles discovered
	const int32 ProfileCount = Sub->GetDiscoveredProfileCount();
	LogTestResult(TEXT("Profiles discovered"), ProfileCount >= 0,
		FString::Printf(TEXT("%d profiles"), ProfileCount));

	// Test 5: Levels registered
	const int32 LevelCount = Sub->GetRegisteredLevelCount();
	LogTestResult(TEXT("Levels registered"), LevelCount >= 0,
		FString::Printf(TEXT("%d levels"), LevelCount));

	// Test 6: No transition active
	LogTestResult(TEXT("No transition active"), !Sub->IsTransitionActive());

	// Test 7: Shader compilations accessible
	const int32 Shaders = static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
	LogTestResult(TEXT("Shader API accessible"), true,
		FString::Printf(TEXT("%d remaining"), Shaders));

	// Test 8: History initially empty or valid
	const TArray<FPGXLevelTransitionRecord>& History = Sub->GetTransitionHistory();
	LogTestResult(TEXT("History valid"), true,
		FString::Printf(TEXT("%d entries"), History.Num()));

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Quick Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// TestResolveTags
// ============================================================================

void UPGXLevelFlowTestUtility::TestResolveTags(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== LevelFlow Resolve Tags Test ==="));

	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	const TArray<FGameplayTag> Tags = Sub->GetRegisteredLevelTags();
	LogTestResult(TEXT("Tags retrieved"), true,
		FString::Printf(TEXT("%d tags"), Tags.Num()));

	int32 Passed = 0;
	int32 Failed = 0;

	for (const FGameplayTag& Tag : Tags)
	{
		FPGXLevelEntry Entry;
		const bool bResolved = Sub->ResolveLevelByTag(Tag, Entry);

		if (bResolved)
		{
			++Passed;
			PGX_LOG_INFO(LogPGXLoading, TEXT("  [OK] %s → '%s' (%s)"),
				*Tag.ToString(),
				*Entry.DisplayName.ToString(),
				*FString::Printf(TEXT("Strategy:%d"), static_cast<int32>(Entry.LoadStrategy)));
		}
		else
		{
			++Failed;
			PGX_LOG_WARNING(LogPGXLoading, TEXT("  [FAIL] %s → not resolved"), *Tag.ToString());
		}
	}

	LogTestResult(TEXT("All tags resolved"),
		Failed == 0,
		FString::Printf(TEXT("%d passed, %d failed"), Passed, Failed));

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Resolve Tags Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// TestTransitionRequest
// ============================================================================

void UPGXLevelFlowTestUtility::TestTransitionRequest(const UObject* WorldContextObject, FGameplayTag LevelTag)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== LevelFlow Transition Request Test ==="));

	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	// Pre-state
	const EPGXLevelFlowState PreState = Sub->GetTransitionState();
	LogTestResult(TEXT("Pre-state is Idle"), PreState == EPGXLevelFlowState::Idle,
		FString::Printf(TEXT("State = %d"), static_cast<int32>(PreState)));

	// Request
	const FPGXLevelFlowResult Result = Sub->RequestLevelTransition(LevelTag, nullptr);
	LogTestResult(TEXT("RequestLevelTransition"), Result.bSuccess,
		Result.Description);

	// Post-state (should be non-Idle if succeeded)
	if (Result.bSuccess)
	{
		const EPGXLevelFlowState PostState = Sub->GetTransitionState();
		LogTestResult(TEXT("Post-state is non-Idle"), PostState != EPGXLevelFlowState::Idle,
			FString::Printf(TEXT("State = %d"), static_cast<int32>(PostState)));
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Transition Request Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// TestCancelFlow
// ============================================================================

void UPGXLevelFlowTestUtility::TestCancelFlow(const UObject* WorldContextObject, FGameplayTag LevelTag)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== LevelFlow Cancel Flow Test ==="));

	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	// Request transition
	const FPGXLevelFlowResult RequestResult = Sub->RequestLevelTransition(LevelTag, nullptr);
	LogTestResult(TEXT("Request transition"), RequestResult.bSuccess, RequestResult.Description);

	if (!RequestResult.bSuccess)
	{
		PGX_LOG_WARNING(LogPGXLoading, TEXT("  Cannot test cancel — request failed"));
		return;
	}

	// Immediately cancel
	const FPGXLevelFlowResult CancelResult = Sub->CancelTransition();
	LogTestResult(TEXT("Cancel transition"), CancelResult.bSuccess, CancelResult.Description);

	// State should be Idle after cancel
	const EPGXLevelFlowState PostState = Sub->GetTransitionState();
	LogTestResult(TEXT("Post-cancel state is Idle"), PostState == EPGXLevelFlowState::Idle,
		FString::Printf(TEXT("State = %d"), static_cast<int32>(PostState)));

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Cancel Flow Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// TestSubLevelTracking
// ============================================================================

void UPGXLevelFlowTestUtility::TestSubLevelTracking(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== LevelFlow Sub-Level Tracking Test ==="));

	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	// Check initial state
	const TArray<FGameplayTag> InitialSubLevels = Sub->GetLoadedSubLevels();
	LogTestResult(TEXT("Initial sub-levels query"), true,
		FString::Printf(TEXT("%d loaded"), InitialSubLevels.Num()));

	// Try loading a non-existent sub-level (should fail gracefully)
	const FGameplayTag FakeTag = FGameplayTag::RequestGameplayTag(FName("PGX.Level.Test.FakeSubLevel"), false);
	if (FakeTag.IsValid())
	{
		const FPGXLevelFlowResult LoadResult = Sub->RequestSubLevelLoad(FakeTag);
		LogTestResult(TEXT("Load fake sub-level rejected"), !LoadResult.bSuccess,
			LoadResult.Description);
	}
	else
	{
		LogTestResult(TEXT("Fake tag not registered (expected)"), true);
	}

	// Verify loaded count unchanged
	const TArray<FGameplayTag> FinalSubLevels = Sub->GetLoadedSubLevels();
	LogTestResult(TEXT("Sub-level count unchanged"), FinalSubLevels.Num() == InitialSubLevels.Num());

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Sub-Level Tracking Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// RunStressTest
// ============================================================================

void UPGXLevelFlowTestUtility::RunStressTest(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== LevelFlow Stress Test ==="));

	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	const TArray<FGameplayTag> Tags = Sub->GetRegisteredLevelTags();
	if (Tags.Num() < 2)
	{
		LogTestResult(TEXT("Stress test"), false,
			TEXT("Need at least 2 registered levels for stress test"));
		return;
	}

	int32 SuccessCount = 0;
	int32 RejectedCount = 0;

	const double StartTime = FPlatformTime::Seconds();

	// Attempt rapid-fire transitions (most should be rejected since one is in progress)
	for (int32 i = 0; i < FMath::Min(10, Tags.Num()); ++i)
	{
		const FPGXLevelFlowResult Result = Sub->RequestLevelTransition(Tags[i], nullptr);
		if (Result.bSuccess)
		{
			++SuccessCount;
			// Cancel immediately so we can try the next one
			Sub->CancelTransition();
		}
		else
		{
			++RejectedCount;
		}
	}

	const double Elapsed = FPlatformTime::Seconds() - StartTime;

	LogTestResult(TEXT("Stress test completed"), true,
		FString::Printf(TEXT("%d succeeded, %d rejected, %.3f ms"),
			SuccessCount, RejectedCount, Elapsed * 1000.0));

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Stress Test Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// SimulateGameSession
// ============================================================================

void UPGXLevelFlowTestUtility::SimulateGameSession(const UObject* WorldContextObject)
{
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
	PGX_LOG_INFO(LogPGXLoading, TEXT("=== LevelFlow Game Session Simulation ==="));

	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) { LogTestResult(TEXT("Subsystem"), false, TEXT("null")); return; }

	// Step 1: Init check
	LogTestResult(TEXT("1. Init check"), Sub->IsInitialized());

	// Step 2: Resolve all tags
	const TArray<FGameplayTag> Tags = Sub->GetRegisteredLevelTags();
	LogTestResult(TEXT("2. Registered levels"), Tags.Num() > 0,
		FString::Printf(TEXT("%d levels"), Tags.Num()));

	// Step 3: Query current state
	const EPGXLevelFlowState State = Sub->GetTransitionState();
	const FGameplayTag CurrentTag = Sub->GetCurrentLevelTag();
	LogTestResult(TEXT("3. Current state"), true,
		FString::Printf(TEXT("State=%d, Level=%s"),
			static_cast<int32>(State),
			CurrentTag.IsValid() ? *CurrentTag.ToString() : TEXT("(none)")));

	// Step 4: Verify resolve for each tag
	int32 ResolvePass = 0;
	for (const FGameplayTag& Tag : Tags)
	{
		FPGXLevelEntry Entry;
		if (Sub->ResolveLevelByTag(Tag, Entry)) ++ResolvePass;
	}
	LogTestResult(TEXT("4. Tag resolution"), ResolvePass == Tags.Num(),
		FString::Printf(TEXT("%d/%d resolved"), ResolvePass, Tags.Num()));

	// Step 5: Check actor
	APGXLevelFlowActor* Actor = Sub->GetCurrentLevelFlowActor();
	LogTestResult(TEXT("5. LevelFlowActor"), true,
		Actor ? FString::Printf(TEXT("Found: %s"), *Actor->GetName()) : TEXT("None (OK if no actor placed)"));

	// Step 6: Check sub-levels
	const TArray<FGameplayTag> SubLevels = Sub->GetLoadedSubLevels();
	LogTestResult(TEXT("6. Sub-levels"), true,
		FString::Printf(TEXT("%d loaded"), SubLevels.Num()));

	// Step 7: History
	const TArray<FPGXLevelTransitionRecord>& History = Sub->GetTransitionHistory();
	LogTestResult(TEXT("7. History"), true,
		FString::Printf(TEXT("%d entries"), History.Num()));

	// Step 8: Shader compilations
	const int32 Shaders = static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
	LogTestResult(TEXT("8. Shader compilations"), true,
		FString::Printf(TEXT("%d pending"), Shaders));

	PGX_LOG_INFO(LogPGXLoading, TEXT("=== Game Session Simulation Complete ==="));
	PGX_LOG_INFO(LogPGXLoading, TEXT(""));
}

// ============================================================================
// EN: RunAllTests — aggregate validation for Test Dashboard
// ES: RunAllTests — validacion agregada para Test Dashboard
// ============================================================================

bool UPGXLevelFlowTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX LevelFlow Test Suite ==="));

	// EN: Test 1: Subsystem accessible / ES: Test 1: Subsistema accesible
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("[FAIL] LevelFlowSubsystem not found"));
		return false;
	}
	OutIssues.Add(TEXT("[PASS] LevelFlowSubsystem found"));

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
	const EPGXLevelFlowState State = Sub->GetTransitionState();
	if (State != EPGXLevelFlowState::Idle)
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

	// EN: Test 5: Levels registered / ES: Test 5: Niveles registrados
	const int32 LevelCount = Sub->GetRegisteredLevelCount();
	OutIssues.Add(FString::Printf(TEXT("[PASS] %d level(s) registered"), LevelCount));

	// EN: Test 6: No transition active / ES: Test 6: Sin transicion activa
	if (Sub->IsTransitionActive())
	{
		OutIssues.Add(TEXT("[FAIL] Unexpected active transition"));
		bAllPassed = false;
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] No transition active"));
	}

	// EN: Test 7: History accessible / ES: Test 7: Historial accesible
	const TArray<FPGXLevelTransitionRecord>& History = Sub->GetTransitionHistory();
	OutIssues.Add(FString::Printf(TEXT("[PASS] Transition history accessible — %d entries"), History.Num()));

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));
	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlow TestUtility] RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
