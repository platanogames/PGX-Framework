// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "EventHandler/PGXEventHandlerConfig.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "EventHandler/Tags/PGXEventHandlerTags.h"
#include "../PGXEventHandlerTestHelpers.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "StructUtils/InstancedStruct.h"
#include "Misc/AutomationTest.h"

namespace PGXEventHandlerAutomation
{
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

	UPGXEventHandlerSubsystem* FindSubsystem(FAutomationTestBase& Test)
	{
		UGameInstance* GameInstance = FindGameInstance();
		if (!GameInstance)
		{
			Test.AddError(TEXT("PGXEventHandler automation setup failed: no GameInstance available. Run as a game/client automation test."));
			return nullptr;
		}

		UPGXEventHandlerSubsystem* EventHandler = GameInstance->GetSubsystem<UPGXEventHandlerSubsystem>();
		if (!EventHandler)
		{
			Test.AddError(TEXT("PGXEventHandler automation setup failed: UPGXEventHandlerSubsystem missing."));
		}
		return EventHandler;
	}

	UPGXEventHandlerConfig* MakeConfig(const TCHAR* Name, int32 MaxExecutionDepth = 8, bool bRedactBlackboxObjectNames = false)
	{
		UPGXEventHandlerConfig* Config = NewObject<UPGXEventHandlerConfig>(
			GetTransientPackage(),
			UPGXEventHandlerConfig::StaticClass(),
			FName(Name),
			RF_Transient);
		Config->MaxExecutionDepth = MaxExecutionDepth;
		Config->BlackboxBufferSize = 64;
		Config->bRedactBlackboxObjectNames = bRedactBlackboxObjectNames;
		Config->bUseObjectPathInBlackbox = true;
		return Config;
	}

	UDataTable* MakeHandlerTable(const TCHAR* Name, const TArray<FPGXEventHandlerRow>& Rows)
	{
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage(), FName(Name), RF_Transient);
		Table->RowStruct = FPGXEventHandlerRow::StaticStruct();

