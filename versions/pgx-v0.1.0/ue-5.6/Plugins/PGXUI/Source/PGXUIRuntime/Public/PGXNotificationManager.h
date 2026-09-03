// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXUITypes.h"
#include "PGXNotificationManager.generated.h"

/**
 * EN: Notification queue manager.
 *     Handles presentation-only queued messages with deterministic priority ordering.
 *
 * ES: Manager de cola de notificaciones.
 *     Maneja mensajes en cola solo de presentacion con orden determinista por prioridad.
 */
UCLASS(BlueprintType)
class PGXUIRUNTIME_API UPGXNotificationManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(float InDefaultDisplayTimeSeconds);

	UFUNCTION(BlueprintCallable, Category = "PGX|UI")
	FPGXUIResult EnqueueNotification(const FPGXUINotificationRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "PGX|UI")
	FPGXUIResult DismissNotification(FPGXUIHandle NotificationHandle);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	bool HasNotification(FPGXUIHandle NotificationHandle) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	int32 GetQueuedNotificationCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|UI")
	TArray<FPGXUINotificationEntry> GetNotificationQueueSnapshot() const;

	void Clear();

private:
	const FPGXUINotificationEntry* FindQueuedNotification(FPGXUIHandle NotificationHandle) const;
	FPGXUINotificationEntry* FindQueuedNotification(FPGXUIHandle NotificationHandle);

	UPROPERTY(Transient)
	TArray<FPGXUINotificationEntry> NotificationQueue;

	UPROPERTY(Transient)
	float DefaultDisplayTimeSeconds = 3.0f;
};