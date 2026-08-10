// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/PGXEventHandlerBlueprintLibrary.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerBase.h"
#include "Engine/DataTable.h"

// ============================================================
// Core — CustomThunk (morphing Payload pin)
// ============================================================

EPGXEventResult UPGXEventHandlerBlueprintLibrary::K2_ExecuteEvent(const UObject* /*WorldContextObject*/,
	FGameplayTag /*EventTag*/, UObject* /*Instigator*/, const int32& /*Payload*/)
{
	// EN: Never called directly — exec thunk below handles Blueprint calls
	// ES: Nunca se llama directamente — el thunk exec maneja llamadas Blueprint
	checkNoEntry();
	return EPGXEventResult::Failed;
}

DEFINE_FUNCTION(UPGXEventHandlerBlueprintLibrary::execK2_ExecuteEvent)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_STRUCT(FGameplayTag, EventTag);
	P_GET_OBJECT(UObject, Instigator);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* PayloadPtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	EPGXEventResult Result = EPGXEventResult::Failed;
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (IsValid(Sub))
	{
		FInstancedStruct Payload;
		if (StructProp && StructProp->Struct && PayloadPtr)
		{
			Payload.InitializeAs(StructProp->Struct, static_cast<const uint8*>(PayloadPtr));
		}
		Result = Sub->ResolveAndExecute(EventTag, Instigator, Payload);
	}
	*(EPGXEventResult*)RESULT_PARAM = Result;
}

EPGXEventResult UPGXEventHandlerBlueprintLibrary::K2_ExecuteEventAdvanced(const UObject* /*WorldContextObject*/,
	FGameplayTag /*EventTag*/, const FPGXEventContext& /*Context*/, const int32& /*Payload*/)
{
	checkNoEntry();
	return EPGXEventResult::Failed;
}

DEFINE_FUNCTION(UPGXEventHandlerBlueprintLibrary::execK2_ExecuteEventAdvanced)
{
	P_GET_OBJECT(UObject, WorldContextObject);
	P_GET_STRUCT(FGameplayTag, EventTag);
	P_GET_STRUCT(FPGXEventContext, EvtContext);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* PayloadPtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	EPGXEventResult Result = EPGXEventResult::Failed;
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (IsValid(Sub))
	{
		FInstancedStruct Payload;
		if (StructProp && StructProp->Struct && PayloadPtr)
		{
			Payload.InitializeAs(StructProp->Struct, static_cast<const uint8*>(PayloadPtr));
		}
		Result = Sub->ResolveAndExecuteWithContext(EventTag, EvtContext, Payload);
	}
	*(EPGXEventResult*)RESULT_PARAM = Result;
}

// ============================================================
// Core — Non-thunk
// ============================================================

EPGXEventResult UPGXEventHandlerBlueprintLibrary::QuickExecute(const UObject* WorldContextObject, FGameplayTag EventTag)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return EPGXEventResult::NotFound; }
	return Sub->ResolveAndExecute(EventTag, nullptr, FInstancedStruct());
}

// ============================================================
	// C++ only (test utilities and native integration callers)
// ============================================================

EPGXEventResult UPGXEventHandlerBlueprintLibrary::ExecuteEvent(const UObject* WorldContextObject,
	FGameplayTag EventTag, UObject* Instigator, const FInstancedStruct& Payload)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return EPGXEventResult::NotFound; }
	return Sub->ResolveAndExecute(EventTag, Instigator, Payload);
}

EPGXEventResult UPGXEventHandlerBlueprintLibrary::ExecuteEventAdvanced(const UObject* WorldContextObject,
	FGameplayTag EventTag, const FPGXEventContext& Context, const FInstancedStruct& Payload)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return EPGXEventResult::NotFound; }
	return Sub->ResolveAndExecuteWithContext(EventTag, Context, Payload);
}

// ============================================================
// Advanced
// ============================================================

void UPGXEventHandlerBlueprintLibrary::RegisterHandlerTable(const UObject* WorldContextObject, UDataTable* InTable)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (IsValid(Sub)) { Sub->RegisterHandlerTable(InTable); }
}

void UPGXEventHandlerBlueprintLibrary::UnregisterHandlerTable(const UObject* WorldContextObject, UDataTable* InTable)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (IsValid(Sub)) { Sub->UnregisterHandlerTable(InTable); }
}

void UPGXEventHandlerBlueprintLibrary::RegisterHandler(const UObject* WorldContextObject, FGameplayTag EventTag,
	TSubclassOf<UPGXEventHandlerBase> HandlerClass, EPGXHandlerLifecycle Lifecycle, FGameplayTag CategoryTag)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (IsValid(Sub)) { Sub->RegisterHandler(EventTag, HandlerClass, Lifecycle, CategoryTag); }
}

void UPGXEventHandlerBlueprintLibrary::UnregisterHandler(const UObject* WorldContextObject, FGameplayTag EventTag)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (IsValid(Sub)) { Sub->UnregisterHandler(EventTag); }
}

void UPGXEventHandlerBlueprintLibrary::InvalidateHandlerCache(const UObject* WorldContextObject)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (IsValid(Sub)) { Sub->InvalidateCache(); }
}

void UPGXEventHandlerBlueprintLibrary::EvictHandler(const UObject* WorldContextObject, FGameplayTag EventTag)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (IsValid(Sub)) { Sub->EvictHandler(EventTag); }
}

