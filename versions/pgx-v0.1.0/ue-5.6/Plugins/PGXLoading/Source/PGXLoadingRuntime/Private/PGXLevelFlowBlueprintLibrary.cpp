// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLevelFlowBlueprintLibrary.h"
#include "PGXLevelFlowSubsystem.h"
#include "PGXLevelFlowActor.h"
#include "ShaderPipelineCache.h"
#include "Engine/GameInstance.h"

// ============================================================================
// Helper
// ============================================================================

UPGXLevelFlowSubsystem* UPGXLevelFlowBlueprintLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXLevelFlowSubsystem>() : nullptr;
}

// ============================================================================
// Transition Control
// ============================================================================

FPGXLevelFlowResult UPGXLevelFlowBlueprintLibrary::RequestLevelTransition(const UObject* WorldContextObject, FGameplayTag LevelTag, UObject* Source)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed, TEXT("LevelFlow subsystem not available"));
	}
	return Sub->RequestLevelTransition(LevelTag, Source);
}

FPGXLevelFlowResult UPGXLevelFlowBlueprintLibrary::CancelLevelTransition(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed, TEXT("LevelFlow subsystem not available"));
	}
	return Sub->CancelTransition();
}

// ============================================================================
// Query
// ============================================================================

EPGXLevelFlowState UPGXLevelFlowBlueprintLibrary::GetTransitionState(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetTransitionState() : EPGXLevelFlowState::Idle;
}

FGameplayTag UPGXLevelFlowBlueprintLibrary::GetCurrentLevelTag(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetCurrentLevelTag() : FGameplayTag();
}

FGameplayTag UPGXLevelFlowBlueprintLibrary::GetPreviousLevelTag(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetPreviousLevelTag() : FGameplayTag();
}

bool UPGXLevelFlowBlueprintLibrary::IsLevelTransitionActive(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->IsTransitionActive() : false;
}

float UPGXLevelFlowBlueprintLibrary::GetLevelTransitionProgress(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetTransitionProgress() : 0.0f;
}

bool UPGXLevelFlowBlueprintLibrary::ResolveLevelByTag(const UObject* WorldContextObject, FGameplayTag LevelTag, FPGXLevelEntry& OutEntry)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->ResolveLevelByTag(LevelTag, OutEntry) : false;
}

TArray<FGameplayTag> UPGXLevelFlowBlueprintLibrary::GetRegisteredLevelTags(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetRegisteredLevelTags() : TArray<FGameplayTag>();
}

TArray<FPGXLevelTransitionRecord> UPGXLevelFlowBlueprintLibrary::GetTransitionHistory(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetTransitionHistory() : TArray<FPGXLevelTransitionRecord>();
}

// ============================================================================
// Sub-Level Management
// ============================================================================

FPGXLevelFlowResult UPGXLevelFlowBlueprintLibrary::RequestSubLevelLoad(const UObject* WorldContextObject, FGameplayTag SubLevelTag)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed, TEXT("LevelFlow subsystem not available"));
	}
	return Sub->RequestSubLevelLoad(SubLevelTag);
}

FPGXLevelFlowResult UPGXLevelFlowBlueprintLibrary::RequestSubLevelUnload(const UObject* WorldContextObject, FGameplayTag SubLevelTag)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		return FPGXLevelFlowResult::MakeFail(EPGXLevelFlowResultCode::LoadFailed, TEXT("LevelFlow subsystem not available"));
	}
	return Sub->RequestSubLevelUnload(SubLevelTag);
}

bool UPGXLevelFlowBlueprintLibrary::IsSubLevelLoaded(const UObject* WorldContextObject, FGameplayTag SubLevelTag)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->IsSubLevelLoaded(SubLevelTag) : false;
}

TArray<FGameplayTag> UPGXLevelFlowBlueprintLibrary::GetLoadedSubLevels(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetLoadedSubLevels() : TArray<FGameplayTag>();
}

// ============================================================================
// Actor & Info
// ============================================================================

APGXLevelFlowActor* UPGXLevelFlowBlueprintLibrary::GetCurrentLevelFlowActor(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetCurrentLevelFlowActor() : nullptr;
}

int32 UPGXLevelFlowBlueprintLibrary::GetRegisteredLevelCount(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetRegisteredLevelCount() : 0;
}

int32 UPGXLevelFlowBlueprintLibrary::GetDiscoveredProfileCount(const UObject* WorldContextObject)
{
	UPGXLevelFlowSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetDiscoveredProfileCount() : 0;
}

int32 UPGXLevelFlowBlueprintLibrary::GetPendingShaderCompilations()
{
	return static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
}
