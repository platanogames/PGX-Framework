// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXMGOSBlueprintLibrary.h"
#include "PGXGCObserverSubsystem.h"
#include "PGXMGOSRuntime.h"
#include "Engine/Engine.h"

// ============================================================================
// Internal Helper
// ============================================================================

UPGXGCObserverSubsystem* UPGXMGOSBlueprintLibrary::GetSubsystem()
{
	// EN: Engine subsystem — accessed via GEngine, no WorldContext needed
	// ES: Subsistema de Engine — accedido via GEngine, no necesita WorldContext
	return UPGXGCObserverSubsystem::GetCachedInstance();
}

// ============================================================================
// Control
// ============================================================================

void UPGXMGOSBlueprintLibrary::SetMode(EPGXGCObserverMode NewMode)
{
	if (UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		Sub->SetMode(NewMode);
	}
}

void UPGXMGOSBlueprintLibrary::RequestBaselineCapture()
{
	if (UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		Sub->RequestBaselineCapture();
	}
}

void UPGXMGOSBlueprintLibrary::ResetBaseline()
{
	if (UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		Sub->ResetBaseline();
	}
}

void UPGXMGOSBlueprintLibrary::SetSuppressed(bool bSuppress)
{
	if (UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		Sub->SetSuppressed(bSuppress);
	}
}

// ============================================================================
// Query
// ============================================================================

FPGXGCProfile UPGXMGOSBlueprintLibrary::GetCurrentProfile()
{
	if (const UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		return Sub->GetCurrentProfile();
	}
	return FPGXGCProfile();
}

EPGXGCObserverMode UPGXMGOSBlueprintLibrary::GetMode()
{
	if (const UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		return Sub->GetMode();
	}
	return EPGXGCObserverMode::Off;
}

EPGXGCBaselineState UPGXMGOSBlueprintLibrary::GetBaselineState()
{
	if (const UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		return Sub->GetBaselineState();
	}
	return EPGXGCBaselineState::Uninitialized;
}

bool UPGXMGOSBlueprintLibrary::IsInitialized()
{
	if (const UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		return Sub->IsInitialized();
	}
	return false;
}

int64 UPGXMGOSBlueprintLibrary::GetCycleCount()
{
	if (const UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		return Sub->GetCycleCount();
	}
	return 0;
}

bool UPGXMGOSBlueprintLibrary::IsInSuppressedPhase()
{
	if (const UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		return Sub->IsInSuppressedPhase();
	}
	return false;
}

// ============================================================================
// Incidents
// ============================================================================

TArray<FPGXGCIncident> UPGXMGOSBlueprintLibrary::GetCurrentIncidents()
{
	if (const UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		return Sub->GetCurrentIncidents();
	}
	return TArray<FPGXGCIncident>();
}

TArray<FPGXGCClassHealthReport> UPGXMGOSBlueprintLibrary::GetTrackedClassReport()
{
	if (const UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		return Sub->GetTrackedClassReport();
	}
	return TArray<FPGXGCClassHealthReport>();
}

// ============================================================================
// History
// ============================================================================

TArray<FPGXGCSnapshotDiff> UPGXMGOSBlueprintLibrary::GetHistorySummary(int32 Count)
{
	if (const UPGXGCObserverSubsystem* Sub = GetSubsystem())
	{
		return Sub->GetHistorySummary(Count);
	}
	return TArray<FPGXGCSnapshotDiff>();
}
