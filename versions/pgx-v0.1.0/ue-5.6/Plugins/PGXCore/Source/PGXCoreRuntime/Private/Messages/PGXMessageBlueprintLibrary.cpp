// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Messages/PGXMessageBlueprintLibrary.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessageLog.h"

void UPGXMessageBlueprintLibrary::BroadcastPGXMessage(const UObject* WorldContextObject, FGameplayTag Channel, UObject* Sender)
{
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!MsgSub)
	{
		UE_LOG(LogPGXMessage, Warning, TEXT("BroadcastPGXMessage: Subsystem not found (WorldContext=%s, Channel=%s)"),
			*GetNameSafe(WorldContextObject), *Channel.ToString());
		return;
	}

	FPGXMessage Msg;
	Msg.MessageTag = Channel;
	Msg.Owner = Sender;
	Msg.Timestamp = FPlatformTime::Seconds();

	UE_LOG(LogPGXMessage, Log, TEXT("BroadcastPGXMessage: Channel=%s Sender=%s Listeners=%d"),
		*Channel.ToString(), *GetNameSafe(Sender), MsgSub->GetListenerCount(Channel));

	MsgSub->BroadcastMessage<FPGXMessage>(Channel, Msg);
}

bool UPGXMessageBlueprintLibrary::IsChannelActive(const UObject* WorldContextObject, FGameplayTag Channel)
{
	if (const UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject))
	{
		return MsgSub->IsChannelActive(Channel);
	}
	return false;
}

int32 UPGXMessageBlueprintLibrary::GetChannelListenerCount(const UObject* WorldContextObject, FGameplayTag Channel)
{
	if (const UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject))
	{
		return MsgSub->GetListenerCount(Channel);
	}
	return 0;
}

TArray<FGameplayTag> UPGXMessageBlueprintLibrary::GetActiveMessageChannels(const UObject* WorldContextObject)
{
	if (const UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject))
	{
		return MsgSub->GetAllActiveChannels();
	}
	return TArray<FGameplayTag>();
}

FPGXMessageStats UPGXMessageBlueprintLibrary::GetMessageSystemStats(const UObject* WorldContextObject)
{
	if (const UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject))
	{
		return MsgSub->GetStats();
	}
	return FPGXMessageStats();
}
