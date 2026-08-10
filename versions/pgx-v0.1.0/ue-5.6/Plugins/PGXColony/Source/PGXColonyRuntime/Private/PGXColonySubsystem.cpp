// Copyright PGX Framework. All Rights Reserved.

#include "PGXColonySubsystem.h"
#include "Engine/GameInstance.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXColony, Log, All);

void UPGXColonySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	NextSurvivorId = 1;
	SurvivorRegistry.Reset();

	PGX_LOG_INFO(LogPGXColony, TEXT("UPGXColonySubsystem: Initialized (game instance: %s)"),
		GetGameInstance() ? *GetGameInstance()->GetName() : TEXT("<null>"));
}

void UPGXColonySubsystem::Deinitialize()
{
	const int32 LeakedSurvivors = SurvivorRegistry.Num();
	if (LeakedSurvivors > 0)
	{
		PGX_LOG_WARNING(LogPGXColony,
			TEXT("UPGXColonySubsystem: Deinitialize with %d survivor(s) still registered. Owners should unregister explicitly when colony tear-down happens before subsystem teardown."),
			LeakedSurvivors);
	}

	SurvivorRegistry.Reset();
	NextSurvivorId = 1;

	PGX_LOG_INFO(LogPGXColony, TEXT("UPGXColonySubsystem: Deinitialized"));

	Super::Deinitialize();
}

// ============================================================================
// Survivor Registry (Development Preview slice)
// ============================================================================

FPGXColonySurvivorHandle UPGXColonySubsystem::RegisterSurvivor(FGameplayTag DefinitionTag, FPGXColonyResult& OutResult)
{
	FPGXColonySurvivorHandle NewHandle;
	NewHandle.SurvivorId = NextSurvivorId++;
	NewHandle.DefinitionTag = DefinitionTag;

	SurvivorRegistry.Add(NewHandle.SurvivorId, NewHandle);
	OutResult = FPGXColonyResult::MakeSuccess();

	PGX_LOG_INFO(LogPGXColony, TEXT("UPGXColonySubsystem::RegisterSurvivor — id=%d, definitionTag=%s"),
		NewHandle.SurvivorId,
		DefinitionTag.IsValid() ? *DefinitionTag.ToString() : TEXT("<none>"));

	return NewHandle;
}

FPGXColonyResult UPGXColonySubsystem::UnregisterSurvivor(const FPGXColonySurvivorHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return FPGXColonyResult::MakeFailure(EPGXColonyResultCode::InvalidInput,
			TEXT("UnregisterSurvivor: Handle is invalid (SurvivorId==0)"));
	}

	const int32 Removed = SurvivorRegistry.Remove(Handle.SurvivorId);
	if (Removed == 0)
	{
		return FPGXColonyResult::MakeFailure(EPGXColonyResultCode::NotFound,
			FString::Printf(TEXT("UnregisterSurvivor: SurvivorId=%d not registered"), Handle.SurvivorId));
	}

	PGX_LOG_INFO(LogPGXColony, TEXT("UPGXColonySubsystem::UnregisterSurvivor — id=%d"), Handle.SurvivorId);

	return FPGXColonyResult::MakeSuccess();
}

TArray<FPGXColonySurvivorHandle> UPGXColonySubsystem::GetSurvivorSnapshot() const
{
	TArray<FPGXColonySurvivorHandle> Snapshot;
	Snapshot.Reserve(SurvivorRegistry.Num());
	for (const TPair<int32, FPGXColonySurvivorHandle>& Pair : SurvivorRegistry)
	{
		Snapshot.Add(Pair.Value);
	}
	return Snapshot;
}

bool UPGXColonySubsystem::FindSurvivor(int32 SurvivorId, FPGXColonySurvivorHandle& OutHandle) const
{
	const FPGXColonySurvivorHandle* Found = SurvivorRegistry.Find(SurvivorId);
	if (Found)
	{
		OutHandle = *Found;
		return true;
	}
	OutHandle = FPGXColonySurvivorHandle();
	return false;
}
