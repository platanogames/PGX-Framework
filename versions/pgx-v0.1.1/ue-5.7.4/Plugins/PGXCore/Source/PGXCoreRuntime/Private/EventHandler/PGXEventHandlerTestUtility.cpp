// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/PGXEventHandlerTestUtility.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "PGXEventHandlerTestHelpers.h"
#include "EventHandler/PGXEventHandlerConfig.h"
#include "Engine/DataTable.h"
#include "GameplayTagsManager.h"
#include "StructUtils/InstancedStruct.h"

// ============================================================
// Test helpers
// ============================================================

static FPGXEventContext MakeTestContext()
{
	return FPGXEventContext();
}

static FGameplayTag MakeEventHandlerTestTag(const TCHAR* Name)
{
	return UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName(*FString::Printf(TEXT("PGX.Test.EventHandler.%s"), Name)),
		TEXT("PGX EventHandler test tag"));
}

static bool LatestEntryForTag(const UPGXEventHandlerSubsystem* Sub, FGameplayTag EventTag, FPGXBlackboxEntry& OutEntry)
{
	if (!IsValid(Sub))
	{
		return false;
	}

	const TArray<FPGXBlackboxEntry>& Entries = Sub->GetBlackboxEntries();
	for (int32 Index = Entries.Num() - 1; Index >= 0; --Index)
	{
		if (Entries[Index].EventTag == EventTag)
		{
			OutEntry = Entries[Index];
			return true;
		}
	}
	return false;
}

// ============================================================
// Tests
// ============================================================

bool UPGXEventHandlerTestUtility::QuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Reset();

	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub))
	{
		OutIssues.Add(TEXT("EventHandler subsystem not available"));
		return false;
	}

	// EN: Test basic registration + execution cycle
	const FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Quick"));
	Sub->RegisterHandler(TestTag, UPGXTestSuccessHandler::StaticClass(), EPGXHandlerLifecycle::Ephemeral);

	if (!Sub->IsHandlerRegistered(TestTag))
	{
		OutIssues.Add(TEXT("Handler registration failed"));
	}

	FInstancedStruct EmptyPayload;
	EPGXEventResult Result = Sub->ResolveAndExecuteWithContext(TestTag, MakeTestContext(), EmptyPayload);
	if (Result != EPGXEventResult::Success)
	{
		OutIssues.Add(FString::Printf(TEXT("Execution failed: %d"), static_cast<int32>(Result)));
	}

	Sub->UnregisterHandler(TestTag);
	if (Sub->IsHandlerRegistered(TestTag))
	{
		OutIssues.Add(TEXT("Handler unregistration failed"));
	}

	return OutIssues.Num() == 0;
}

bool UPGXEventHandlerTestUtility::LifecycleTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Reset();

	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub))
	{
		OutIssues.Add(TEXT("EventHandler subsystem not available"));
		return false;
	}

	const FGameplayTag STag = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Singleton"));
	const FGameplayTag CTag = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Cached"));
	const FGameplayTag ETag = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Ephemeral"));

	// EN: Register all three lifecycle types
	Sub->RegisterHandler(STag, UPGXTestSuccessHandler::StaticClass(), EPGXHandlerLifecycle::Singleton);
	Sub->RegisterHandler(CTag, UPGXTestSuccessHandler::StaticClass(), EPGXHandlerLifecycle::Cached);
	Sub->RegisterHandler(ETag, UPGXTestSuccessHandler::StaticClass(), EPGXHandlerLifecycle::Ephemeral);

	FInstancedStruct EmptyPayload;
	FPGXEventContext Ctx = MakeTestContext();

	// EN: Execute each — all should succeed
	if (Sub->ResolveAndExecuteWithContext(STag, Ctx, EmptyPayload) != EPGXEventResult::Success)
	{
		OutIssues.Add(TEXT("Singleton execution failed"));
	}
	if (Sub->ResolveAndExecuteWithContext(CTag, Ctx, EmptyPayload) != EPGXEventResult::Success)
	{
		OutIssues.Add(TEXT("Cached execution failed"));
	}
	if (Sub->ResolveAndExecuteWithContext(ETag, Ctx, EmptyPayload) != EPGXEventResult::Success)
	{
		OutIssues.Add(TEXT("Ephemeral execution failed"));
	}

	// EN: Singleton should be cached after first execution
	FPGXHandlerCacheStats Stats = Sub->GetCacheStats();
	if (Stats.CachedHandlers < 1)
	{
		OutIssues.Add(TEXT("Expected at least 1 cached handler (singleton)"));
	}

	// EN: Cleanup
	Sub->UnregisterHandler(STag);
	Sub->UnregisterHandler(CTag);
	Sub->UnregisterHandler(ETag);

	return OutIssues.Num() == 0;
}

