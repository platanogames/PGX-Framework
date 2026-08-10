// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAsyncAction_LoadFromSlot.h"
#include "PGXSaveSubsystem.h"
#include "Engine/GameInstance.h"

// EN: Async latent action for loading a context from a specific slot
// ES: Accion latente async para cargar un contexto desde un slot especifico

UPGXAsyncAction_LoadFromSlot* UPGXAsyncAction_LoadFromSlot::LoadFromSlotAsync(
	UObject* WorldContextObject,
	FGameplayTag ContextTag,
	const FString& SlotName)
{
	UPGXAsyncAction_LoadFromSlot* Action = NewObject<UPGXAsyncAction_LoadFromSlot>();
	Action->RequestedContextTag = ContextTag;
	Action->RequestedSlotName = SlotName;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UPGXAsyncAction_LoadFromSlot::Activate()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		OnFailure.Broadcast(RequestedSlotName, EPGXSaveResult::Failed);
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		OnFailure.Broadcast(RequestedSlotName, EPGXSaveResult::Failed);
		SetReadyToDestroy();
		return;
	}

	UPGXSaveSubsystem* SaveSub = GI->GetSubsystem<UPGXSaveSubsystem>();
	if (!SaveSub)
	{
		OnFailure.Broadcast(RequestedSlotName, EPGXSaveResult::Failed);
		SetReadyToDestroy();
		return;
	}

	// EN: Bind to the subsystem's delegate to receive completion notification
	// ES: Bind al delegate del subsistema para recibir notificacion de completado
	TWeakObjectPtr<UPGXAsyncAction_LoadFromSlot> WeakThis(this);
	const FString CapturedSlot = RequestedSlotName;

	CompletedHandle = SaveSub->OnLoadCompletedNative.AddLambda(
		[WeakThis, CapturedSlot, SaveSub](const FString& SlotName, EPGXSaveResult Result, UPGXSaveGame* /*SaveGame*/)
		{
			// EN: Only respond to our specific slot's completion
			// ES: Solo responder a la completacion de nuestro slot especifico
			if (SlotName != CapturedSlot)
			{
				return;
			}

			if (UPGXAsyncAction_LoadFromSlot* Self = WeakThis.Get())
			{
				// EN: Remove delegate before broadcasting to prevent re-entry
				// ES: Remover delegate antes de broadcast para prevenir re-entrada
				SaveSub->OnLoadCompletedNative.Remove(Self->CompletedHandle);
				Self->CompletedHandle.Reset();

				if (Result == EPGXSaveResult::Success)
				{
					Self->OnSuccess.Broadcast(SlotName);
				}
				else
				{
					Self->OnFailure.Broadcast(SlotName, Result);
				}

				Self->SetReadyToDestroy();
			}
		});

	// EN: Initiate the async load
	// ES: Iniciar la carga async
	SaveSub->LoadContextAsync(RequestedContextTag, RequestedSlotName);
}

void UPGXAsyncAction_LoadFromSlot::BeginDestroy()
{
	// EN: Safety net — remove delegate if action is GC'd before completion
	// ES: Red de seguridad — remover delegate si la accion es GC'd antes de completar
	if (CompletedHandle.IsValid())
	{
		if (const UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (UPGXSaveSubsystem* SaveSub = GI->GetSubsystem<UPGXSaveSubsystem>())
				{
					SaveSub->OnLoadCompletedNative.Remove(CompletedHandle);
				}
			}
		}
		CompletedHandle.Reset();
	}

	Super::BeginDestroy();
}
