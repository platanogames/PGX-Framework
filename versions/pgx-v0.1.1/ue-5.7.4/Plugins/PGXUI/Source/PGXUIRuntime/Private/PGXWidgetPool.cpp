// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXWidgetPool.h"

#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "HAL/PlatformTime.h"

void UPGXWidgetPool::Initialize(int32 InCapacity)
{
	Capacity = FMath::Max(0, InCapacity);
	PoolEntries.Reset();
}

FPGXUIResult UPGXWidgetPool::AcquireWidget(TSubclassOf<UUserWidget> WidgetClass, FString DebugName)
{
	if (!WidgetClass)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXUI: AcquireWidget rejected invalid widget class"));
		return FPGXUIResult::Failure(EPGXUIResultCode::InvalidWidgetClass, TEXT("Widget class is invalid."));
	}

	FPGXWidgetPoolEntry* AvailableEntry = PoolEntries.FindByPredicate([](const FPGXWidgetPoolEntry& Entry)
	{
		return Entry.State == EPGXWidgetPoolState::Available;
	});

	if (!AvailableEntry && GetAcquiredCount() >= Capacity)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXUI: AcquireWidget rejected because widget pool is exhausted"));
		return FPGXUIResult::Failure(EPGXUIResultCode::PoolExhausted, TEXT("Widget pool capacity exhausted."));
	}

	if (!AvailableEntry)
	{
		FPGXWidgetPoolEntry Entry;
		Entry.WidgetHandle = FPGXUIHandle::NewHandle();
		PoolEntries.Add(Entry);
		AvailableEntry = &PoolEntries.Last();
	}

	AvailableEntry->WidgetClass = WidgetClass;
	AvailableEntry->State = EPGXWidgetPoolState::Acquired;
	AvailableEntry->DebugName = DebugName.IsEmpty() ? WidgetClass.Get()->GetName() : MoveTemp(DebugName);
	AvailableEntry->AcquiredTimeSeconds = FPlatformTime::Seconds();

	return FPGXUIResult::Success(AvailableEntry->WidgetHandle, TEXT("Widget pool entry acquired."));
}

FPGXUIResult UPGXWidgetPool::ReleaseWidget(FPGXUIHandle WidgetHandle)
{
	if (!WidgetHandle.IsValid())
	{
		return FPGXUIResult::Failure(EPGXUIResultCode::WidgetNotAcquired, TEXT("Widget handle is invalid."), WidgetHandle);
	}

	FPGXWidgetPoolEntry* Entry = FindPoolEntry(WidgetHandle);
	if (!Entry)
	{
		return FPGXUIResult::Failure(EPGXUIResultCode::WidgetNotAcquired, TEXT("Widget handle was not acquired."), WidgetHandle);
	}

	if (Entry->State == EPGXWidgetPoolState::Available)
	{
		return FPGXUIResult::Failure(EPGXUIResultCode::AlreadyReleased, TEXT("Widget handle was already released."), WidgetHandle);
	}

	Entry->State = EPGXWidgetPoolState::Available;
	return FPGXUIResult::Success(WidgetHandle, TEXT("Widget pool entry released."));
}

bool UPGXWidgetPool::HasAcquiredWidget(FPGXUIHandle WidgetHandle) const
{
	const FPGXWidgetPoolEntry* Entry = FindPoolEntry(WidgetHandle);
	return Entry && Entry->State == EPGXWidgetPoolState::Acquired;
}

int32 UPGXWidgetPool::GetCapacity() const
{
	return Capacity;
}

int32 UPGXWidgetPool::GetAcquiredCount() const
{
	int32 AcquiredCount = 0;
	for (const FPGXWidgetPoolEntry& Entry : PoolEntries)
	{
		if (Entry.State == EPGXWidgetPoolState::Acquired)
		{
			++AcquiredCount;
		}
	}
	return AcquiredCount;
}

int32 UPGXWidgetPool::GetAvailableCount() const
{
	return FMath::Max(0, Capacity - GetAcquiredCount());
}

TArray<FPGXWidgetPoolEntry> UPGXWidgetPool::GetPoolSnapshot() const
{
	return PoolEntries;
}

void UPGXWidgetPool::Clear()
{
	PoolEntries.Reset();
}

FPGXWidgetPoolEntry* UPGXWidgetPool::FindPoolEntry(FPGXUIHandle WidgetHandle)
{
	return PoolEntries.FindByPredicate([WidgetHandle](const FPGXWidgetPoolEntry& Entry)
	{
		return WidgetHandle.IsValid() && Entry.WidgetHandle.Id == WidgetHandle.Id;
	});
}

const FPGXWidgetPoolEntry* UPGXWidgetPool::FindPoolEntry(FPGXUIHandle WidgetHandle) const
{
	return PoolEntries.FindByPredicate([WidgetHandle](const FPGXWidgetPoolEntry& Entry)
	{
		return WidgetHandle.IsValid() && Entry.WidgetHandle.Id == WidgetHandle.Id;
	});
}