EPGXEventResult UPGXEventHandlerBlueprintLibrary::ExecuteEventSequence(const UObject* WorldContextObject,
	const TArray<FGameplayTag>& EventTags, const FPGXEventContext& Context, bool bStopOnFailure)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return EPGXEventResult::NotFound; }
	return Sub->ExecuteSequence(EventTags, Context, TArray<FInstancedStruct>(), bStopOnFailure);
}

bool UPGXEventHandlerBlueprintLibrary::ValidateEventConditions(const UObject* WorldContextObject,
	const TArray<FGameplayTag>& EventTags, const FPGXEventContext& Context)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return false; }
	return Sub->ValidateConditions(EventTags, Context);
}

// ============================================================
// Query
// ============================================================

bool UPGXEventHandlerBlueprintLibrary::CanExecuteEvent(const UObject* WorldContextObject,
	FGameplayTag EventTag, const FPGXEventContext& Context)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return false; }
	TArray<FGameplayTag> Tags;
	Tags.Add(EventTag);
	return Sub->ValidateConditions(Tags, Context);
}

bool UPGXEventHandlerBlueprintLibrary::IsEventRegistered(const UObject* WorldContextObject, FGameplayTag EventTag)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return false; }
	return Sub->IsHandlerRegistered(EventTag);
}

FPGXEventHandlerInfo UPGXEventHandlerBlueprintLibrary::GetHandlerInfo(const UObject* WorldContextObject, FGameplayTag EventTag)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return FPGXEventHandlerInfo(); }
	return Sub->GetHandlerInfo(EventTag);
}

TArray<FPGXEventHandlerInfo> UPGXEventHandlerBlueprintLibrary::GetHandlersByCategory(const UObject* WorldContextObject, FGameplayTag CategoryTag)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return TArray<FPGXEventHandlerInfo>(); }
	return Sub->GetHandlersByCategory(CategoryTag);
}

TArray<FGameplayTag> UPGXEventHandlerBlueprintLibrary::GetAllRegisteredTags(const UObject* WorldContextObject)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return TArray<FGameplayTag>(); }
	return Sub->GetAllRegisteredTags();
}

TArray<FGameplayTag> UPGXEventHandlerBlueprintLibrary::GetAllCategories(const UObject* WorldContextObject)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return TArray<FGameplayTag>(); }
	return Sub->GetAllCategories();
}

FPGXHandlerCacheStats UPGXEventHandlerBlueprintLibrary::GetHandlerCacheStats(const UObject* WorldContextObject)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return FPGXHandlerCacheStats(); }
	return Sub->GetCacheStats();
}

FPGXEventContext UPGXEventHandlerBlueprintLibrary::MakeEventContext(UObject* Instigator, UObject* Target)
{
	FPGXEventContext Context;
	Context.Instigator = Instigator;
	Context.Target = Target;
	return Context;
}

bool UPGXEventHandlerBlueprintLibrary::WasSuccessful(EPGXEventResult Result)
{
	return Result == EPGXEventResult::Success || Result == EPGXEventResult::Partial;
}

FString UPGXEventHandlerBlueprintLibrary::GetResultString(EPGXEventResult Result)
{
	switch (Result)
	{
	case EPGXEventResult::Success:  return TEXT("Success");
	case EPGXEventResult::Partial:  return TEXT("Partial");
	case EPGXEventResult::Failed:   return TEXT("Failed");
	case EPGXEventResult::Skipped:  return TEXT("Skipped");
	case EPGXEventResult::NotFound: return TEXT("NotFound");
	case EPGXEventResult::Blocked:  return TEXT("Blocked");
	default:                        return TEXT("Unknown");
	}
}

// ============================================================
// Debug
// ============================================================

FPGXHandlerTelemetry UPGXEventHandlerBlueprintLibrary::GetHandlerTelemetry(const UObject* WorldContextObject, FGameplayTag EventTag)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return FPGXHandlerTelemetry(); }
	return Sub->GetHandlerTelemetry(EventTag);
}

TArray<FPGXHandlerTelemetry> UPGXEventHandlerBlueprintLibrary::GetAllHandlerTelemetry(const UObject* WorldContextObject)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return TArray<FPGXHandlerTelemetry>(); }
	return Sub->GetAllTelemetry();
}

void UPGXEventHandlerBlueprintLibrary::ResetHandlerTelemetry(const UObject* WorldContextObject)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (IsValid(Sub)) { Sub->ResetTelemetry(); }
}

TArray<FPGXBlackboxEntry> UPGXEventHandlerBlueprintLibrary::GetBlackboxEntries(const UObject* WorldContextObject)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return TArray<FPGXBlackboxEntry>(); }
	// EN: Return by value (copy) for Blueprint safety — subsystem returns const ref
	// ES: Retornar por valor (copia) para seguridad Blueprint — subsistema retorna const ref
	return Sub->GetBlackboxEntries();
}

FString UPGXEventHandlerBlueprintLibrary::DumpBlackboxToString(const UObject* WorldContextObject)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return FString(); }
	return Sub->DumpBlackboxToString();
}

FString UPGXEventHandlerBlueprintLibrary::ExportReport(const UObject* WorldContextObject)
{
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(WorldContextObject);
	if (!IsValid(Sub)) { return FString(); }
	return Sub->ExportReport();
}
