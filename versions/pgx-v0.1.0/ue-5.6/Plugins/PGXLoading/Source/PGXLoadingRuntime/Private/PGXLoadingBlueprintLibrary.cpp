// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLoadingBlueprintLibrary.h"
#include "PGXLoadingSubsystem.h"
#include "Engine/GameInstance.h"

// ============================================================================
// Helper
// ============================================================================

UPGXLoadingSubsystem* UPGXLoadingBlueprintLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXLoadingSubsystem>() : nullptr;
}

// ============================================================================
// Loading Control
// ============================================================================

FPGXLoadingResult UPGXLoadingBlueprintLibrary::RequestLoading(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::AssetLoadFailed,
			TEXT("Loading subsystem not available"));
	}
	return Sub->RequestLoading(ContextTag);
}

FPGXLoadingResult UPGXLoadingBlueprintLibrary::ForceCloseLoading(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::AssetLoadFailed,
			TEXT("Loading subsystem not available"));
	}
	return Sub->ForceClose();
}

FPGXLoadingResult UPGXLoadingBlueprintLibrary::RequestSkip(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		return FPGXLoadingResult::MakeFail(EPGXLoadingResultCode::AssetLoadFailed,
			TEXT("Loading subsystem not available"));
	}
	return Sub->RequestSkip();
}

// ============================================================================
// Query
// ============================================================================

bool UPGXLoadingBlueprintLibrary::IsLoadingActive(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->IsLoadingActive() : false;
}

EPGXLoadingScreenState UPGXLoadingBlueprintLibrary::GetLoadingScreenState(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetCurrentState() : EPGXLoadingScreenState::Idle;
}

FGameplayTag UPGXLoadingBlueprintLibrary::GetCurrentLoadingContext(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetCurrentContext() : FGameplayTag();
}

FPGXLoadingProgress UPGXLoadingBlueprintLibrary::GetLoadingProgress(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetProgress() : FPGXLoadingProgress();
}

float UPGXLoadingBlueprintLibrary::GetLoadingElapsedTime(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetElapsedTime() : 0.0f;
}

EPGXLoadingVisualType UPGXLoadingBlueprintLibrary::GetActiveVisualType(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetActiveVisualType() : EPGXLoadingVisualType::Minimal;
}

// ============================================================================
// Profiles
// ============================================================================

bool UPGXLoadingBlueprintLibrary::IsLoadingProfileValid(const UObject* WorldContextObject, FGameplayTag ContextTag)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->IsProfileValid(ContextTag) : false;
}

int32 UPGXLoadingBlueprintLibrary::GetDiscoveredProfileCount(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetDiscoveredProfileCount() : 0;
}

TArray<FGameplayTag> UPGXLoadingBlueprintLibrary::GetRegisteredContextTags(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetRegisteredContextTags() : TArray<FGameplayTag>();
}

// ============================================================================
// History & Info
// ============================================================================

TArray<FPGXLoadingRecord> UPGXLoadingBlueprintLibrary::GetLoadingHistory(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	return Sub ? Sub->GetLoadingHistory() : TArray<FPGXLoadingRecord>();
}

float UPGXLoadingBlueprintLibrary::GetLastLoadingDuration(const UObject* WorldContextObject)
{
	UPGXLoadingSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub) return 0.0f;

	const TArray<FPGXLoadingRecord> History = Sub->GetLoadingHistory();
	if (History.Num() == 0) return 0.0f;

	return History.Last().TotalDuration;
}
