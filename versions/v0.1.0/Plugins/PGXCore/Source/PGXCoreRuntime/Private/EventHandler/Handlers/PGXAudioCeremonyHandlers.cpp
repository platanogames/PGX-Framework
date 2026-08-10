// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "EventHandler/Handlers/PGXAudioCeremonyHandlers.h"
#include "EventHandler/PGXEventHandlerLog.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessage.h"

// ============================================================
// UPGXPauseAudioHandler
// ============================================================

EPGXEventResult UPGXPauseAudioHandler::Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& /*Payload*/)
{
	// EN: Broadcast a pause-audio request via MessageSubsystem — the Audio L2 plugin listens
	// ES: Emitir solicitud de pausa-audio via MessageSubsystem — el plugin L2 Audio escucha
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(GetOuter());
	if (!IsValid(MsgSub))
	{
		UE_LOG(LogPGXEventHandler, Warning, TEXT("PauseAudioHandler: MessageSubsystem not available"));
		return EPGXEventResult::Failed;
	}

	const FGameplayTag PauseTag = FGameplayTag::RequestGameplayTag(FName("PGX.Event.System.PauseAudio"));
	FPGXMessage Msg;
	Msg.MessageTag = PauseTag;
	Msg.Owner = Context.Instigator.IsValid() ? Context.Instigator.Get() : nullptr;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(PauseTag, Msg);

	UE_LOG(LogPGXEventHandler, Log, TEXT("PauseAudioHandler: Pause audio request broadcast"));
	return EPGXEventResult::Success;
}

// ============================================================
// UPGXResumeAudioHandler
// ============================================================

EPGXEventResult UPGXResumeAudioHandler::Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& /*Payload*/)
{
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(GetOuter());
	if (!IsValid(MsgSub))
	{
		UE_LOG(LogPGXEventHandler, Warning, TEXT("ResumeAudioHandler: MessageSubsystem not available"));
		return EPGXEventResult::Failed;
	}

	const FGameplayTag ResumeTag = FGameplayTag::RequestGameplayTag(FName("PGX.Event.System.ResumeAudio"));
	FPGXMessage Msg;
	Msg.MessageTag = ResumeTag;
	Msg.Owner = Context.Instigator.IsValid() ? Context.Instigator.Get() : nullptr;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(ResumeTag, Msg);

	UE_LOG(LogPGXEventHandler, Log, TEXT("ResumeAudioHandler: Resume audio request broadcast"));
	return EPGXEventResult::Success;
}
