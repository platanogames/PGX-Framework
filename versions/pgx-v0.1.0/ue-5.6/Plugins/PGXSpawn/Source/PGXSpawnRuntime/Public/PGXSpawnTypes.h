// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "PGXSpawnTypes.generated.h"

class AActor;

/** EN: Spawn request lifecycle status / ES: Estado de ciclo de vida de una peticion de spawn */
UENUM(BlueprintType)
enum class EPGXSpawnRequestStatus : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Queued = 1 UMETA(DisplayName = "Queued"),
	Running = 2 UMETA(DisplayName = "Running"),
	Completed = 3 UMETA(DisplayName = "Completed"),
	Failed = 4 UMETA(DisplayName = "Failed"),
	Cancelled = 5 UMETA(DisplayName = "Cancelled"),
	Expired = 6 UMETA(DisplayName = "Expired")
};

/** EN: Typed outcome codes for spawn operations / ES: Codigos tipados para resultados de spawn */
UENUM(BlueprintType)
enum class EPGXSpawnResultCode : uint8
{
	Success = 0 UMETA(DisplayName = "Success"),
	InvalidRequest = 1 UMETA(DisplayName = "Invalid Request"),
	InvalidSpawnClass = 2 UMETA(DisplayName = "Invalid Spawn Class"),
	InvalidTransform = 3 UMETA(DisplayName = "Invalid Transform"),
	BudgetExceeded = 4 UMETA(DisplayName = "Budget Exceeded"),
	RecordNotFound = 5 UMETA(DisplayName = "Record Not Found"),
	AlreadyCompleted = 6 UMETA(DisplayName = "Already Completed"),
	InternalError = 7 UMETA(DisplayName = "Internal Error"),
	InvalidWorld = 8 UMETA(DisplayName = "Invalid World"),
	SpawnActorFailed = 9 UMETA(DisplayName = "Spawn Actor Failed")
};

/** EN: Typed lifecycle event for spawn record visibility / ES: Evento tipado de ciclo de vida para visibilidad del registro */
UENUM(BlueprintType)
enum class EPGXSpawnLifecycleEventType : uint8
{
	Requested = 0 UMETA(DisplayName = "Requested"),
	Spawned = 1 UMETA(DisplayName = "Spawned"),
	Completed = 2 UMETA(DisplayName = "Completed"),
	Cancelled = 3 UMETA(DisplayName = "Cancelled"),
	Failed = 4 UMETA(DisplayName = "Failed"),
	Cleanup = 5 UMETA(DisplayName = "Cleanup")
};

/** EN: One visible lifecycle event in a spawn record / ES: Evento visible de ciclo de vida en un registro de spawn */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnLifecycleEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	EPGXSpawnLifecycleEventType EventType = EPGXSpawnLifecycleEventType::Requested;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	EPGXSpawnResultCode ResultCode = EPGXSpawnResultCode::Success;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	double TimestampSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	FString Message;
};

/** EN: Stable handle for a spawn request / ES: Handle estable para una peticion de spawn */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnRequestHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	FGuid Id;

	bool IsValid() const { return Id.IsValid(); }
	static FPGXSpawnRequestHandle NewHandle();
};

/** EN: Immutable baseline spawn request / ES: Peticion base inmutable de spawn */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn")
	TSubclassOf<AActor> SpawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (Categories = "PGX.Spawn.Source"))
	FGameplayTag SourceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn")
	int32 Priority = 0;
};

/** EN: Registry record for active or completed spawn work / ES: Registro de trabajo de spawn activo o completado */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	FPGXSpawnRequestHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	FPGXSpawnRequest Request;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	EPGXSpawnRequestStatus Status = EPGXSpawnRequestStatus::None;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	EPGXSpawnResultCode ResultCode = EPGXSpawnResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	double CreatedTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	double CompletedTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	TArray<FPGXSpawnLifecycleEvent> LifecycleEvents;

	TWeakObjectPtr<AActor> SpawnedActor;
};

