// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Messages/PGXMessage.h"
#include "PGXMessageBlueprintLibrary.generated.h"

/**
 * EN: Blueprint function library for the PGX Message System.
 *     Sole BP entry point for message operations (per BLUEPRINT API CONVENTION).
 *     C++ users use UPGXMessageSubsystem template API directly.
 * ES: Libreria de funciones Blueprint para el Sistema de Mensajes PGX.
 *     Unico punto de entrada BP para operaciones de mensajes (segun BLUEPRINT API CONVENTION).
 *     Usuarios C++ usan la API template de UPGXMessageSubsystem directamente.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXMessageBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * EN: Broadcast a message on the specified channel.
	 *     Channel = what happened. Sender = who did it.
	 *     Both are visible in the Message Inspector history for full traceability.
	 * ES: Emitir un mensaje en el canal especificado.
	 *     Channel = que paso. Sender = quien lo hizo.
	 *     Ambos son visibles en el historial del Message Inspector para trazabilidad completa.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Broadcast PGX Message"))
	static void BroadcastPGXMessage(const UObject* WorldContextObject, FGameplayTag Channel, UObject* Sender);

	/** EN: Check if a message channel has active listeners / ES: Verificar si un canal tiene listeners activos */
	UFUNCTION(BlueprintPure, Category = "PGX|Messages|Query",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsChannelActive(const UObject* WorldContextObject, FGameplayTag Channel);

	/** EN: Get the number of listeners on a channel / ES: Obtener numero de listeners en un canal */
	UFUNCTION(BlueprintPure, Category = "PGX|Messages|Query",
		meta = (WorldContext = "WorldContextObject"))
	static int32 GetChannelListenerCount(const UObject* WorldContextObject, FGameplayTag Channel);

	/** EN: Get all active message channels / ES: Obtener todos los canales de mensaje activos */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages|Query",
		meta = (WorldContext = "WorldContextObject"))
	static TArray<FGameplayTag> GetActiveMessageChannels(const UObject* WorldContextObject);

	/** EN: Get message system stats / ES: Obtener estadisticas del sistema de mensajes */
	UFUNCTION(BlueprintPure, Category = "PGX|Messages|Query",
		meta = (WorldContext = "WorldContextObject"))
	static FPGXMessageStats GetMessageSystemStats(const UObject* WorldContextObject);
};
