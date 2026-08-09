// Copyright PGX Framework. All Rights Reserved.

#include "PGXUISubsystem.h"

#include "PGXNotificationManager.h"
#include "PGXScreenManager.h"
#include "PGXUIConfig.h"
#include "PGXWidgetPool.h"

void UPGXUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	EnsureRuntimeObjects();
	ApplyConfigToManagers();
}

void UPGXUISubsystem::Deinitialize()
{
	if (ScreenManager)
	{
		ScreenManager->Clear();
	}
	if (NotificationManager)
	{
		NotificationManager->Clear();
	}
	if (WidgetPool)
	{
		WidgetPool->Clear();
	}

	ScreenManager = nullptr;
	NotificationManager = nullptr;
	WidgetPool = nullptr;
	UIConfig = nullptr;

	Super::Deinitialize();
}

UPGXScreenManager* UPGXUISubsystem::GetScreenManager() const
{
	EnsureRuntimeObjects();
	return ScreenManager;
}

UPGXNotificationManager* UPGXUISubsystem::GetNotificationManager() const
{
	EnsureRuntimeObjects();
	return NotificationManager;
}

UPGXWidgetPool* UPGXUISubsystem::GetWidgetPool() const
{
	EnsureRuntimeObjects();
	return WidgetPool;
}

UPGXUIConfig* UPGXUISubsystem::GetUIConfig() const
{
	EnsureRuntimeObjects();
	return UIConfig;
}

#if WITH_DEV_AUTOMATION_TESTS
void UPGXUISubsystem::InjectTestUIConfig(UPGXUIConfig* InConfig)
{
	UIConfig = InConfig;
	EnsureRuntimeObjects();
	ApplyConfigToManagers();
}

void UPGXUISubsystem::ResetUIManagersForTesting()
{
	EnsureRuntimeObjects();
	ApplyConfigToManagers();
}
#endif

void UPGXUISubsystem::EnsureRuntimeObjects() const
{
	if (!UIConfig)
	{
		UIConfig = NewObject<UPGXUIConfig>(const_cast<UPGXUISubsystem*>(this), UPGXUIConfig::StaticClass(), NAME_None, RF_Transient);
	}
	if (!ScreenManager)
	{
		ScreenManager = NewObject<UPGXScreenManager>(const_cast<UPGXUISubsystem*>(this), UPGXScreenManager::StaticClass(), NAME_None, RF_Transient);
	}
	if (!NotificationManager)
	{
		NotificationManager = NewObject<UPGXNotificationManager>(const_cast<UPGXUISubsystem*>(this), UPGXNotificationManager::StaticClass(), NAME_None, RF_Transient);
	}
	if (!WidgetPool)
	{
		WidgetPool = NewObject<UPGXWidgetPool>(const_cast<UPGXUISubsystem*>(this), UPGXWidgetPool::StaticClass(), NAME_None, RF_Transient);
	}
}

void UPGXUISubsystem::ApplyConfigToManagers() const
{
	EnsureRuntimeObjects();
	ScreenManager->Initialize(UIConfig ? UIConfig->MaxScreenStackDepth : 10);
	NotificationManager->Initialize(UIConfig ? UIConfig->NotificationDisplayTime : 3.0f);
	WidgetPool->Initialize(UIConfig ? UIConfig->WidgetPoolInitialSize : 16);
}