// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAsyncAction_SaveToSlot.h"
#include "PGXSaveSubsystem.h"
#include "Engine/GameInstance.h"

// EN: Async latent action for saving a context to a specific slot
// ES: Accion latente async para guardar un contexto en un slot especifico

UPGXAsyncAction_SaveToSlot* UPGXAsyncAction_SaveToSlot::SaveToSlotAsync(
	UObject* WorldContextObject,
	FGameplayTag ContextTag,
	const FString& SlotName)
{
	UPGXAsyncAction_SaveToSlot* Action = NewObject<UPGXAsyncAction_SaveToSlot>();
	Action->RequestedContextTag = ContextTag;
	Action->RequestedSlotName = SlotName;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UPGXAsyncAction_SaveToSlot::Activate()
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
	TWeakObjectPtr<UPGXAsyncAction_SaveToSlot> WeakThis(this);
	const FString CapturedSlot = RequestedSlotName;

	CompletedHandle = SaveSub->OnSaveCompletedNative.AddLambda(
		[WeakThis, CapturedSlot, SaveSub](const FString& SlotName, EPGXSaveResult Result)
		{
			// EN: Only respond to our specific slot's completion
			// ES: Solo responder a la completacion de nuestro slot especifico
			if (SlotName != CapturedSlot)
			{
				return;
			}

			if (UPGXAsyncAction_SaveToSlot* Self = WeakThis.Get())
			{
				// EN: Remove delegate before broadcasting to prevent re-entry
				// ES: Remover delegate antes de broadcast para prevenir re-entrada
				SaveSub->OnSaveCompletedNative.Remove(Self->CompletedHandle);
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

	// EN: Initiate the async save
	// ES: Iniciar el guardado async
	SaveSub->SaveContextAsync(RequestedContextTag, RequestedSlotName);
}

void UPGXAsyncAction_SaveToSlot::BeginDestroy()
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
					SaveSub->OnSaveCompletedNative.Remove(CompletedHandle);
				}
			}
		}
		CompletedHandle.Reset();
	}

	Super::BeginDestroy();
}
