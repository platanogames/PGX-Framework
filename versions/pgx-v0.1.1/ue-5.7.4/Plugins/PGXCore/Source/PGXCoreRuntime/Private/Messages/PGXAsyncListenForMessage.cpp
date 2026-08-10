// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Messages/PGXAsyncListenForMessage.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessageLog.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/Stack.h"

UPGXAsyncListenForMessage* UPGXAsyncListenForMessage::ListenForPGXMessages(
	UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType, EPGXMessageMatch MatchType)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World))
	{
		return nullptr;
	}

	UPGXAsyncListenForMessage* Action = NewObject<UPGXAsyncListenForMessage>();
	Action->WorldPtr = World;
	Action->ChannelToRegister = Channel;
	Action->MessageStructType = PayloadType;
	Action->MessageMatchType = MatchType;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UPGXAsyncListenForMessage::Activate()
{
	UWorld* World = WorldPtr.Get();
	if (!IsValid(World))
	{
		UE_LOG(LogPGXMessage, Warning, TEXT("ListenForPGXMessages: Activate failed — World is invalid"));
		SetReadyToDestroy();
		return;
	}

	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(World);
	if (!IsValid(MsgSub))
	{
		UE_LOG(LogPGXMessage, Warning, TEXT("ListenForPGXMessages: Activate failed — Subsystem not found"));
		SetReadyToDestroy();
		return;
	}

	TWeakObjectPtr<UPGXAsyncListenForMessage> WeakThis(this);
	ListenerHandle = MsgSub->RegisterListenerInternal(
		ChannelToRegister,
		[WeakThis](FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload)
		{
			if (UPGXAsyncListenForMessage* StrongThis = WeakThis.Get())
			{
				StrongThis->HandleMessageReceived(Channel, StructType, Payload);
			}
		},
		MessageStructType.Get(),
		MessageMatchType);

	UE_LOG(LogPGXMessage, Log, TEXT("ListenForPGXMessages: Activated on Channel=%s PayloadType=%s HandleID=%d"),
		*ChannelToRegister.ToString(),
		MessageStructType.IsValid() ? *MessageStructType->GetName() : TEXT("Any"),
		ListenerHandle.GetID());
}

void UPGXAsyncListenForMessage::SetReadyToDestroy()
{
	ListenerHandle.Unregister();
	Super::SetReadyToDestroy();
}

bool UPGXAsyncListenForMessage::GetPayload(int32& /*OutPayload*/)
{
	// EN: Never called directly — the CustomThunk exec version below handles Blueprint calls
	// ES: Nunca se llama directamente — la version exec CustomThunk de abajo maneja llamadas Blueprint
	checkNoEntry();
	return false;
}

DEFINE_FUNCTION(UPGXAsyncListenForMessage::execGetPayload)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;

	bool bSuccess = false;

	// EN: Copy payload if types match — same pattern as Lyra's execGetPayload
	// ES: Copiar payload si los tipos coinciden — mismo patron que execGetPayload de Lyra
	if ((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr)
		&& (StructProp->Struct == P_THIS->MessageStructType.Get())
		&& (P_THIS->ReceivedMessagePayloadPtr != nullptr))
	{
		StructProp->Struct->CopyScriptStruct(MessagePtr, P_THIS->ReceivedMessagePayloadPtr);
		bSuccess = true;
	}

	*(bool*)RESULT_PARAM = bSuccess;
}

void UPGXAsyncListenForMessage::HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload)
{
	UE_LOG(LogPGXMessage, Log, TEXT("HandleMessageReceived: Channel=%s BroadcastType=%s ListenerType=%s"),
		*Channel.ToString(),
		StructType ? *StructType->GetName() : TEXT("null"),
		MessageStructType.IsValid() ? *MessageStructType->GetName() : TEXT("Any"));

	// EN: Fire when filter is unset (accept all) OR broadcast struct is compatible with the
	//     listener's filter type — i.e., broadcast struct is the same as or a child of the
	//     filter. Aligns with the C++ template path (BroadcastMessageInternal IsChildOf check),
	//     enabling payload backward compatibility (older listener receives newer child
	//     broadcast without crash). GetPayload remains strict: a wildcard type
	//     not the expected type" → failure).
	// ES: Disparar cuando el filtro esta vacio (aceptar todo) O cuando el struct del broadcast
	//     es compatible con el tipo de filtro del listener — es decir, el broadcast es el mismo
	//     o un hijo del filtro. Alinea con el path template C++ (BroadcastMessageInternal IsChildOf),
	//     habilitando payload backward compatibility (listener antiguo recibe broadcast
	//     hijo nuevo sin crash). GetPayload permanece estricto per the strict payload type policy.
	if (!MessageStructType.Get() || (StructType && StructType->IsChildOf(MessageStructType.Get())))
	{
		// EN: Store payload temporarily for GetPayload() — only valid during this broadcast
		// ES: Almacenar payload temporalmente para GetPayload() — solo valido durante este broadcast
		ReceivedMessagePayloadPtr = Payload;

		// EN: Extract Sender from FPGXMessage base if the payload inherits from it (PGX addition over Lyra)
		// ES: Extraer Sender de FPGXMessage base si el payload hereda de el (adicion PGX sobre Lyra)
		UObject* Sender = nullptr;
		if (StructType && Payload && StructType->IsChildOf(FPGXMessage::StaticStruct()))
		{
			const FPGXMessage* BaseMsg = reinterpret_cast<const FPGXMessage*>(Payload);
			Sender = BaseMsg->Owner.Get();
		}

		OnMessageReceived.Broadcast(this, Channel, Sender);

		// EN: Clear after broadcast — payload pointer may be stack-allocated by the broadcaster
		// ES: Limpiar despues del broadcast — el puntero del payload puede ser stack-allocated por el broadcaster
		ReceivedMessagePayloadPtr = nullptr;
	}

	if (!OnMessageReceived.IsBound())
	{
		SetReadyToDestroy();
	}
}
