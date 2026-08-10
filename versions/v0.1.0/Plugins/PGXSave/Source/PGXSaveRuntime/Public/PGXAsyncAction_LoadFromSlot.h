// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXAsyncAction.h"
#include "GameplayTagContainer.h"
#include "PGXSaveTypes.h"
#include "PGXAsyncAction_LoadFromSlot.generated.h"

class UPGXSaveGame;

/**
 * EN: Async latent Blueprint action for loading a context from a specific slot.
 *     Reads from disk on background thread, deserializes on GameThread.
 *     Exposes OnSuccess and OnFailure output execution pins in Blueprint.
 *
 * ES: Accion latente async de Blueprint para cargar un contexto desde un slot especifico.
 *     Lee de disco en hilo de fondo, deserializa en GameThread.
 *     Expone pins de ejecucion OnSuccess y OnFailure en Blueprint.
 */
UCLASS(meta = (ExposedAsyncProxy = "AsyncAction"))
class PGXSAVERUNTIME_API UPGXAsyncAction_LoadFromSlot : public UPGXAsyncAction
{
	GENERATED_BODY()

public:
	/** EN: Fired when load completes successfully / ES: Disparado cuando la carga completa exitosamente */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadSuccess, const FString&, SlotName);
	UPROPERTY(BlueprintAssignable)
	FOnLoadSuccess OnSuccess;

	/** EN: Fired when load fails / ES: Disparado cuando la carga falla */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadFailure, const FString&, SlotName, EPGXSaveResult, Result);
	UPROPERTY(BlueprintAssignable)
	FOnLoadFailure OnFailure;

	/**
	 * EN: Load a context from a specific slot asynchronously. Returns a latent node with Success/Failure pins.
	 * ES: Cargar un contexto desde un slot especifico asincronamente. Retorna un nodo latente con pins Success/Failure.
	 *
	 * @param WorldContextObject  EN: World context / ES: Contexto del mundo
	 * @param ContextTag          EN: Save context identifier / ES: Identificador del contexto de guardado
	 * @param SlotName            EN: Source slot name / ES: Nombre del slot fuente
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		WorldContext = "WorldContextObject", DisplayName = "Load from Slot (Async)"),
		Category = "PGX|Save|Advanced")
	static UPGXAsyncAction_LoadFromSlot* LoadFromSlotAsync(
		UObject* WorldContextObject,
		FGameplayTag ContextTag,
		const FString& SlotName);

	void Activate() override;
	void BeginDestroy() override;

private:
	FGameplayTag RequestedContextTag;
	FString RequestedSlotName;

	/** EN: Handle for cleanup of subsystem delegate / ES: Handle para limpieza del delegate del subsistema */
	FDelegateHandle CompletedHandle;
};
