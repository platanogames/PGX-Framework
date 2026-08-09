// Copyright PGX Framework. All Rights Reserved.

#include "PGXNotificationManager.h"

#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "HAL/PlatformTime.h"

void UPGXNotificationManager::Initialize(float InDefaultDisplayTimeSeconds)
{
	DefaultDisplayTimeSeconds = FMath::Max(0.0f, InDefaultDisplayTimeSeconds);
	NotificationQueue.Reset();
}

FPGXUIResult UPGXNotificationManager::EnqueueNotification(const FPGXUINotificationRequest& Request)
{
	if (!Request.NotificationTag.IsValid() || Request.Message.ToString().IsEmpty())
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXUI: EnqueueNotification rejected invalid notification request"));
		return FPGXUIResult::Failure(EPGXUIResultCode::InvalidNotification, TEXT("Notification tag or message is invalid."));
	}

	FPGXUINotificationEntry Entry;
	Entry.NotificationHandle = FPGXUIHandle::NewHandle();
	Entry.Request = Request;
	if (Entry.Request.DisplayTimeSeconds <= 0.0f)
	{
		Entry.Request.DisplayTimeSeconds = DefaultDisplayTimeSeconds;
	}
	Entry.State = EPGXUINotificationState::Queued;
	Entry.QueuedTimeSeconds = FPlatformTime::Seconds();
	NotificationQueue.Add(Entry);

	return FPGXUIResult::Success(Entry.NotificationHandle, TEXT("Notification queued."));
}

FPGXUIResult UPGXNotificationManager::DismissNotification(FPGXUIHandle NotificationHandle)
{
	if (!NotificationHandle.IsValid())
	{
		return FPGXUIResult::Failure(EPGXUIResultCode::InvalidNotification, TEXT("Notification handle is invalid."), NotificationHandle);
	}

	FPGXUINotificationEntry* Entry = FindQueuedNotification(NotificationHandle);
	if (!Entry)
	{
		return FPGXUIResult::Failure(EPGXUIResultCode::NotificationNotFound, TEXT("Notification was not queued."), NotificationHandle);
	}

	Entry->State = EPGXUINotificationState::Dismissed;
	Entry->DismissedTimeSeconds = FPlatformTime::Seconds();
	NotificationQueue.RemoveAll([NotificationHandle](const FPGXUINotificationEntry& Candidate)
	{
		return Candidate.NotificationHandle.Id == NotificationHandle.Id;
	});

	return FPGXUIResult::Success(NotificationHandle, TEXT("Notification dismissed."));
}

bool UPGXNotificationManager::HasNotification(FPGXUIHandle NotificationHandle) const
{
	return FindQueuedNotification(NotificationHandle) != nullptr;
}

int32 UPGXNotificationManager::GetQueuedNotificationCount() const
{
	int32 QueuedCount = 0;
	for (const FPGXUINotificationEntry& Entry : NotificationQueue)
	{
		if (Entry.State == EPGXUINotificationState::Queued)
		{
			++QueuedCount;
		}
	}
	return QueuedCount;
}

TArray<FPGXUINotificationEntry> UPGXNotificationManager::GetNotificationQueueSnapshot() const
{
	TArray<FPGXUINotificationEntry> Snapshot = NotificationQueue;
	Snapshot.Sort([](const FPGXUINotificationEntry& Left, const FPGXUINotificationEntry& Right)
	{
		if (Left.Request.Priority != Right.Request.Priority)
		{
			return Left.Request.Priority > Right.Request.Priority;
		}
		return Left.QueuedTimeSeconds < Right.QueuedTimeSeconds;
	});
	return Snapshot;
}

void UPGXNotificationManager::Clear()
{
	NotificationQueue.Reset();
}

const FPGXUINotificationEntry* UPGXNotificationManager::FindQueuedNotification(FPGXUIHandle NotificationHandle) const
{
	return NotificationQueue.FindByPredicate([NotificationHandle](const FPGXUINotificationEntry& Entry)
	{
		return NotificationHandle.IsValid() && Entry.NotificationHandle.Id == NotificationHandle.Id && Entry.State == EPGXUINotificationState::Queued;
	});
}

FPGXUINotificationEntry* UPGXNotificationManager::FindQueuedNotification(FPGXUIHandle NotificationHandle)
{
	return NotificationQueue.FindByPredicate([NotificationHandle](const FPGXUINotificationEntry& Entry)
	{
		return NotificationHandle.IsValid() && Entry.NotificationHandle.Id == NotificationHandle.Id && Entry.State == EPGXUINotificationState::Queued;
	});
}