/** EN: Typed spawn operation result / ES: Resultado tipado de operacion de spawn */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	EPGXSpawnResultCode Code = EPGXSpawnResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	EPGXSpawnRequestStatus Status = EPGXSpawnRequestStatus::None;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	FPGXSpawnRequestHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	TObjectPtr<AActor> SpawnedActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	FString Message;

	static FPGXSpawnResult Success(FPGXSpawnRequestHandle InHandle, EPGXSpawnRequestStatus InStatus, AActor* InSpawnedActor, FString InMessage = FString());
	static FPGXSpawnResult Failure(EPGXSpawnResultCode InCode, EPGXSpawnRequestStatus InStatus, FString InMessage, FPGXSpawnRequestHandle InHandle = FPGXSpawnRequestHandle());
};

// ========================================================================
// EN: condition implementation — Condition evaluator payloads
// ES: condition implementation — Payloads del evaluador de condiciones
// ========================================================================

/** EN: Payload for PGX.Spawn.Condition.PlayerDistance. Pass when distance from nearest player is within [MinDistance, MaxDistance]. / ES: Payload para PGX.Spawn.Condition.PlayerDistance. Pasa cuando la distancia del jugador mas cercano esta en [MinDistance, MaxDistance]. */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnPlayerDistancePayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (ClampMin = "0.0"))
	float MinDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (ClampMin = "0.0"))
	float MaxDistance = 10000.0f;
};

/** EN: Payload for PGX.Spawn.Condition.MaxConcurrent. Pass when active count for Request.SourceTag is <= Max. / ES: Payload para PGX.Spawn.Condition.MaxConcurrent. Pasa cuando el conteo activo para Request.SourceTag es <= Max. */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnMaxConcurrentPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (ClampMin = "0"))
	int32 Max = 50;
};

/** EN: Payload for PGX.Spawn.Condition.TimeOfDay. Pass when current hour is within [MinHour, MaxHour]. / ES: Payload para PGX.Spawn.Condition.TimeOfDay. Pasa cuando la hora actual esta en [MinHour, MaxHour]. */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnTimeOfDayPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float MinHour = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float MaxHour = 24.0f;
};

/** EN: Payload for PGX.Spawn.Condition.GameplayTag. Pass when Request.SourceTag matches RequiredTag. / ES: Payload para PGX.Spawn.Condition.GameplayTag. Pasa cuando Request.SourceTag coincide con RequiredTag. */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnGameplayTagPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (Categories = "PGX.Spawn.Source"))
	FGameplayTag RequiredTag;
};

/** EN: Generic condition definition. Dispatched by ConditionTag via FInstancedStruct payload. / ES: Definicion generica de condicion. Despachada por ConditionTag via payload FInstancedStruct. */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnConditionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (Categories = "PGX.Spawn.Condition"))
	FGameplayTag ConditionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (BaseStruct = "PGXSpawnPlayerDistancePayload / PGXSpawnMaxConcurrentPayload / PGXSpawnTimeOfDayPayload / PGXSpawnGameplayTagPayload"))
	FInstancedStruct Payload;
};

/** EN: Entry in a weighted-class list for wave spawn distribution. / ES: Entrada en una lista ponderada de clases para distribucion de spawn de oleada. */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnClassEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn")
	TSubclassOf<AActor> Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (ClampMin = "0"))
	int32 MinCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Spawn", meta = (ClampMin = "1"))
	int32 MaxCount = 1;
};

/** EN: Aggregate debug snapshot of spawn subsystem state / ES: Snapshot agregado de estado del subsistema spawn */
USTRUCT(BlueprintType)
struct PGXSPAWNRUNTIME_API FPGXSpawnDebugSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	int32 ActiveRecordCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	int32 TotalRecordCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	int32 PeakConcurrentActors = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	int32 ActiveWaveCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	int32 PooledActorCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	TArray<FPGXSpawnRecord> LastCleanedRecords;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Spawn")
	FString DiagnosticsText;
};
