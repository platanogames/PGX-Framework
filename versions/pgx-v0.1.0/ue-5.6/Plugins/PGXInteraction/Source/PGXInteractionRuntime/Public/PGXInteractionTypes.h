// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXInteractionTypes.generated.h"

class AActor;

/** EN: Lifecycle state for an interaction action / ES: Estado de ciclo de vida de una accion de interaccion */
UENUM(BlueprintType)
enum class EPGXInteractionActionState : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Requested = 1 UMETA(DisplayName = "Requested"),
	Started = 2 UMETA(DisplayName = "Started"),
	Completed = 3 UMETA(DisplayName = "Completed"),
	Failed = 4 UMETA(DisplayName = "Failed"),
	Cancelled = 5 UMETA(DisplayName = "Cancelled")
};

/** EN: Typed outcome codes for interaction operations / ES: Codigos tipados para resultados de interaccion */
UENUM(BlueprintType)
enum class EPGXInteractionResultCode : uint8
{
	Success = 0 UMETA(DisplayName = "Success"),
	InvalidInteractor = 1 UMETA(DisplayName = "Invalid Interactor"),
	InvalidTarget = 2 UMETA(DisplayName = "Invalid Target"),
	InvalidAction = 3 UMETA(DisplayName = "Invalid Action"),
	TargetNotRegistered = 4 UMETA(DisplayName = "Target Not Registered"),
	AlreadyActive = 5 UMETA(DisplayName = "Already Active"),
	NoActiveAction = 6 UMETA(DisplayName = "No Active Action"),
	AlreadyResolved = 7 UMETA(DisplayName = "Already Resolved"),
	ConditionFailed = 8 UMETA(DisplayName = "Condition Failed"),
	InternalError = 9 UMETA(DisplayName = "Internal Error"),
	OwnerMissing = 10 UMETA(DisplayName = "Owner Missing"),
	WorldUnavailable = 11 UMETA(DisplayName = "World Unavailable"),
	NoTargetFound = 12 UMETA(DisplayName = "No Target Found"),
	TargetOutOfRange = 13 UMETA(DisplayName = "Target Out Of Range"),
	InvalidInterface = 14 UMETA(DisplayName = "Invalid Interface")
};

/** EN: Stable opaque interaction handle / ES: Handle opaco estable de interaccion */
USTRUCT(BlueprintType)
struct PGXINTERACTIONRUNTIME_API FPGXInteractionHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FGuid Id;

	bool IsValid() const { return Id.IsValid(); }
	static FPGXInteractionHandle NewHandle();
};

/** EN: Registered target snapshot / ES: Snapshot de target registrado */
USTRUCT(BlueprintType)
struct PGXINTERACTIONRUNTIME_API FPGXInteractableTarget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FPGXInteractionHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction", meta = (Categories = "PGX.Interaction.Target"))
	FGameplayTag TargetTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FText PromptText;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	int32 Priority = 0;
};

/** EN: Interaction action request / ES: Peticion de accion de interaccion */
USTRUCT(BlueprintType)
struct PGXINTERACTIONRUNTIME_API FPGXInteractionActionRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FPGXInteractionHandle TargetHandle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction", meta = (Categories = "PGX.Interaction.Action"))
	FGameplayTag ActionTag;
};

/** EN: Runtime interaction record / ES: Registro runtime de interaccion */
USTRUCT(BlueprintType)
struct PGXINTERACTIONRUNTIME_API FPGXInteractionRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FPGXInteractionHandle ActionHandle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FPGXInteractionHandle TargetHandle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction", meta = (Categories = "PGX.Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	EPGXInteractionActionState State = EPGXInteractionActionState::None;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	EPGXInteractionResultCode ResultCode = EPGXInteractionResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	double StartedTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	double ResolvedTimeSeconds = 0.0;
};

/** EN: Presentation-only prompt payload / ES: Payload de prompt solo-presentacion */
USTRUCT(BlueprintType)
struct PGXINTERACTIONRUNTIME_API FPGXInteractionPromptSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	bool bHasPrompt = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	bool bPresentationOnly = true;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FPGXInteractionHandle TargetHandle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction", meta = (Categories = "PGX.Interaction.Target"))
	FGameplayTag TargetTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction", meta = (Categories = "PGX.Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FText PromptText;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	float Distance = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	int32 Priority = 0;
};

/** EN: Typed trace/query result / ES: Resultado tipado de trace/query */
USTRUCT(BlueprintType)
struct PGXINTERACTIONRUNTIME_API FPGXInteractionQueryResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	EPGXInteractionResultCode Code = EPGXInteractionResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FPGXInteractionPromptSnapshot PromptSnapshot;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FString Message;

	static FPGXInteractionQueryResult Success(FPGXInteractionPromptSnapshot InPromptSnapshot, FString InMessage = FString());
	static FPGXInteractionQueryResult Failure(EPGXInteractionResultCode InCode, FString InMessage, FPGXInteractionHandle InTargetHandle = FPGXInteractionHandle());
};

/** EN: Typed interaction operation result / ES: Resultado tipado de operacion de interaccion */
USTRUCT(BlueprintType)
struct PGXINTERACTIONRUNTIME_API FPGXInteractionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	EPGXInteractionResultCode Code = EPGXInteractionResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	EPGXInteractionActionState State = EPGXInteractionActionState::None;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FPGXInteractionHandle ActionHandle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FPGXInteractionHandle TargetHandle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Interaction")
	FString Message;

	static FPGXInteractionResult Success(FPGXInteractionHandle InActionHandle, FPGXInteractionHandle InTargetHandle, EPGXInteractionActionState InState, FString InMessage = FString());
	static FPGXInteractionResult Failure(EPGXInteractionResultCode InCode, EPGXInteractionActionState InState, FString InMessage, FPGXInteractionHandle InActionHandle = FPGXInteractionHandle(), FPGXInteractionHandle InTargetHandle = FPGXInteractionHandle());
};
