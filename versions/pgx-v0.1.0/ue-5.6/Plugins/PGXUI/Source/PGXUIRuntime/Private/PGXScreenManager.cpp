// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXScreenManager.h"

#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "HAL/PlatformTime.h"

void UPGXScreenManager::Initialize(int32 InMaxStackDepth)
{
	MaxStackDepth = FMath::Max(1, InMaxStackDepth);
	ScreenStack.Reset();
}

FPGXUIResult UPGXScreenManager::PushScreen(FGameplayTag ScreenTag, FString DebugName, int32 Layer)
{
	if (!ScreenTag.IsValid())
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXUI: PushScreen rejected invalid screen tag"));
		return FPGXUIResult::Failure(EPGXUIResultCode::InvalidScreen, TEXT("Screen tag is invalid."));
	}

	if (GetOpenScreenCount() >= MaxStackDepth)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXUI: PushScreen rejected because screen stack is full"));
		return FPGXUIResult::Failure(EPGXUIResultCode::StackOverflow, TEXT("Screen stack depth limit reached."));
	}

	FPGXUIScreenEntry Entry;
	Entry.ScreenHandle = FPGXUIHandle::NewHandle();
	Entry.ScreenTag = ScreenTag;
	Entry.Layer = Layer;
	Entry.State = EPGXUIScreenState::Open;
	Entry.DebugName = DebugName.IsEmpty() ? ScreenTag.ToString() : MoveTemp(DebugName);
	Entry.OpenedTimeSeconds = FPlatformTime::Seconds();
	ScreenStack.Add(Entry);

	return FPGXUIResult::Success(Entry.ScreenHandle, TEXT("Screen pushed."));
}

FPGXUIResult UPGXScreenManager::PopScreen()
{
	for (int32 Index = ScreenStack.Num() - 1; Index >= 0; --Index)
	{
		FPGXUIScreenEntry& Entry = ScreenStack[Index];
		if (Entry.State == EPGXUIScreenState::Open)
		{
			Entry.State = EPGXUIScreenState::Closed;
			Entry.ClosedTimeSeconds = FPlatformTime::Seconds();
			const FPGXUIHandle Handle = Entry.ScreenHandle;
			ScreenStack.RemoveAt(Index);
			return FPGXUIResult::Success(Handle, TEXT("Screen popped."));
		}
	}

	return FPGXUIResult::Failure(EPGXUIResultCode::StackUnderflow, TEXT("Screen stack is empty."));
}

FPGXUIResult UPGXScreenManager::CloseScreen(FPGXUIHandle ScreenHandle)
{
	if (!ScreenHandle.IsValid())
	{
		return FPGXUIResult::Failure(EPGXUIResultCode::InvalidScreen, TEXT("Screen handle is invalid."), ScreenHandle);
	}

	const int32 RemovedCount = ScreenStack.RemoveAll([ScreenHandle](const FPGXUIScreenEntry& Entry)
	{
		return Entry.ScreenHandle.Id == ScreenHandle.Id && Entry.State == EPGXUIScreenState::Open;
	});

	if (RemovedCount <= 0)
	{
		return FPGXUIResult::Failure(EPGXUIResultCode::InvalidScreen, TEXT("Screen handle was not open."), ScreenHandle);
	}

	return FPGXUIResult::Success(ScreenHandle, TEXT("Screen closed."));
}

bool UPGXScreenManager::HasScreen(FPGXUIHandle ScreenHandle) const
{
	return FindOpenScreen(ScreenHandle) != nullptr;
}

int32 UPGXScreenManager::GetOpenScreenCount() const
{
	int32 OpenCount = 0;
	for (const FPGXUIScreenEntry& Entry : ScreenStack)
	{
		if (Entry.State == EPGXUIScreenState::Open)
		{
			++OpenCount;
		}
	}
	return OpenCount;
}

int32 UPGXScreenManager::GetMaxStackDepth() const
{
	return MaxStackDepth;
}

TArray<FPGXUIScreenEntry> UPGXScreenManager::GetScreenStackSnapshot() const
{
	return ScreenStack;
}

void UPGXScreenManager::Clear()
{
	ScreenStack.Reset();
}

FPGXUIScreenEntry* UPGXScreenManager::FindOpenScreen(FPGXUIHandle ScreenHandle)
{
	return ScreenStack.FindByPredicate([ScreenHandle](const FPGXUIScreenEntry& Entry)
	{
		return ScreenHandle.IsValid() && Entry.ScreenHandle.Id == ScreenHandle.Id && Entry.State == EPGXUIScreenState::Open;
	});
}

const FPGXUIScreenEntry* UPGXScreenManager::FindOpenScreen(FPGXUIHandle ScreenHandle) const
{
	return ScreenStack.FindByPredicate([ScreenHandle](const FPGXUIScreenEntry& Entry)
	{
		return ScreenHandle.IsValid() && Entry.ScreenHandle.Id == ScreenHandle.Id && Entry.State == EPGXUIScreenState::Open;
	});
}