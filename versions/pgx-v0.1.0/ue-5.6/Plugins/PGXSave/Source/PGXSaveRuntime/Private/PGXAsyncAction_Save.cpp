// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAsyncAction_Save.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveConfig.h"
#include "Engine/GameInstance.h"

// EN: Async latent action for saving with auto-resolved slot name
// ES: Accion latente async para guardar con slot auto-resuelto

UPGXAsyncAction_Save* UPGXAsyncAction_Save::SaveAsync(
	UObject* WorldContextObject,
	FGameplayTag ContextTag)
{
	UPGXAsyncAction_Save* Action = NewObject<UPGXAsyncAction_Save>();
	Action->RequestedContextTag = ContextTag;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UPGXAsyncAction_Save::Activate()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		OnFailed.Broadcast(EPGXSaveResult::Failed);
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		OnFailed.Broadcast(EPGXSaveResult::Failed);
		SetReadyToDestroy();
		return;
	}

	UPGXSaveSubsystem* SaveSub = GI->GetSubsystem<UPGXSaveSubsystem>();
	if (!SaveSub)
	{
		OnFailed.Broadcast(EPGXSaveResult::Failed);
		SetReadyToDestroy();
		return;
	}

	// EN: Resolve slot name from the context's config DA
	// ES: Resolver nombre de slot desde el DA config del contexto
	const UPGXSaveConfig* Config = SaveSub->GetContextConfig(RequestedContextTag);
	if (!Config)
	{
		OnFailed.Broadcast(EPGXSaveResult::Failed);
		SetReadyToDestroy();
		return;
	}

	ResolvedSlotName = Config->QuickSaveSlotName;

	// EN: Bind to the subsystem's delegate to receive completion notification
	// ES: Bind al delegate del subsistema para recibir notificacion de completado
	TWeakObjectPtr<UPGXAsyncAction_Save> WeakThis(this);
	const FString CapturedSlot = ResolvedSlotName;

	CompletedHandle = SaveSub->OnSaveCompletedNative.AddLambda(
		[WeakThis, CapturedSlot, SaveSub](const FString& SlotName, EPGXSaveResult Result)
		{
			// EN: Only respond to our specific slot's completion
			// ES: Solo responder a la completacion de nuestro slot especifico
			if (SlotName != CapturedSlot)
			{
				return;
			}

			if (UPGXAsyncAction_Save* Self = WeakThis.Get())
			{
				// EN: Remove delegate before broadcasting to prevent re-entry
				// ES: Remover delegate antes de broadcast para prevenir re-entrada
				SaveSub->OnSaveCompletedNative.Remove(Self->CompletedHandle);
				Self->CompletedHandle.Reset();

				if (Result == EPGXSaveResult::Success)
				{
					Self->OnCompleted.Broadcast();
				}
				else
				{
					Self->OnFailed.Broadcast(Result);
				}

				Self->SetReadyToDestroy();
			}
		});

	// EN: Initiate the async save with auto-resolved slot
	// ES: Iniciar el guardado async con slot auto-resuelto
	SaveSub->SaveContextAsync(RequestedContextTag, ResolvedSlotName);
}

void UPGXAsyncAction_Save::BeginDestroy()
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
