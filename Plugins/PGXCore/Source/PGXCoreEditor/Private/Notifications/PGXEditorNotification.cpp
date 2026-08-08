// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Notifications/PGXEditorNotification.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void UPGXEditorNotification::ShowInfo(const FText& Message)
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = 4.0f;
	Info.bFireAndForget = true;
	Info.bUseSuccessFailIcons = false;
	FSlateNotificationManager::Get().AddNotification(Info);
}

void UPGXEditorNotification::ShowWarning(const FText& Message)
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = 6.0f;
	Info.bFireAndForget = true;
	Info.bUseSuccessFailIcons = true;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	if (NotificationItem.IsValid())
	{
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
	}
}

void UPGXEditorNotification::ShowError(const FText& Message)
{
	FNotificationInfo Info(Message);
	Info.ExpireDuration = 8.0f;
	Info.bFireAndForget = true;
	Info.bUseSuccessFailIcons = true;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	if (NotificationItem.IsValid())
	{
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
	}
}
