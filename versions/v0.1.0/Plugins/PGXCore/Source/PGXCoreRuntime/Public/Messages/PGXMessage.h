// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "PGXMessage.generated.h"

class UPGXMessageSubsystem;

/**
 * EN: Match rule for message listeners (reimplemented from Lyra pattern).
 * ES: Regla de coincidencia para listeners de mensajes (reimplementado del patron Lyra).
 */
UENUM(BlueprintType)
enum class EPGXMessageMatch : uint8
{
	/** EN: Exact match only (A.B matches A.B but not A.B.C) / ES: Coincidencia exacta */
	ExactMatch,
	/** EN: Partial match (A.B matches A.B and A.B.C) / ES: Coincidencia parcial */
	PartialMatch
};

/**
 * EN: Base message struct for the PGX Message System.
 *     Extends Lyra's message concept with owner, timestamp, and scope.
 * ES: Struct de mensaje base para el Sistema de Mensajes PGX.
 *     Extiende el concepto de mensaje de Lyra con owner, timestamp y scope.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXMessage
{
	GENERATED_BODY()

	/** EN: Tag identifying the message channel / ES: Tag que identifica el canal del mensaje */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag MessageTag;

	/** EN: Object that sent the message / ES: Objeto que envio el mensaje */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> Owner;

	/** EN: Timestamp when the message was broadcast / ES: Timestamp de cuando se emitio el mensaje */
	UPROPERTY(BlueprintReadOnly)
	double Timestamp = 0.0;
};

/**
 * EN: Filter for listening to specific messages.
 * ES: Filtro para escuchar mensajes especificos.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXMessageFilter
{
	GENERATED_BODY()

	/** EN: Tag required for the message to pass the filter / ES: Tag requerido para que el mensaje pase el filtro */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag RequiredTag;

	/** EN: Owner required for the message to pass the filter / ES: Owner requerido para que el mensaje pase el filtro */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UObject> RequiredOwner;
};

/**
 * EN: Opaque handle for managing message listener registration.
 *     Can be used to unregister via Unregister() or via the subsystem.
 * ES: Handle opaco para gestionar el registro de listeners de mensajes.
 *     Se puede usar para desregistrar via Unregister() o via el subsistema.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXMessageListenerHandle
{
	GENERATED_BODY()

	FPGXMessageListenerHandle() {}

	/** EN: Unregister this listener / ES: Desregistrar este listener */
	void Unregister();

	/** EN: Returns true if the handle is valid / ES: Retorna true si el handle es valido */
	bool IsValid() const { return ID != 0; }

	/** EN: Get the listener ID (for debug logging) / ES: Obtener el ID del listener (para logging de debug) */
	int32 GetID() const { return ID; }

private:
	friend UPGXMessageSubsystem;

	// EN: Stored as UObject to avoid TWeakObjectPtr requiring complete type
	// ES: Almacenado como UObject para evitar que TWeakObjectPtr requiera tipo completo
	UPROPERTY(Transient)
	TWeakObjectPtr<UObject> Subsystem;

	UPROPERTY(Transient)
	FGameplayTag Channel;

	UPROPERTY(Transient)
	int32 ID = 0;

	FPGXMessageListenerHandle(UPGXMessageSubsystem* InSubsystem, FGameplayTag InChannel, int32 InID);
};

/**
 * EN: Internal data for a single registered listener.
 * ES: Datos internos para un listener registrado.
 */
USTRUCT()
struct FPGXMessageListenerData
{
	GENERATED_BODY()

	/** EN: Callback for message receipt / ES: Callback al recibir mensaje */
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)> ReceivedCallback;

	int32 HandleID = 0;
	EPGXMessageMatch MatchType = EPGXMessageMatch::ExactMatch;

	TWeakObjectPtr<const UScriptStruct> ListenerStructType = nullptr;
	bool bHadValidType = false;
};

