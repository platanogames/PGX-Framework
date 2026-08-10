// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Subsystems/PGXConfigSubsystem.h"
#include "Engine/Engine.h"

// EN: Runtime configuration management system implementation.
//     Centralizes config loading with cache + change notification + hot-reload.
// ES: Implementacion del sistema centralizado de gestion de configuracion.

void UPGXConfigSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogPGXSettings, Log, TEXT("[PGXConfigSubsystem] Initialized. Config caching active."));
}

void UPGXConfigSubsystem::Deinitialize()
{
	ConfigCache.Empty();
	UE_LOG(LogPGXSettings, Log, TEXT("[PGXConfigSubsystem] Deinitialized. Cache cleared."));
	Super::Deinitialize();
}

void UPGXConfigSubsystem::ReloadAll()
{
	// Collect system names from cache before clearing
	TArray<FString> Systems;
	ConfigCache.GenerateKeyArray(Systems);

	// Clear cache — next GetActiveConfig will re-resolve from scratch
	ConfigCache.Empty();

	// Notify all subscribers that their config MAY have changed
	// (they re-fetch via GetActiveConfig on the notification handler)
	for (const FString& SystemName : Systems)
	{
		OnPGXConfigChanged.Broadcast(SystemName, nullptr);
	}

	UE_LOG(LogPGXSettings, Log, TEXT("[PGXConfigSubsystem] ReloadAll: Cleared %d cached configs. Subscribers notified."), Systems.Num());
}

UPGXConfigSubsystem* UPGXConfigSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UPGXConfigSubsystem>();
}