bool UPGXEventHandlerTestUtility::PayloadTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Reset();

	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub))
	{
		OutIssues.Add(TEXT("EventHandler subsystem not available"));
		return false;
	}

	// EN: Register handler and execute with empty payload — should succeed
	const FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Payload"));
	Sub->RegisterHandler(TestTag, UPGXTestSuccessHandler::StaticClass(), EPGXHandlerLifecycle::Ephemeral);

	FInstancedStruct EmptyPayload;
	EPGXEventResult Result = Sub->ResolveAndExecuteWithContext(TestTag, MakeTestContext(), EmptyPayload);
	if (Result != EPGXEventResult::Success)
	{
		OutIssues.Add(TEXT("Payload execution failed with empty payload"));
	}

	Sub->UnregisterHandler(TestTag);
	return OutIssues.Num() == 0;
}

bool UPGXEventHandlerTestUtility::ConditionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Reset();

	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub))
	{
		OutIssues.Add(TEXT("EventHandler subsystem not available"));
		return false;
	}

	const FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Condition"));
	Sub->RegisterHandler(TestTag, UPGXTestConditionHandler::StaticClass(), EPGXHandlerLifecycle::Ephemeral);

	FInstancedStruct EmptyPayload;

	// EN: Without the allow tag, should be skipped
	FPGXEventContext CtxDenied = MakeTestContext();
	EPGXEventResult DeniedResult = Sub->ResolveAndExecuteWithContext(TestTag, CtxDenied, EmptyPayload);
	if (DeniedResult != EPGXEventResult::Skipped)
	{
		OutIssues.Add(FString::Printf(TEXT("Expected Skipped without allow tag, got %d"), static_cast<int32>(DeniedResult)));
	}

	// EN: With the allow tag, should succeed
	FPGXEventContext CtxAllowed = MakeTestContext();
	const FGameplayTag AllowTag = FGameplayTag::RequestGameplayTag(FName("PGX.Test.Allow"));
	CtxAllowed.SourceTags.AddTag(AllowTag);
	EPGXEventResult AllowedResult = Sub->ResolveAndExecuteWithContext(TestTag, CtxAllowed, EmptyPayload);
	if (AllowedResult != EPGXEventResult::Success)
	{
		OutIssues.Add(FString::Printf(TEXT("Expected Success with allow tag, got %d"), static_cast<int32>(AllowedResult)));
	}

	Sub->UnregisterHandler(TestTag);
	return OutIssues.Num() == 0;
}

bool UPGXEventHandlerTestUtility::SequenceTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Reset();

	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub))
	{
		OutIssues.Add(TEXT("EventHandler subsystem not available"));
		return false;
	}

	const FGameplayTag Tag1 = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Seq1"));
	const FGameplayTag Tag2 = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Seq2"));
	const FGameplayTag Tag3 = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Seq3"));

	Sub->RegisterHandler(Tag1, UPGXTestSuccessHandler::StaticClass(), EPGXHandlerLifecycle::Ephemeral);
	Sub->RegisterHandler(Tag2, UPGXTestSuccessHandler::StaticClass(), EPGXHandlerLifecycle::Ephemeral);
	Sub->RegisterHandler(Tag3, UPGXTestFailHandler::StaticClass(), EPGXHandlerLifecycle::Ephemeral);

	TArray<FGameplayTag> SuccessSeq = { Tag1, Tag2 };
	TArray<FGameplayTag> FailSeq = { Tag1, Tag3, Tag2 };
	FPGXEventContext Ctx = MakeTestContext();

	// EN: All-success sequence (empty payloads array = no per-event payload)
	EPGXEventResult SeqResult = Sub->ExecuteSequence(SuccessSeq, Ctx, TArray<FInstancedStruct>(), true);
	if (SeqResult != EPGXEventResult::Success)
	{
		OutIssues.Add(TEXT("Success sequence should return Success"));
	}

	// EN: Sequence with failure and bStopOnFailure=true
	EPGXEventResult FailResult = Sub->ExecuteSequence(FailSeq, Ctx, TArray<FInstancedStruct>(), true);
	if (FailResult != EPGXEventResult::Failed)
	{
		OutIssues.Add(FString::Printf(TEXT("Fail sequence should return Failed, got %d"), static_cast<int32>(FailResult)));
	}

	Sub->UnregisterHandler(Tag1);
	Sub->UnregisterHandler(Tag2);
	Sub->UnregisterHandler(Tag3);

	return OutIssues.Num() == 0;
}

