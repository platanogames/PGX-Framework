// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/PGXLogRingBuffer.h"

// EN: Thread-safe ring buffer implementation for PGX Log System
// ES: Implementacion del ring buffer thread-safe para el Sistema de Logs PGX

FPGXLogRingBuffer::FPGXLogRingBuffer(int32 InCapacity)
	: Capacity(FMath::Max(InCapacity, 64))
{
	Buffer.SetNum(Capacity);
}

void FPGXLogRingBuffer::Push(FPGXLogEntry&& Entry)
{
	FScopeLock ScopedLock(&Lock);

	Buffer[Head] = MoveTemp(Entry);
	Head = (Head + 1) % Capacity;

	if (Count < Capacity)
	{
		++Count;
	}
}

TArray<FPGXLogEntry> FPGXLogRingBuffer::Snapshot() const
{
	FScopeLock ScopedLock(&Lock);

	TArray<FPGXLogEntry> Result;
	Result.Reserve(Count);

	// EN: Read from oldest to newest in chronological order
	// ES: Leer de mas antigua a mas nueva en orden cronologico
	const int32 Start = (Count < Capacity) ? 0 : Head;
	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Index = (Start + i) % Capacity;
		Result.Add(Buffer[Index]);
	}

	return Result;
}

TArray<FPGXLogEntry> FPGXLogRingBuffer::SnapshotFiltered(EPGXLogVerbosity MinLevel, FName CategoryFilter) const
{
	FScopeLock ScopedLock(&Lock);

	TArray<FPGXLogEntry> Result;

	const int32 Start = (Count < Capacity) ? 0 : Head;
	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Index = (Start + i) % Capacity;
		const FPGXLogEntry& Entry = Buffer[Index];

		// EN: Filter by verbosity (higher enum value = more severe)
		// ES: Filtrar por verbosity (valor enum mayor = mas severo)
		if (Entry.Verbosity < MinLevel)
		{
			continue;
		}

		// EN: Filter by category if specified
		// ES: Filtrar por categoria si se especifico
		if (CategoryFilter != NAME_None && Entry.Category != CategoryFilter)
		{
			continue;
		}

		Result.Add(Entry);
	}

	return Result;
}

TArray<FPGXLogEntry> FPGXLogRingBuffer::SnapshotByDomain(const FGameplayTag& DomainTag) const
{
	FScopeLock ScopedLock(&Lock);

	TArray<FPGXLogEntry> Result;

	const int32 Start = (Count < Capacity) ? 0 : Head;
	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Index = (Start + i) % Capacity;
		const FPGXLogEntry& Entry = Buffer[Index];

		if (Entry.DomainTag == DomainTag)
		{
			Result.Add(Entry);
		}
	}

	return Result;
}

int32 FPGXLogRingBuffer::Num() const
{
	FScopeLock ScopedLock(&Lock);
	return Count;
}

void FPGXLogRingBuffer::Clear()
{
	FScopeLock ScopedLock(&Lock);

	Head = 0;
	Count = 0;
	// EN: Reset entries to default state
	// ES: Resetear entradas a estado default
	for (FPGXLogEntry& Entry : Buffer)
	{
		Entry = FPGXLogEntry();
	}
}

void FPGXLogRingBuffer::Resize(int32 NewCapacity)
{
	FScopeLock ScopedLock(&Lock);

	Capacity = FMath::Max(NewCapacity, 64);
	Buffer.SetNum(Capacity);
	Head = 0;
	Count = 0;
}
