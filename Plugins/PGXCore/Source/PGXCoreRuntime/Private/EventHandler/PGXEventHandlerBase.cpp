// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/PGXEventHandlerBase.h"
#include "EventHandler/PGXEventHandlerLog.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessage.h"

EPGXEventResult UPGXEventHandlerBase::Execute_Implementation(const FPGXEventContext& /*Context*/, const FInstancedStruct& /*Payload*/)
{
	// EN: Default implementation — override in subclasses
	// ES: Implementacion por defecto — override en subclases
	UE_LOG(LogPGXEventHandler, Warning, TEXT("Execute_Implementation not overridden in %s"), *GetClass()->GetName());
	return EPGXEventResult::Failed;
}

bool UPGXEventHandlerBase::CanExecute_Implementation(const FPGXEventContext& /*Context*/) const
{
	// EN: Default: always can execute / ES: Por defecto: siempre puede ejecutar
	return true;
}

void UPGXEventHandlerBase::OnActivated_Implementation(const FPGXEventContext& Context)
{
	// EN: Default: no-op / ES: Por defecto: no-op
}

void UPGXEventHandlerBase::OnDeactivated_Implementation()
{
	// EN: Default: no-op / ES: Por defecto: no-op
}

void UPGXEventHandlerBase::BroadcastMessage(FGameplayTag Channel, UObject* Sender)
{
	UObject* WorldCtx = GetOuter();
	if (UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldCtx))
	{
		FPGXMessage Msg;
		Msg.MessageTag = Channel;
		Msg.Owner = Sender;
		Msg.Timestamp = FPlatformTime::Seconds();
		MsgSub->BroadcastMessage<FPGXMessage>(Channel, Msg);
	}
}

EPGXEventResult UPGXEventHandlerBase::ExecuteSubHandler(FGameplayTag EventTag, const FPGXEventContext& Context, const FInstancedStruct& Payload)
{
	// EN: Delegate to the subsystem for recursive handler execution
	// ES: Delegar al subsistema para ejecucion recursiva de handlers
	UPGXEventHandlerSubsystem* Sub = UPGXEventHandlerSubsystem::Get(GetOuter());
	if (!IsValid(Sub))
	{
		UE_LOG(LogPGXEventHandler, Warning, TEXT("ExecuteSubHandler: EventHandler subsystem not available"));
		return EPGXEventResult::NotFound;
	}

	return Sub->ResolveAndExecuteWithContext(EventTag, Context, Payload);
}