bool UPGXEventHandlerTestUtility::CacheTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Reset();

	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub))
	{
		OutIssues.Add(TEXT("EventHandler subsystem not available"));
		return false;
	}

	const FGameplayTag CachedTag = FGameplayTag::RequestGameplayTag(FName("PGX.Test.EventHandler.Cache"));
	Sub->RegisterHandler(CachedTag, UPGXTestSuccessHandler::StaticClass(), EPGXHandlerLifecycle::Cached);

	FInstancedStruct EmptyPayload;
	FPGXEventContext Ctx = MakeTestContext();

	// EN: First execution — cache miss
	FPGXHandlerCacheStats StatsBefore = Sub->GetCacheStats();
	Sub->ResolveAndExecuteWithContext(CachedTag, Ctx, EmptyPayload);
	FPGXHandlerCacheStats StatsAfter = Sub->GetCacheStats();

	if (StatsAfter.CacheMisses <= StatsBefore.CacheMisses)
	{
		OutIssues.Add(TEXT("Expected cache miss on first execution"));
	}

	// EN: Second execution — cache hit
	FPGXHandlerCacheStats StatsBeforeHit = Sub->GetCacheStats();
	Sub->ResolveAndExecuteWithContext(CachedTag, Ctx, EmptyPayload);
	FPGXHandlerCacheStats StatsAfterHit = Sub->GetCacheStats();

	if (StatsAfterHit.CacheHits <= StatsBeforeHit.CacheHits)
	{
		OutIssues.Add(TEXT("Expected cache hit on second execution"));
	}

	// EN: Evict and verify
	Sub->EvictHandler(CachedTag);
	FPGXHandlerCacheStats StatsEvict = Sub->GetCacheStats();
	if (StatsEvict.Evictions <= StatsAfter.Evictions)
	{
		OutIssues.Add(TEXT("Expected eviction count to increase"));
	}

	Sub->UnregisterHandler(CachedTag);
	return OutIssues.Num() == 0;
}


bool UPGXEventHandlerTestUtility::P0ContractTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Reset();

	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub))
	{
		OutIssues.Add(TEXT("EventHandler subsystem not available"));
		return false;
	}

	const FGameplayTag DisabledTag = MakeEventHandlerTestTag(TEXT("P0.Disabled"));
	const FGameplayTag PayloadTag = MakeEventHandlerTestTag(TEXT("P0.PayloadMismatch"));
	const FGameplayTag MissingTag = MakeEventHandlerTestTag(TEXT("P0.Missing"));

	UDataTable* Table = NewObject<UDataTable>(GetTransientPackage(), NAME_None, RF_Transient);
	Table->RowStruct = FPGXEventHandlerRow::StaticStruct();

	FPGXEventHandlerRow DisabledRow;
	DisabledRow.EventTag = DisabledTag;
	DisabledRow.HandlerClass = UPGXTestSuccessHandler::StaticClass();
	DisabledRow.Lifecycle = EPGXHandlerLifecycle::Ephemeral;
	DisabledRow.bEnabled = false;
	Table->AddRow(FName("Disabled"), DisabledRow);

	FPGXEventHandlerRow PayloadRow;
	PayloadRow.EventTag = PayloadTag;
	PayloadRow.HandlerClass = UPGXTestSuccessHandler::StaticClass();
	PayloadRow.Lifecycle = EPGXHandlerLifecycle::Ephemeral;
	PayloadRow.ExpectedPayloadType = TEXT("PGX.Expected.Payload.Struct.That.Does.Not.Match");
	PayloadRow.bEnabled = true;
	Table->AddRow(FName("PayloadMismatch"), PayloadRow);

	Sub->RegisterHandlerTable(Table);

