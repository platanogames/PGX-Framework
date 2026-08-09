// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "PGXMessageConfig.generated.h"

/**
 * EN: Configuration DataAsset for the PGX Message System.
 *     Auto-discovered via AssetRegistry during subsystem initialization.
 * ES: DataAsset de configuracion para el Sistema de Mensajes PGX.
 *     Auto-descubierto via AssetRegistry durante inicializacion del subsistema.
 */
UCLASS(BlueprintType)
class PGXCORERUNTIME_API UPGXMessageConfig : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:
	UPGXMessageConfig();

	/** EN: Maximum number of messages kept in history / ES: Maximo de mensajes en el historial */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Message",
		meta = (ClampMin = "16", ClampMax = "1024"))
	int32 MaxMessageHistory = 100;

	/** EN: Log every broadcast to the output log / ES: Registrar cada broadcast en el log de salida */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Message")
	bool bLogBroadcasts = false;

	/** EN: Log listener registrations and unregistrations / ES: Registrar registros y desregistros de listeners */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Message")
	bool bLogRegistrations = false;

	/** EN: Enable partial tag matching (parent tags receive child broadcasts) / ES: Habilitar coincidencia parcial de tags */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Message")
	bool bEnablePartialMatching = true;

	/**
	 * EN: Allow developer/editor test broadcasts via the `pgx.message.broadcast` console command
	 *     in development builds. The command is also gated at compile time, so
	 *     this flag is only consulted when the command is registered. Default true in dev builds;
	 *     projects can set false to fully disable test broadcasts even in editor.
	 * ES: Permitir test broadcasts de developer/editor via el comando de consola `pgx.message.broadcast`
	 *     en builds de desarrollo. El comando tambien esta limitado en compile-time,
	 *     asi que esta flag solo se consulta cuando el comando esta registrado. Default true en builds
	 *     dev; los proyectos pueden setear false para deshabilitar test broadcasts incluso en editor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Message|Diagnostics")
	bool bAllowTestBroadcasts = true;

	/**
	 * EN: Maximum nested broadcast levels permitted. With this set to N, dispatches at depths
	 *     1..N execute normally; the (N+1)th nesting attempt is rejected (the guard trips at
	 *     entry when current depth == N, before incrementing to N+1). Note: with the
	 *     deferred-dispatch queue, nested broadcasts queue rather than
	 *     recurse, so dispatch depth normally stays at 1 — the guard remains as a sanity
	 *     backstop when DispatchOne is invoked directly.
	 *     Default 4 covers typical request->response->ack->finalize chains.
	 *     This also keeps recursion and fan-out diagnostics bounded.
	 * ES: Maximo de niveles anidados permitidos. Con N, los dispatches en depths 1..N ejecutan
	 *     normalmente; el intento (N+1) es rechazado (el guard salta cuando current depth == N,
	 *     antes de incrementar a N+1). Con la cola deferred-dispatch, los broadcasts
	 *     anidados encolan en vez de recursar, asi que la profundidad normalmente queda en 1.
	 *     Default 4.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Message|Diagnostics",
		meta = (ClampMin = "1", ClampMax = "32"))
	int32 MaxBroadcastRecursionDepth = 4;

	/**
	 * EN: Maximum number of broadcasts that can be queued during a parent dispatch.
	 *     When the queue reaches this cap, additional nested
	 *     broadcasts are dropped with a LogError to prevent unbounded growth. Default 64 covers
	 *     typical cascade bursts (audit chain, save domain restore) while bounding worst-case
	 *     memory + drain time.
	 * ES: Maximo numero de broadcasts que pueden encolarse durante un dispatch padre. Cuando la
	 *     cola alcanza este limite, broadcasts nested adicionales se descartan con LogError para
	 *     prevenir crecimiento sin limite. Default 64.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Message|Diagnostics",
		meta = (ClampMin = "1", ClampMax = "1024"))
	int32 MaxBroadcastQueueDepth = 64;
};
