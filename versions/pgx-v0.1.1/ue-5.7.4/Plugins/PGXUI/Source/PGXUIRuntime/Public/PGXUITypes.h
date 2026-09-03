// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PGXUITypes.generated.h"

/** EN: Typed outcome codes for UI presentation operations / ES: Codigos tipados para operaciones de presentacion UI */
UENUM(BlueprintType)
enum class EPGXUIResultCode : uint8
{
	Success = 0 UMETA(DisplayName = "Success"),
	InvalidScreen = 1 UMETA(DisplayName = "Invalid Screen"),
	InvalidNotification = 2 UMETA(DisplayName = "Invalid Notification"),
	InvalidWidgetClass = 3 UMETA(DisplayName = "Invalid Widget Class"),
	StackOverflow = 4 UMETA(DisplayName = "Stack Overflow"),
	StackUnderflow = 5 UMETA(DisplayName = "Stack Underflow"),
	NotificationNotFound = 6 UMETA(DisplayName = "Notification Not Found"),
	PoolExhausted = 7 UMETA(DisplayName = "Pool Exhausted"),
	WidgetNotAcquired = 8 UMETA(DisplayName = "Widget Not Acquired"),
	AlreadyReleased = 9 UMETA(DisplayName = "Already Released"),
	InternalError = 10 UMETA(DisplayName = "Internal Error")
};

/** EN: Screen lifecycle state / ES: Estado de ciclo de vida de pantalla */
UENUM(BlueprintType)
enum class EPGXUIScreenState : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Open = 1 UMETA(DisplayName = "Open"),
	Closed = 2 UMETA(DisplayName = "Closed")
};

/** EN: Notification lifecycle state / ES: Estado de ciclo de vida de notificacion */
UENUM(BlueprintType)
enum class EPGXUINotificationState : uint8
{
	Queued = 0 UMETA(DisplayName = "Queued"),
	Dismissed = 1 UMETA(DisplayName = "Dismissed")
};

/** EN: Widget pool slot state / ES: Estado de entrada del pool de widgets */
UENUM(BlueprintType)
enum class EPGXWidgetPoolState : uint8
{
	Available = 0 UMETA(DisplayName = "Available"),
	Acquired = 1 UMETA(DisplayName = "Acquired")
};

/** EN: Stable opaque UI handle / ES: Handle opaco estable de UI */
USTRUCT(BlueprintType)
struct PGXUIRUNTIME_API FPGXUIHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FGuid Id;

	bool IsValid() const { return Id.IsValid(); }
	static FPGXUIHandle NewHandle();
};

/** EN: Presentation-only screen stack entry / ES: Entrada de stack de pantalla solo de presentacion */
USTRUCT(BlueprintType)
struct PGXUIRUNTIME_API FPGXUIScreenEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FPGXUIHandle ScreenHandle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI", meta = (Categories = "PGX.UI.Screen"))
	FGameplayTag ScreenTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	int32 Layer = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	EPGXUIScreenState State = EPGXUIScreenState::None;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FString DebugName;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	double OpenedTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	double ClosedTimeSeconds = 0.0;
};

/** EN: Notification request data / ES: Datos de peticion de notificacion */
USTRUCT(BlueprintType)
struct PGXUIRUNTIME_API FPGXUINotificationRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|UI", meta = (Categories = "PGX.UI.Notification"))
	FGameplayTag NotificationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|UI")
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|UI")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|UI", meta = (ClampMin = "0.0"))
	float DisplayTimeSeconds = 0.0f;
};

/** EN: Queued notification record / ES: Registro de notificacion en cola */
USTRUCT(BlueprintType)
struct PGXUIRUNTIME_API FPGXUINotificationEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FPGXUIHandle NotificationHandle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FPGXUINotificationRequest Request;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	EPGXUINotificationState State = EPGXUINotificationState::Queued;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	double QueuedTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	double DismissedTimeSeconds = 0.0;
};

/** EN: State-only widget pool entry; no widget spawning is performed / ES: Entrada de pool solo de estado; no crea widgets */
USTRUCT(BlueprintType)
struct PGXUIRUNTIME_API FPGXWidgetPoolEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FPGXUIHandle WidgetHandle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	TSubclassOf<UUserWidget> WidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	EPGXWidgetPoolState State = EPGXWidgetPoolState::Available;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FString DebugName;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	double AcquiredTimeSeconds = 0.0;
};

/** EN: Presentation-only view snapshot / ES: Snapshot de vista solo de presentacion */
USTRUCT(BlueprintType)
struct PGXUIRUNTIME_API FPGXUIViewSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI", meta = (Categories = "PGX.UI.View"))
	FGameplayTag SourceTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FName ViewName;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FString PayloadSummary;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	double TimestampSeconds = 0.0;
};

/** EN: Typed UI operation result / ES: Resultado tipado de operacion UI */
USTRUCT(BlueprintType)
struct PGXUIRUNTIME_API FPGXUIResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	EPGXUIResultCode Code = EPGXUIResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FPGXUIHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|UI")
	FString Message;

	static FPGXUIResult Success(FPGXUIHandle InHandle, FString InMessage = FString());
	static FPGXUIResult Failure(EPGXUIResultCode InCode, FString InMessage, FPGXUIHandle InHandle = FPGXUIHandle());
};