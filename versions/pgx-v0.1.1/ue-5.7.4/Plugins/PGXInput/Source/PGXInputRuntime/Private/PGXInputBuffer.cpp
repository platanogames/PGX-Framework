// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXInputBuffer.h"

#include "HAL/PlatformTime.h"

void UPGXInputBuffer::Configure(int32 InCapacity, double InWindowSeconds)
{
	Capacity = FMath::Clamp(InCapacity, 1, 128);
	WindowSeconds = FMath::Max(0.0, InWindowSeconds);
	TrimExpired(FPlatformTime::Seconds());
	EnforceCapacity();
}

void UPGXInputBuffer::RecordInput(FGameplayTag ActionTag, FVector Value, double Timestamp)
{
	if (!ActionTag.IsValid())
	{
		return;
	}

	const double ResolvedTimestamp = ResolveTime(Timestamp);
	TrimExpired(ResolvedTimestamp);

	FPGXInputBufferEntry Entry;
	Entry.ActionTag = ActionTag;
	Entry.Timestamp = ResolvedTimestamp;
	Entry.Value = Value;
	Entries.Add(Entry);
	EnforceCapacity();
}

bool UPGXInputBuffer::ContainsRecentInput(FGameplayTag ActionTag, double CurrentTime) const
{
	if (!ActionTag.IsValid())
	{
		return false;
	}

	const double ResolvedTime = ResolveTime(CurrentTime);
	return Entries.ContainsByPredicate([ActionTag, ResolvedTime, this](const FPGXInputBufferEntry& Entry)
	{
		return Entry.ActionTag == ActionTag && ResolvedTime - Entry.Timestamp <= WindowSeconds;
	});
}

bool UPGXInputBuffer::ConsumeRecentInput(FGameplayTag ActionTag, double CurrentTime)
{
	if (!ActionTag.IsValid())
	{
		return false;
	}

	const double ResolvedTime = ResolveTime(CurrentTime);
	TrimExpired(ResolvedTime);

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		const FPGXInputBufferEntry& Entry = Entries[Index];
		if (Entry.ActionTag == ActionTag && ResolvedTime - Entry.Timestamp <= WindowSeconds)
		{
			Entries.RemoveAt(Index, 1, EAllowShrinking::No);
			return true;
		}
	}

	return false;
}

void UPGXInputBuffer::Clear()
{
	Entries.Reset();
}

TArray<FPGXInputBufferEntry> UPGXInputBuffer::GetBufferedInputs() const
{
	return Entries;
}

int32 UPGXInputBuffer::Num() const
{
	return Entries.Num();
}

int32 UPGXInputBuffer::GetCapacity() const
{
	return Capacity;
}

double UPGXInputBuffer::GetWindowSeconds() const
{
	return WindowSeconds;
}

void UPGXInputBuffer::TrimExpired(double CurrentTime)
{
	for (int32 Index = Entries.Num() - 1; Index >= 0; --Index)
	{
		if (CurrentTime - Entries[Index].Timestamp > WindowSeconds)
		{
			Entries.RemoveAt(Index, 1, EAllowShrinking::No);
		}
	}
}

void UPGXInputBuffer::EnforceCapacity()
{
	const int32 ExcessCount = Entries.Num() - Capacity;
	if (ExcessCount > 0)
	{
		Entries.RemoveAt(0, ExcessCount, EAllowShrinking::No);
	}
}

double UPGXInputBuffer::ResolveTime(double MaybeTime)
{
	return MaybeTime >= 0.0 ? MaybeTime : FPlatformTime::Seconds();
}
