// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Profile/PGXProfileBlueprintLibrary.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

// ============================================================================
// Subsystem Resolution
// ============================================================================

UPGXProfileSubsystem* UPGXProfileBlueprintLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;

	return GI->GetSubsystem<UPGXProfileSubsystem>();
}

// ============================================================================
// Profile State
// ============================================================================

FPGXResolvedProfile UPGXProfileBlueprintLibrary::GetProjectProfile(const UObject* WorldContextObject)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->GetResolvedProfile();
	}
	return FPGXResolvedProfile::MakeDefaultPermissive();
}

bool UPGXProfileBlueprintLibrary::IsProfileResolved(const UObject* WorldContextObject)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->IsProfileResolved();
	}
	return false;
}

EPGXProfileState UPGXProfileBlueprintLibrary::GetProfileState(const UObject* WorldContextObject)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->GetProfileState();
	}
	return EPGXProfileState::Unresolved;
}

// ============================================================================
// Capability Queries
// ============================================================================

bool UPGXProfileBlueprintLibrary::IsCapabilityEnabled(const UObject* WorldContextObject, FName CapabilityName)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->IsCapabilityEnabled(CapabilityName);
	}
	return true;
}

// ============================================================================
// Feature Queries
// ============================================================================

bool UPGXProfileBlueprintLibrary::IsFeatureAllowed(const UObject* WorldContextObject, FName FeatureName)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->IsFeatureAllowed(FeatureName);
	}
	return true;
}

// ============================================================================
// Budget Queries
// ============================================================================

int64 UPGXProfileBlueprintLibrary::GetBudgetValue(const UObject* WorldContextObject, FName BudgetName)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->GetBudget(BudgetName);
	}
	return 0;
}

// ============================================================================
// Identity Queries
// ============================================================================

EPGXProjectMode UPGXProfileBlueprintLibrary::GetProjectMode(const UObject* WorldContextObject)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->GetResolvedProfile().Identity.ProjectMode;
	}
	return EPGXProjectMode::Game;
}

bool UPGXProfileBlueprintLibrary::IsTargetPlatformActive(const UObject* WorldContextObject, EPGXTargetPlatform Platform)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->GetResolvedProfile().Identity.ActiveTargets.Contains(Platform);
	}
	return false;
}

TArray<EPGXTargetPlatform> UPGXProfileBlueprintLibrary::GetActiveTargets(const UObject* WorldContextObject)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->GetResolvedProfile().Identity.ActiveTargets;
	}
	return { EPGXTargetPlatform::PC };
}

EPGXRestrictionLevel UPGXProfileBlueprintLibrary::GetRestrictionLevel(const UObject* WorldContextObject)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->GetResolvedProfile().Identity.RestrictionLevel;
	}
	return EPGXRestrictionLevel::Relaxed;
}

EPGXBuildContext UPGXProfileBlueprintLibrary::GetBuildContext(const UObject* WorldContextObject)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->GetResolvedProfile().Identity.BuildContext;
	}
	return EPGXBuildContext::Development;
}

// ============================================================================
// Persistence Queries
// ============================================================================

EPGXPersistenceBackend UPGXProfileBlueprintLibrary::GetPersistenceBackend(const UObject* WorldContextObject, FName SystemName)
{
	if (UPGXProfileSubsystem* Sub = GetSubsystem(WorldContextObject))
	{
		return Sub->GetPersistenceBackend(SystemName);
	}
	return EPGXPersistenceBackend::INI;
}
