// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXGameFlowBlueprintLibrary.h"
#include "PGXGameFlowSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

// EN: GameFlow Blueprint Library — thin wrappers resolving subsystem via GameInstance
// ES: Blueprint Library de GameFlow — wrappers delgados que resuelven subsistema via GameInstance

// ============================================================================
// Internal Helper
// ============================================================================

UPGXGameFlowSubsystem* UPGXGameFlowBlueprintLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;

	return GI->GetSubsystem<UPGXGameFlowSubsystem>();
}

// ============================================================================
// Set API
// ============================================================================

FPGXFlowResult UPGXGameFlowBlueprintLibrary::SetStateByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FGameplayTag FlowTag, UObject* Source)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->SetStateByTag(Channel, FlowTag, Source) : FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Subsystem not available"));
}

FPGXFlowResult UPGXGameFlowBlueprintLibrary::SetBatchSequentialStateByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, const TArray<FGameplayTag>& FlowTags, UObject* Source)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->SetBatchSequentialStateByTag(Channel, FlowTags, Source) : FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Subsystem not available"));
}

FPGXFlowResult UPGXGameFlowBlueprintLibrary::SetBatchStateByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, const FGameplayTagContainer& FlowTags, UObject* Source)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->SetBatchStateByTag(Channel, FlowTags, Source) : FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Subsystem not available"));
}

FPGXFlowResult UPGXGameFlowBlueprintLibrary::RevertToPreviousFlow(const UObject* WorldContextObject, EPGXFlowChannel Channel, UObject* Source)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->RevertToPreviousFlow(Channel, Source) : FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Subsystem not available"));
}

// ============================================================================
// Validation API
// ============================================================================

FPGXFlowResult UPGXGameFlowBlueprintLibrary::CanChangeByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FGameplayTag FlowTag)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->CanChangeByTag(Channel, FlowTag) : FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Subsystem not available"));
}

FPGXFlowResult UPGXGameFlowBlueprintLibrary::CanBatchChangeByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, const TArray<FGameplayTag>& FlowTags)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->CanBatchChangeByTag(Channel, FlowTags) : FPGXFlowResult::MakeFail(EPGXFlowResultCode::InvalidContext, TEXT("Subsystem not available"));
}

bool UPGXGameFlowBlueprintLibrary::IsCurrentFlowTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FGameplayTag FlowTag)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->IsCurrentFlowTag(Channel, FlowTag) : false;
}

bool UPGXGameFlowBlueprintLibrary::CheckCanRevert(const UObject* WorldContextObject, EPGXFlowChannel Channel)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->CheckCanRevert(Channel) : false;
}

// ============================================================================
// Query API
// ============================================================================

FGameplayTag UPGXGameFlowBlueprintLibrary::GetCurrentFlowTag(const UObject* WorldContextObject, EPGXFlowChannel Channel)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetCurrentFlowTag(Channel) : FGameplayTag();
}

FGameplayTag UPGXGameFlowBlueprintLibrary::GetLastFlowTag(const UObject* WorldContextObject, EPGXFlowChannel Channel)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetLastFlowTag(Channel) : FGameplayTag();
}

TArray<FPGXFlowHistoryEntry> UPGXGameFlowBlueprintLibrary::GetChannelHistory(const UObject* WorldContextObject, EPGXFlowChannel Channel)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetChannelHistory(Channel) : TArray<FPGXFlowHistoryEntry>();
}

bool UPGXGameFlowBlueprintLibrary::GetAllowedTransitionByTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FGameplayTag FlowTag, FPGXFlowRule& OutRule)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetAllowedTransitionByTag(Channel, FlowTag, OutRule) : false;
}

bool UPGXGameFlowBlueprintLibrary::GetAllowedTransitionByCurrentFlowTag(const UObject* WorldContextObject, EPGXFlowChannel Channel, FPGXFlowRule& OutRule)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetAllowedTransitionByCurrentFlowTag(Channel, OutRule) : false;
}

// ============================================================================
// Utility
// ============================================================================

FString UPGXGameFlowBlueprintLibrary::GetChannelName(EPGXFlowChannel Channel)
{
	return UPGXGameFlowSubsystem::GetChannelName(Channel);
}

bool UPGXGameFlowBlueprintLibrary::IsGameFlowInitialized(const UObject* WorldContextObject)
{
	UPGXGameFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->IsInitialized() : false;
}