/**
 * EN: Record of a broadcast message for history tracking.
 * ES: Registro de un mensaje broadcast para historial.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXMessageRecord
{
	GENERATED_BODY()

	/** EN: Channel the message was broadcast on / ES: Canal donde se emitio el mensaje */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Channel;

	/** EN: Struct type name of the payload / ES: Nombre del tipo struct del payload */
	UPROPERTY(BlueprintReadOnly)
	FString PayloadTypeName;

	/** EN: World time when broadcast occurred / ES: Tiempo de mundo cuando ocurrio el broadcast */
	UPROPERTY(BlueprintReadOnly)
	double Timestamp = 0.0;

	/** EN: Number of listeners that received this message / ES: Numero de listeners que recibieron este mensaje */
	UPROPERTY(BlueprintReadOnly)
	int32 ListenersNotified = 0;

	/**
	 * EN: True when this record originates from a test/debug broadcast (console cmd, test utility,
	 *     inspector test broadcast). Inspector/diagnostics filter by this flag to keep production
	 *     telemetry clean. Default false for production broadcasts.
	 * ES: True cuando el record proviene de un broadcast test/debug (cmd consola, test utility,
	 *     inspector test broadcast). Inspector/diagnostics filtran por esta flag para mantener la
	 *     telemetria de produccion limpia. Default false para broadcasts de produccion.
	 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsTestOrigin = false;
};

/**
 * EN: Aggregate statistics for the message subsystem.
 * ES: Estadisticas agregadas del subsistema de mensajes.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXMessageStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 TotalBroadcasts = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalListenersNotified = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ActiveChannels = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ActiveListeners = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 HistorySize = 0;

	/**
	 * EN: Maximum listeners notified by any single BroadcastMessageInternal call. Inspector
	 *     surfaces this as a fan-out diagnostic (channels with high counts indicate broad
	 *     partial-match listeners or hot pub/sub paths).
	 * ES: Maximo de listeners notificados por una sola llamada BroadcastMessageInternal.
	 *     Inspector lo expone como diagnostico de fan-out.
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxFanOutOnSingleBroadcast = 0;

	/**
	 * EN: Maximum broadcast nesting depth observed during dispatch. With deferred-dispatch
	 *     dispatch this normally stays at 1; values > 1 indicate DispatchOne was reached
	 *     through a non-orchestrator path. Hard cap is Config.MaxBroadcastRecursionDepth (N):
	 *     dispatches at depths 1..N proceed; the (N+1)th nesting attempt trips the guard.
	 *     Inspector surfaces this so cascade chains are visible.
	 * ES: Maxima profundidad de anidamiento observada durante dispatch. Con deferred-dispatch
	 *     normalmente queda en 1; valores > 1 indican un path no-orquestador.
	 *     El cap es Config.MaxBroadcastRecursionDepth (N): depths 1..N ejecutan;
	 *     el intento (N+1) trips el guard.
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxBroadcastDepth = 0;

	/**
	 * EN: Maximum size observed in PendingBroadcasts queue (broadcasts deferred while a parent
	 *     dispatch was in flight under same-frame deferred-dispatch ordering). Inspector surfaces
	 *     this so cascade burst patterns are visible. Queue cap per Config.MaxBroadcastQueueDepth.
	 * ES: Maximo tamano observado en la cola PendingBroadcasts (broadcasts diferidos durante un
	 *     dispatch padre en curso bajo el orden deferred-dispatch del mismo frame).
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxQueueDepth = 0;
};

/**
 * EN: A broadcast queued during a parent dispatch — replayed AFTER the parent fan-out completes
 *     to honor same-frame deferred-dispatch ordering invariant ("parent fan-out completes before
 *     nested broadcasts begin"). FInstancedStruct owns a copy of the payload bytes so the
 *     queue is decoupled from the original caller's stack lifetime.
 * ES: Un broadcast encolado durante un dispatch padre — replayed TRAS completar el fan-out
 *     padre para respetar el orden deferred-dispatch del mismo frame.
 */
USTRUCT()
struct PGXCORERUNTIME_API FPGXPendingBroadcast
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag Channel;

	UPROPERTY()
	FInstancedStruct Payload;

	UPROPERTY()
	bool bIsTestOrigin = false;
};
