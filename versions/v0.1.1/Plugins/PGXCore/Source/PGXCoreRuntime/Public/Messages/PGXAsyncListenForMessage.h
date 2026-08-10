// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "GameplayTagContainer.h"
#include "Messages/PGXMessage.h"
#include "PGXAsyncListenForMessage.generated.h"

class UPGXMessageSubsystem;
class UPGXAsyncListenForMessage;

/**
 * EN: Async delegate — fires when a message is received.
 *     ProxyObject = the async action (use GetPayload to extract the struct).
 *     Channel = what happened. Sender = who did it (from FPGXMessage::Owner).
 *     Follows Lyra's 2-step pattern: delegate fires → call GetPayload for typed data.
 * ES: Delegate async — se dispara al recibir un mensaje.
 *     ProxyObject = la accion async (usar GetPayload para extraer el struct).
 *     Channel = que paso. Sender = quien lo hizo (de FPGXMessage::Owner).
 *     Sigue el patron de 2 pasos de Lyra: delegate se dispara → llamar GetPayload para datos tipados.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPGXAsyncMessageReceived,
	UPGXAsyncListenForMessage*, ProxyObject, FGameplayTag, Channel, UObject*, Sender);

/**
 * EN: Blueprint async action to listen for PGX messages.
 *     Wraps Lyra's GameplayMessageSubsystem pattern with PGX additions (Sender/Owner).
 *     Fires OnMessageReceived(ProxyObject, Channel, Sender) when a message arrives.
 *     Call GetPayload() inside the delegate to copy the typed struct payload.
 *     Auto-cancels when the owning actor is destroyed (S1 safe).
 *     Uses UCancellableAsyncAction (NOT UBlueprintAsyncActionBase — UE5 gotcha #9).
 * ES: Accion async Blueprint para escuchar mensajes PGX.
 *     Envuelve el patron GameplayMessageSubsystem de Lyra con adiciones PGX (Sender/Owner).
 *     Dispara OnMessageReceived(ProxyObject, Channel, Sender) cuando un mensaje llega.
 *     Llamar GetPayload() dentro del delegate para copiar el struct tipado.
 *     Se auto-cancela cuando el actor propietario se destruye (S1 safe).
 *     Usa UCancellableAsyncAction (NO UBlueprintAsyncActionBase — gotcha UE5 #9).
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncAction", HasDedicatedAsyncNode))
class PGXCORERUNTIME_API UPGXAsyncListenForMessage : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * EN: Listen for PGX messages on a channel.
	 *     PayloadType filters by struct type — leave empty to accept all.
	 *     On receive: delegate fires with Channel + Sender, then call GetPayload for the struct data.
	 * ES: Escuchar mensajes PGX en un canal.
	 *     PayloadType filtra por tipo de struct — dejar vacio para aceptar todos.
	 *     Al recibir: el delegate se dispara con Channel + Sender, luego llamar GetPayload para los datos.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Messages",
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true",
			DisplayName = "Listen For PGX Messages"))
	static UPGXAsyncListenForMessage* ListenForPGXMessages(
		UObject* WorldContextObject,
		FGameplayTag Channel,
		UScriptStruct* PayloadType,
		EPGXMessageMatch MatchType = EPGXMessageMatch::ExactMatch);

	/**
	 * EN: Copy the received message payload into the specified wildcard output.
	 *     Must be called inside the OnMessageReceived delegate — the payload is only valid during the broadcast.
	 *     The wildcard type must match the PayloadType specified when creating the listener.
	 * ES: Copiar el payload del mensaje recibido al output wildcard especificado.
	 *     Debe llamarse dentro del delegate OnMessageReceived — el payload solo es valido durante el broadcast.
	 *     El tipo wildcard debe coincidir con el PayloadType especificado al crear el listener.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "PGX|Messages",
		meta = (CustomStructureParam = "OutPayload"))
	bool GetPayload(UPARAM(ref) int32& OutPayload);
	DECLARE_FUNCTION(execGetPayload);

	void Activate() override;
	void SetReadyToDestroy() override;

	/** EN: Called when a message is received / ES: Se llama al recibir un mensaje */
	UPROPERTY(BlueprintAssignable)
	FOnPGXAsyncMessageReceived OnMessageReceived;

private:
	void HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload);

	// EN: Temporarily holds the raw payload during broadcast (only valid inside delegate callback)
	// ES: Mantiene temporalmente el payload crudo durante broadcast (solo valido dentro del callback del delegate)
	const void* ReceivedMessagePayloadPtr = nullptr;

	TWeakObjectPtr<UWorld> WorldPtr;
	FGameplayTag ChannelToRegister;
	TWeakObjectPtr<UScriptStruct> MessageStructType = nullptr;
	EPGXMessageMatch MessageMatchType = EPGXMessageMatch::ExactMatch;
	FPGXMessageListenerHandle ListenerHandle;
};