#if WITH_EDITOR
	UPGXEventHandlerConfig* TestConfig = NewObject<UPGXEventHandlerConfig>(GetTransientPackage(), NAME_None, RF_Transient);
	TestConfig->BlackboxBufferSize = 32;
	TestConfig->bRedactBlackboxObjectNames = true;
	TestConfig->bUseObjectPathInBlackbox = true;
	Sub->InjectTestConfig(TestConfig);
#endif

	FInstancedStruct EmptyPayload;
	FPGXEventContext Context = MakeTestContext();
	UObject* SensitiveObject = NewObject<UObject>(GetTransientPackage(), FName("PGXSecretActor"), RF_Transient);
	Context.Instigator = SensitiveObject;
	Context.Target = SensitiveObject;

	if (Sub->ResolveAndExecuteWithContext(DisabledTag, Context, EmptyPayload) != EPGXEventResult::Skipped)
	{
		OutIssues.Add(TEXT("Disabled handler did not return Skipped"));
	}

	if (Sub->ResolveAndExecuteWithContext(PayloadTag, Context, EmptyPayload) != EPGXEventResult::Failed)
	{
		OutIssues.Add(TEXT("Payload mismatch did not return Failed"));
	}

	if (Sub->ResolveAndExecuteWithContext(MissingTag, Context, EmptyPayload) != EPGXEventResult::NotFound)
	{
		OutIssues.Add(TEXT("Missing handler did not return NotFound"));
	}

	FPGXBlackboxEntry DisabledEntry;
	if (!LatestEntryForTag(Sub, DisabledTag, DisabledEntry)
		|| !DisabledEntry.FailureReason.StartsWith(TEXT("PGX.EventHandler.Validation.HandlerDisabled"))
		|| !DisabledEntry.FailureReasonTag.IsValid())
	{
		OutIssues.Add(TEXT("Disabled blackbox entry missing canonical validation reason"));
	}
#if WITH_EDITOR
	else if (DisabledEntry.InstigatorName != TEXT("REDACTED") || DisabledEntry.TargetName != TEXT("REDACTED"))
	{
		OutIssues.Add(TEXT("Blackbox object names were not redacted under test config"));
	}
#endif

	FPGXBlackboxEntry PayloadEntry;
	if (!LatestEntryForTag(Sub, PayloadTag, PayloadEntry)
		|| !PayloadEntry.FailureReason.StartsWith(TEXT("PGX.EventHandler.Validation.PayloadMismatch"))
		|| !PayloadEntry.FailureReasonTag.IsValid())
	{
		OutIssues.Add(TEXT("Payload mismatch blackbox entry missing canonical validation reason"));
	}

	FPGXBlackboxEntry MissingEntry;
	if (!LatestEntryForTag(Sub, MissingTag, MissingEntry)
		|| !MissingEntry.FailureReason.StartsWith(TEXT("PGX.EventHandler.Result.HandlerNotFound"))
		|| !MissingEntry.FailureReasonTag.IsValid())
	{
		OutIssues.Add(TEXT("Missing-handler blackbox entry missing canonical result reason"));
	}

#if WITH_EDITOR
	Sub->ClearTestConfigs();
#endif
	Sub->UnregisterHandlerTable(Table);
	Sub->UnregisterHandler(DisabledTag);
	Sub->UnregisterHandler(PayloadTag);

	return OutIssues.Num() == 0;
}

bool UPGXEventHandlerTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Reset();
	bool bAllPassed = true;
	TArray<FString> SubIssues;

	auto RunOne = [&](const TCHAR* Name, bool(*Fn)(const UObject*, TArray<FString>&))
	{
		SubIssues.Reset();
		if (!Fn(WorldContextObject, SubIssues))
		{
			bAllPassed = false;
			for (const FString& Issue : SubIssues)
			{
				OutIssues.Add(FString::Printf(TEXT("[%s] %s"), Name, *Issue));
			}
		}
	};

	RunOne(TEXT("Quick"), &QuickTest);
	RunOne(TEXT("Lifecycle"), &LifecycleTest);
	RunOne(TEXT("Payload"), &PayloadTest);
	RunOne(TEXT("Condition"), &ConditionTest);
	RunOne(TEXT("Sequence"), &SequenceTest);
	RunOne(TEXT("Cache"), &CacheTest);
	RunOne(TEXT("P0Contracts"), &P0ContractTest);

	return bAllPassed;
}
