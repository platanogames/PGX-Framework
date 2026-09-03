// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXUISubsystem.generated.h"

class UPGXNotificationManager;
class UPGXScreenManager;
class UPGXUIConfig;
class UPGXWidgetPool;

/**
 * EN: Central UI manager.
 *     Owns presentation service objects for screen stack, notifications, and widget pool state.
 *
 * ES: Manager central de UI.
 *     Posee servicios de presentacion para stack de pantallas, notificaciones y estado de widget pool.
 */
UCLASS(BlueprintType)
class PGXUIRUNTIME_API UPGXUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	UPGXScreenManager* GetScreenManager() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	UPGXNotificationManager* GetNotificationManager() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	UPGXWidgetPool* GetWidgetPool() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	UPGXUIConfig* GetUIConfig() const;

#if WITH_DEV_AUTOMATION_TESTS
	void InjectTestUIConfig(UPGXUIConfig* InConfig);
	void ResetUIManagersForTesting();
#endif

private:
	void EnsureRuntimeObjects() const;
	void ApplyConfigToManagers() const;

	UPROPERTY(Transient)
	mutable TObjectPtr<UPGXUIConfig> UIConfig;

	UPROPERTY(Transient)
	mutable TObjectPtr<UPGXScreenManager> ScreenManager;

	UPROPERTY(Transient)
	mutable TObjectPtr<UPGXNotificationManager> NotificationManager;

	UPROPERTY(Transient)
	mutable TObjectPtr<UPGXWidgetPool> WidgetPool;
};