// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/Handlers/PGXAutoSaveHandler.h"
#include "EventHandler/PGXEventHandlerLog.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessage.h"

EPGXEventResult UPGXAutoSaveHandler::Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& /*Payload*/)
{
	// EN: Broadcast a save request via MessageSubsystem — the Save L2 plugin listens for this tag
	// ES: Emitir solicitud de guardado via MessageSubsystem — el plugin L2 Save escucha este tag
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(GetOuter());
	if (!IsValid(MsgSub))
	{
		UE_LOG(LogPGXEventHandler, Warning, TEXT("AutoSaveHandler: MessageSubsystem not available"));
		return EPGXEventResult::Failed;
	}

	const FGameplayTag SaveTag = FGameplayTag::RequestGameplayTag(FName("PGX.Event.System.AutoSave"));
	FPGXMessage Msg;
	Msg.MessageTag = SaveTag;
	Msg.Owner = Context.Instigator.IsValid() ? Context.Instigator.Get() : nullptr;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(SaveTag, Msg);

	UE_LOG(LogPGXEventHandler, Log, TEXT("AutoSaveHandler: Auto-save request broadcast"));
	return EPGXEventResult::Success;
}
