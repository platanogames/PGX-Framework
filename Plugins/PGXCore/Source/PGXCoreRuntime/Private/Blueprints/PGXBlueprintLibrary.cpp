// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Blueprints/PGXBlueprintLibrary.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessage.h"

// EN: PGX Blueprint function library implementation
// ES: Implementacion de la libreria de funciones Blueprint PGX

bool UPGXBlueprintLibrary::IsPGXReady(const UObject* /*WorldContextObject*/)
{
	// EN: Stub - will check if PGXSubsystemManager reports Ready state
	// ES: Stub - verificara si PGXSubsystemManager reporta estado Ready
	return false;
}

void UPGXBlueprintLibrary::SendPGXMessage(const UObject* WorldContextObject, FGameplayTag MessageTag, UObject* Payload)
{
	// EN: Forward to UPGXMessageSubsystem / ES: Reenviar a UPGXMessageSubsystem
	if (UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject))
	{
		FPGXMessage Msg;
		Msg.MessageTag = MessageTag;
		Msg.Owner = Payload;
		Msg.Timestamp = FPlatformTime::Seconds();
		MsgSub->BroadcastMessage<FPGXMessage>(MessageTag, Msg);
	}
}

UObject* UPGXBlueprintLibrary::GetPGXData(const UObject* /*WorldContextObject*/, FName /*DataId*/)
{
	// EN: Stub - will query UPGXDataRegistrySubsystem
	// ES: Stub - consultara UPGXDataRegistrySubsystem
	return nullptr;
}

bool UPGXBlueprintLibrary::IsPGXDataLoaded(const UObject* /*WorldContextObject*/, FName /*DataId*/)
{
	// EN: Stub - will check UPGXDataRegistrySubsystem cache
	// ES: Stub - verificara cache de UPGXDataRegistrySubsystem
	return false;
}

bool UPGXBlueprintLibrary::ValidatePGXAsset(UObject* /*Asset*/, TArray<FText>& /*OutErrors*/)
{
	// EN: Stub - will call Validate() on the asset if it's a UPGXDataAsset
	// ES: Stub - llamara Validate() en el asset si es un UPGXDataAsset
	return true;
}
