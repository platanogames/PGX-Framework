// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "Engine/DataTable.h"
#include "PGXEventHandlerTypes.generated.h"

class UPGXEventHandlerBase;

/**
 * EN: Result of an event handler execution.
 * ES: Resultado de la ejecucion de un event handler.
 */
UENUM(BlueprintType)
enum class EPGXEventResult : uint8
{
	/** EN: Handler executed successfully / ES: Handler ejecuto exitosamente */
	Success,
	/** EN: Handler executed but conditions not fully met / ES: Handler ejecuto pero condiciones no plenamente cumplidas */
	Partial,
	/** EN: Handler execution failed / ES: Ejecucion del handler fallo */
	Failed,
	/** EN: Handler was skipped (conditions not met) / ES: Handler fue omitido (condiciones no cumplidas) */
	Skipped,
	/** EN: No handler found for the given tag / ES: No se encontro handler para el tag dado */
	NotFound,
	/** EN: Handler execution was blocked (recursion, budget) / ES: Ejecucion bloqueada (recursion, presupuesto) */
	Blocked
};

/**
 * EN: Lifecycle mode for handler instances.
 * ES: Modo de ciclo de vida para instancias de handlers.
 */
UENUM(BlueprintType)
enum class EPGXHandlerLifecycle : uint8
{
	/** EN: Created once, persists for the lifetime of the subsystem / ES: Creado una vez, persiste toda la vida del subsistema */
	Singleton,
	/** EN: Pooled with LRU eviction / ES: Pooled con eviccion LRU */
	Cached,
	/** EN: Created and destroyed per execution / ES: Creado y destruido por ejecucion */
	Ephemeral
};

/**
 * EN: Context passed to event handlers during execution.
 *     Slim struct (3 fields): Instigator, Target, SourceTags.
 *     TriggerTag and WorldContext were removed — TriggerTag is redundant with the EventTag parameter,
 *     and WorldContext is auto-resolved by UE meta=(WorldContext).
 * ES: Contexto pasado a los event handlers durante ejecucion.
 *     Struct slim (3 campos): Instigator, Target, SourceTags.
 *     TriggerTag y WorldContext fueron removidos — TriggerTag es redundante con el parametro EventTag,
 *     y WorldContext se auto-resuelve via UE meta=(WorldContext).
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXEventContext
{
	GENERATED_BODY()

	/** EN: Object that triggered the event / ES: Objeto que disparo el evento */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|EventHandler")
	TWeakObjectPtr<UObject> Instigator;

	/** EN: Target object of the event / ES: Objeto objetivo del evento */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|EventHandler")
	TWeakObjectPtr<UObject> Target;

	/** EN: Additional tags for filtering / ES: Tags adicionales para filtrado */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|EventHandler")
	FGameplayTagContainer SourceTags;
};

/**
 * EN: DataTable row for handler registration.
 * ES: Fila de DataTable para registro de handlers.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXEventHandlerRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Tag that this handler responds to / ES: Tag al que responde este handler */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler")
	FGameplayTag EventTag;

	/** EN: Class of the handler to instantiate / ES: Clase del handler a instanciar */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler")
	TSoftClassPtr<UPGXEventHandlerBase> HandlerClass;

	/** EN: Lifecycle management mode / ES: Modo de gestion del ciclo de vida */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler")
	EPGXHandlerLifecycle Lifecycle = EPGXHandlerLifecycle::Cached;

	/** EN: Category tag for budgeting and grouping / ES: Tag de categoria para presupuesto y agrupamiento */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler")
	FGameplayTag CategoryTag;

	/** EN: Human-readable description / ES: Descripcion legible para humanos */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler")
	FString Description;

	/** EN: Expected payload struct type name (for validation) / ES: Nombre del tipo struct del payload esperado */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler")
	FString ExpectedPayloadType;

	/** EN: Whether this handler is enabled / ES: Si este handler esta habilitado */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler")
	bool bEnabled = true;
};

/**
 * EN: Read-only view of a registered handler.
 * ES: Vista de solo lectura de un handler registrado.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXEventHandlerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly)
	FString HandlerClassName;

	UPROPERTY(BlueprintReadOnly)
	EPGXHandlerLifecycle Lifecycle = EPGXHandlerLifecycle::Cached;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag CategoryTag;

	UPROPERTY(BlueprintReadOnly)
	FString Description;

	UPROPERTY(BlueprintReadOnly)
	bool bEnabled = true;

	UPROPERTY(BlueprintReadOnly)
	bool bCached = false;
};

/**
 * EN: Cache statistics for the event handler subsystem.
 * ES: Estadisticas de cache para el subsistema de event handler.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXHandlerCacheStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 CachedHandlers = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxCachedHandlers = 128;

	UPROPERTY(BlueprintReadOnly)
	int32 CacheHits = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CacheMisses = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Evictions = 0;
};

/**
 * EN: Per-handler telemetry data.
 * ES: Datos de telemetria por handler.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXHandlerTelemetry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly)
	int32 ExecutionCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 SuccessCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 FailCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 SkipCount = 0;

	UPROPERTY(BlueprintReadOnly)
	double TotalExecutionTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double AvgExecutionTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double MaxExecutionTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double LastExecutionTimestamp = 0.0;
};

/**
 * EN: Per-category memory budget configuration.
 * ES: Configuracion de presupuesto de memoria por categoria.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXCategoryBudget
{
	GENERATED_BODY()

	/** EN: Category tag / ES: Tag de categoria */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler")
	FGameplayTag CategoryTag;

	/** EN: Maximum number of cached handlers for this category / ES: Maximo de handlers cacheados para esta categoria */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler",
		meta = (ClampMin = "1", ClampMax = "256"))
	int32 MaxCachedHandlers = 32;
};

/**
 * EN: Blackbox entry for execution history tracking.
 * ES: Entrada de blackbox para tracking del historial de ejecucion.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXBlackboxEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	double Timestamp = 0.0;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly)
	EPGXEventResult Result = EPGXEventResult::NotFound;

	UPROPERTY(BlueprintReadOnly)
	FString HandlerClassName;

	UPROPERTY(BlueprintReadOnly)
	double ExecutionTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	FString InstigatorName;

	/** EN: Target object name snapshot / ES: Snapshot del nombre del target */
	UPROPERTY(BlueprintReadOnly)
	FString TargetName;

	/** EN: Payload struct type used for this execution / ES: Tipo de struct del payload usado en esta ejecucion */
	UPROPERTY(BlueprintReadOnly)
	FString PayloadTypeName;

	/** EN: Failure/block reason for typed diagnostics / ES: Razon de fallo/bloqueo para diagnostico tipado */
	UPROPERTY(BlueprintReadOnly)
	FString FailureReason;

	/** EN: Canonical failure/block reason tag when available / ES: Tag canonico de razon si existe */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag FailureReasonTag;

	/** EN: Depth in the handler execution chain / ES: Profundidad en la cadena de ejecucion de handlers */
	UPROPERTY(BlueprintReadOnly)
	int32 ChainDepth = 0;
};
