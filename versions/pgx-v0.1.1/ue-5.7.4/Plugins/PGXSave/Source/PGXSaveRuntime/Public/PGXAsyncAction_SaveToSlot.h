// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXAsyncAction.h"
#include "GameplayTagContainer.h"
#include "PGXSaveTypes.h"
#include "PGXAsyncAction_SaveToSlot.generated.h"

/**
 * EN: Async latent Blueprint action for saving a context to a specific slot.
 *     Serializes on GameThread, writes to disk on background thread.
 *     Exposes OnSuccess and OnFailure output execution pins in Blueprint.
 *
 * ES: Accion latente async de Blueprint para guardar un contexto en un slot especifico.
 *     Serializa en GameThread, escribe a disco en hilo de fondo.
 *     Expone pins de ejecucion OnSuccess y OnFailure en Blueprint.
 */
UCLASS(meta = (ExposedAsyncProxy = "AsyncAction"))
class PGXSAVERUNTIME_API UPGXAsyncAction_SaveToSlot : public UPGXAsyncAction
{
	GENERATED_BODY()

public:
	/** EN: Fired when save completes successfully / ES: Disparado cuando el guardado completa exitosamente */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveSuccess, const FString&, SlotName);
	UPROPERTY(BlueprintAssignable)
	FOnSaveSuccess OnSuccess;

	/** EN: Fired when save fails / ES: Disparado cuando el guardado falla */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveFailure, const FString&, SlotName, EPGXSaveResult, Result);
	UPROPERTY(BlueprintAssignable)
	FOnSaveFailure OnFailure;

	/**
	 * EN: Save a context to a specific slot asynchronously. Returns a latent node with Success/Failure pins.
	 * ES: Guardar un contexto en un slot especifico asincronamente. Retorna un nodo latente con pins Success/Failure.
	 *
	 * @param WorldContextObject  EN: World context / ES: Contexto del mundo
	 * @param ContextTag          EN: Save context identifier / ES: Identificador del contexto de guardado
	 * @param SlotName            EN: Target slot name / ES: Nombre del slot destino
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		WorldContext = "WorldContextObject", DisplayName = "Save to Slot (Async)"),
		Category = "PGX|Save|Advanced")
	static UPGXAsyncAction_SaveToSlot* SaveToSlotAsync(
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
