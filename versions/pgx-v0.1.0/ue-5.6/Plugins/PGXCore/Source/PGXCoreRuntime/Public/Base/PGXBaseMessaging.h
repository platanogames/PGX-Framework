// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessage.h"

/**
 * EN: Shared template utilities for PGX base class message integration.
 *     All base classes delegate to these functions for broadcast/listen/cleanup.
 * ES: Utilidades template compartidas para integracion de mensajes en clases base PGX.
 *     Todas las clases base delegan a estas funciones para broadcast/listen/cleanup.
 */
namespace PGXBaseMessaging
{
	/**
	 * EN: Broadcast a typed message on the given channel.
	 *     No-op if Message subsystem is not available (graceful degradation).
	 * ES: Publicar un mensaje tipado en el canal dado.
	 *     No-op si el subsistema Message no esta disponible (degradacion elegante).
	 */
	template<typename T>
	void Broadcast(const UObject* WorldContext, FGameplayTag Channel, const T& Payload)
	{
		UPGXMessageSubsystem* MessageSub = UPGXMessageSubsystem::Get(WorldContext);
		if (!MessageSub) { return; }
		MessageSub->BroadcastMessage<T>(Channel, Payload);
	}

	/**
	 * EN: Register a listener for a typed message on the given channel.
	 *     Returns a handle that must be stored and unregistered in EndPlay/Shutdown.
	 * ES: Registrar un listener para un mensaje tipado en el canal dado.
	 *     Devuelve un handle que debe almacenarse y desregistrarse en EndPlay/Shutdown.
	 */
	template<typename T>
	FPGXMessageListenerHandle Listen(const UObject* WorldContext, FGameplayTag Channel,
		TFunction<void(FGameplayTag, const T&)> Callback)
	{
		UPGXMessageSubsystem* MessageSub = UPGXMessageSubsystem::Get(WorldContext);
		if (!MessageSub) { return FPGXMessageListenerHandle(); }
		return MessageSub->RegisterListener<T>(Channel, MoveTemp(Callback));
	}

	/**
	 * EN: Unregister all stored listener handles. Call in EndPlay/Shutdown for S1 compliance.
	 * ES: Desregistrar todos los handles almacenados. Llamar en EndPlay/Shutdown para cumplir S1.
	 */
	inline void UnregisterAll(TArray<FPGXMessageListenerHandle>& Handles)
	{
		for (FPGXMessageListenerHandle& Handle : Handles)
		{
			Handle.Unregister();
		}
		Handles.Empty();
	}
}
