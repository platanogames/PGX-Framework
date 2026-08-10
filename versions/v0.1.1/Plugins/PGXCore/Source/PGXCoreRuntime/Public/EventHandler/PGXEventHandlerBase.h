// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "PGXEventHandlerBase.generated.h"

/**
 * EN: Base class for all PGX Event Handlers.
 *     Handlers are UObject-based behavior units resolved by GameplayTag.
 *     Override Execute() and optionally CanExecute() to define behavior.
 *     Provides helper methods for payload access, messaging, and sub-handler delegation.
 * ES: Clase base para todos los PGX Event Handlers.
 *     Los handlers son unidades de comportamiento basadas en UObject resueltas por GameplayTag.
 *     Override Execute() y opcionalmente CanExecute() para definir comportamiento.
 *     Provee metodos helper para acceso a payload, mensajeria y delegacion a sub-handlers.
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class PGXCORERUNTIME_API UPGXEventHandlerBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * EN: Execute this handler with the given context and payload.
	 * ES: Ejecutar este handler con el contexto y payload dados.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|EventHandler")
	EPGXEventResult Execute(const FPGXEventContext& Context, const FInstancedStruct& Payload);
	virtual EPGXEventResult Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& Payload);

	/**
	 * EN: Check if this handler can execute in the given context.
	 * ES: Verificar si este handler puede ejecutar en el contexto dado.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|EventHandler")
	bool CanExecute(const FPGXEventContext& Context) const;
	virtual bool CanExecute_Implementation(const FPGXEventContext& Context) const;

	/**
	 * EN: Called when the handler is activated (cached/singleton).
	 * ES: Llamado cuando el handler es activado (cacheado/singleton).
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|EventHandler")
	void OnActivated(const FPGXEventContext& Context);
	virtual void OnActivated_Implementation(const FPGXEventContext& Context);

	/**
	 * EN: Called when the handler is deactivated (evicted from cache/shutdown).
	 * ES: Llamado cuando el handler es desactivado (evicto del cache/shutdown).
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PGX|EventHandler")
	void OnDeactivated();
	virtual void OnDeactivated_Implementation();

	// ============================================================
	// Utility Helpers
	// ============================================================

	/**
	 * EN: Extract a typed payload from FInstancedStruct.
	 * ES: Extraer un payload tipado de FInstancedStruct.
	 */
	template<typename T>
	const T* GetPayload(const FInstancedStruct& Payload) const
	{
		return Payload.GetPtr<T>();
	}

	/**
	 * EN: Broadcast a message through the PGX Message System.
	 * ES: Emitir un mensaje a traves del Sistema de Mensajes PGX.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Helpers")
	void BroadcastMessage(FGameplayTag Channel, UObject* Sender);

	/**
	 * EN: Execute a sub-handler through the EventHandler subsystem.
	 * ES: Ejecutar un sub-handler a traves del subsistema EventHandler.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Helpers")
	EPGXEventResult ExecuteSubHandler(FGameplayTag EventTag, const FPGXEventContext& Context, const FInstancedStruct& Payload);
};