		for (int32 Index = 0; Index < Rows.Num(); ++Index)
		{
			Table->AddRow(FName(*FString::Printf(TEXT("Row_%d"), Index)), Rows[Index]);
		}
		return Table;
	}

	FPGXEventHandlerRow MakeRow(FGameplayTag EventTag, TSubclassOf<UPGXEventHandlerBase> HandlerClass, bool bEnabled = true, const FString& ExpectedPayloadType = FString())
	{
		FPGXEventHandlerRow Row;
		Row.EventTag = EventTag;
		Row.HandlerClass = HandlerClass;
		Row.Lifecycle = EPGXHandlerLifecycle::Ephemeral;
		Row.CategoryTag = TAG_PGX_EventHandler_Category_Core.GetTag();
		Row.bEnabled = bEnabled;
		Row.ExpectedPayloadType = ExpectedPayloadType;
		Row.Description = TEXT("PGX EventHandler automation row");
		return Row;
	}

	const FPGXBlackboxEntry* FindLastEntry(const UPGXEventHandlerSubsystem* EventHandler, FGameplayTag EventTag)
	{
		const TArray<FPGXBlackboxEntry>& Entries = EventHandler->GetBlackboxEntries();
		for (int32 Index = Entries.Num() - 1; Index >= 0; --Index)
		{
			if (Entries[Index].EventTag == EventTag)
			{
				return &Entries[Index];
			}
		}
		return nullptr;
	}

	void CleanupHandlerTable(UPGXEventHandlerSubsystem* EventHandler, UDataTable* Table)
	{
		if (EventHandler && Table)
		{
			EventHandler->UnregisterHandlerTable(Table);
		}
#if WITH_EDITOR
		if (EventHandler)
		{
			EventHandler->ClearTestConfigs();
		}
#endif
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXEventHandler_P0ContractAutomationTest,
	"PGX.Core.EventHandler.P0Contract",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXEventHandler_P0ContractAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UPGXEventHandlerSubsystem* EventHandler = PGXEventHandlerAutomation::FindSubsystem(*this);
	if (!EventHandler)
	{
		return true;
	}

	const FGameplayTag DisabledTag = TAG_PGX_Event_Item.GetTag();
	const FGameplayTag PayloadMismatchTag = TAG_PGX_Event_Interact.GetTag();
	const FGameplayTag MissingTag = TAG_PGX_Event_Flow.GetTag();

	UDataTable* Table = PGXEventHandlerAutomation::MakeHandlerTable(TEXT("PGXEventHandler_P0ContractAutomationTable"),
		{
			PGXEventHandlerAutomation::MakeRow(DisabledTag, UPGXTestSuccessHandler::StaticClass(), false),
			PGXEventHandlerAutomation::MakeRow(PayloadMismatchTag, UPGXTestSuccessHandler::StaticClass(), true, TEXT("PGX.Expected.Payload.Struct.That.Does.Not.Match"))
		});

#if WITH_EDITOR
	EventHandler->InjectTestConfig(PGXEventHandlerAutomation::MakeConfig(TEXT("PGXEventHandler_P0ContractAutomationConfig"), 8, true));
#endif
	EventHandler->RegisterHandlerTable(Table);

	UPGXTestSuccessHandler* SensitiveObject = NewObject<UPGXTestSuccessHandler>(
		GetTransientPackage(),
		UPGXTestSuccessHandler::StaticClass(),
		FName(TEXT("PGXSecretActor_Automation")),
		RF_Transient);
	FPGXEventContext Context;
	Context.Instigator = SensitiveObject;
	Context.Target = SensitiveObject;
	const FInstancedStruct EmptyPayload;

	TestEqual(TEXT("Disabled handler is skipped"), EventHandler->ResolveAndExecuteWithContext(DisabledTag, Context, EmptyPayload), EPGXEventResult::Skipped);
	AddExpectedError(TEXT("ResolveAndExecute: PayloadMismatch for PGX.Event.Interact"), EAutomationExpectedErrorFlags::Contains, 1, false);
	TestEqual(TEXT("Payload mismatch is rejected"), EventHandler->ResolveAndExecuteWithContext(PayloadMismatchTag, Context, EmptyPayload), EPGXEventResult::Failed);
	TestEqual(TEXT("Missing handler reports NotFound"), EventHandler->ResolveAndExecuteWithContext(MissingTag, Context, EmptyPayload), EPGXEventResult::NotFound);

	const FPGXBlackboxEntry* DisabledEntry = PGXEventHandlerAutomation::FindLastEntry(EventHandler, DisabledTag);
	TestNotNull(TEXT("Disabled blackbox entry recorded"), DisabledEntry);
	if (DisabledEntry)
	{
		TestTrue(TEXT("Disabled reason is canonical"), DisabledEntry->FailureReason.StartsWith(TEXT("PGX.EventHandler.Validation.HandlerDisabled")));
		TestTrue(TEXT("Disabled reason tag is valid"), DisabledEntry->FailureReasonTag.IsValid());
#if WITH_EDITOR
		TestEqual(TEXT("Disabled instigator redacted"), DisabledEntry->InstigatorName, FString(TEXT("REDACTED")));
		TestEqual(TEXT("Disabled target redacted"), DisabledEntry->TargetName, FString(TEXT("REDACTED")));
#else
		AddInfo(TEXT("Redaction assertion requires editor-only InjectTestConfig; runtime compile path keeps harness registered without module-rule/editor dependency."));
#endif
	}

	const FPGXBlackboxEntry* PayloadEntry = PGXEventHandlerAutomation::FindLastEntry(EventHandler, PayloadMismatchTag);
	TestNotNull(TEXT("Payload mismatch blackbox entry recorded"), PayloadEntry);
	if (PayloadEntry)
	{
		TestTrue(TEXT("Payload mismatch reason is canonical"), PayloadEntry->FailureReason.StartsWith(TEXT("PGX.EventHandler.Validation.PayloadMismatch")));
		TestTrue(TEXT("Payload mismatch reason tag is valid"), PayloadEntry->FailureReasonTag.IsValid());
	}

	const FPGXBlackboxEntry* MissingEntry = PGXEventHandlerAutomation::FindLastEntry(EventHandler, MissingTag);
	TestNotNull(TEXT("Missing handler blackbox entry recorded"), MissingEntry);
	if (MissingEntry)
	{
		TestTrue(TEXT("Missing handler reason is canonical"), MissingEntry->FailureReason.StartsWith(TEXT("PGX.EventHandler.Result.HandlerNotFound")));
		TestTrue(TEXT("Missing handler reason tag is valid"), MissingEntry->FailureReasonTag.IsValid());
	}

	PGXEventHandlerAutomation::CleanupHandlerTable(EventHandler, Table);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXEventHandler_RAIIAndDepthAutomationTest,
	"PGX.Core.EventHandler.RAIIAndDepthSafety",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FPGXEventHandler_RAIIAndDepthAutomationTest::RunTest(const FString& /*Parameters*/)
{
	UPGXEventHandlerSubsystem* EventHandler = PGXEventHandlerAutomation::FindSubsystem(*this);
	if (!EventHandler)
	{
		return true;
	}

	const FGameplayTag ConditionalTag = TAG_PGX_Event_UI.GetTag();
	const FGameplayTag SuccessTag = TAG_PGX_Event_System.GetTag();

	UDataTable* Table = PGXEventHandlerAutomation::MakeHandlerTable(TEXT("PGXEventHandler_RAIIAutomationTable"),
		{
			PGXEventHandlerAutomation::MakeRow(ConditionalTag, UPGXTestConditionHandler::StaticClass()),
			PGXEventHandlerAutomation::MakeRow(SuccessTag, UPGXTestSuccessHandler::StaticClass())
		});

#if WITH_EDITOR
	EventHandler->InjectTestConfig(PGXEventHandlerAutomation::MakeConfig(TEXT("PGXEventHandler_RAIIAutomationConfig"), 8, false));
#endif
	EventHandler->RegisterHandlerTable(Table);

	const FPGXEventContext EmptyContext;
	const FInstancedStruct EmptyPayload;

	TestEqual(TEXT("CanExecute false skips without leaking stack"), EventHandler->ResolveAndExecuteWithContext(ConditionalTag, EmptyContext, EmptyPayload), EPGXEventResult::Skipped);
	TestEqual(TEXT("Second CanExecute false is still skipped, not cycle-blocked"), EventHandler->ResolveAndExecuteWithContext(ConditionalTag, EmptyContext, EmptyPayload), EPGXEventResult::Skipped);
	TestEqual(TEXT("Subsequent success proves RAII guard unwound"), EventHandler->ResolveAndExecuteWithContext(SuccessTag, EmptyContext, EmptyPayload), EPGXEventResult::Success);

#if WITH_EDITOR
	EventHandler->InjectTestConfig(PGXEventHandlerAutomation::MakeConfig(TEXT("PGXEventHandler_DepthAutomationConfig"), 0, false));
	AddExpectedError(TEXT("ResolveAndExecute: Max recursion depth (0) reached for PGX.Event.System"), EAutomationExpectedErrorFlags::Contains, 1, false);
	TestEqual(TEXT("Depth budget blocks execution when max depth is exhausted"), EventHandler->ResolveAndExecuteWithContext(SuccessTag, EmptyContext, EmptyPayload), EPGXEventResult::Blocked);

	const FPGXBlackboxEntry* DepthEntry = PGXEventHandlerAutomation::FindLastEntry(EventHandler, SuccessTag);
	TestNotNull(TEXT("Depth budget blackbox entry recorded"), DepthEntry);
	if (DepthEntry)
	{
		TestTrue(TEXT("Depth budget reason is canonical"), DepthEntry->FailureReason.StartsWith(TEXT("PGX.EventHandler.Result.ChainBudgetExceeded")));
		TestTrue(TEXT("Depth budget reason tag is valid"), DepthEntry->FailureReasonTag.IsValid());
	}
#else
	AddInfo(TEXT("Depth-budget injection requires editor-only InjectTestConfig; RAII early-return guard remains covered in runtime automation."));
#endif

	PGXEventHandlerAutomation::CleanupHandlerTable(EventHandler, Table);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
