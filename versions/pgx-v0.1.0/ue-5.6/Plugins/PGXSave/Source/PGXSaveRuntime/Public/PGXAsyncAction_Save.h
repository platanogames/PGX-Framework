// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXAsyncAction.h"
#include "GameplayTagContainer.h"
#include "PGXSaveTypes.h"
#include "PGXAsyncAction_Save.generated.h"

/**
 * EN: Async latent Blueprint action for saving with auto-resolved slot name.
 *     Resolves the QuickSaveSlotName from the context's SaveConfig DA,
 *     then delegates to SaveContextAsync. Exposes OnCompleted / OnFailed pins.
 *
 * ES: Accion latente async de Blueprint para guardar con slot auto-resuelto.
 *     Resuelve QuickSaveSlotName del DA SaveConfig del contexto,
 *     luego delega a SaveContextAsync. Expone pins OnCompleted / OnFailed.
 */
UCLASS(meta = (ExposedAsyncProxy = "AsyncAction"))
class PGXSAVERUNTIME_API UPGXAsyncAction_Save : public UPGXAsyncAction
{
	GENERATED_BODY()

public:
	/** EN: Fired when save completes successfully / ES: Disparado cuando el guardado completa exitosamente */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveCompleted);
	UPROPERTY(BlueprintAssignable)
	FOnSaveCompleted OnCompleted;

	/** EN: Fired when save fails / ES: Disparado cuando el guardado falla */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveFailed, EPGXSaveResult, Result);
	UPROPERTY(BlueprintAssignable)
	FOnSaveFailed OnFailed;

	/**
	 * EN: Save a context asynchronously using the configured slot name.
	 *     Returns a latent node with Completed/Failed pins.
	 * ES: Guardar un contexto asincronamente usando el nombre de slot configurado.
	 *     Retorna un nodo latente con pins Completed/Failed.
	 *
	 * @param WorldContextObject  EN: World context / ES: Contexto del mundo
	 * @param ContextTag          EN: Save context identifier / ES: Identificador del contexto de guardado
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		WorldContext = "WorldContextObject", DisplayName = "Save (Async)"),
		Category = "PGX|Save")
	static UPGXAsyncAction_Save* SaveAsync(
		UObject* WorldContextObject,
		FGameplayTag ContextTag);

	void Activate() override;
	void BeginDestroy() override;

private:
	FGameplayTag RequestedContextTag;
	FString ResolvedSlotName;

	/** EN: Handle for cleanup of subsystem delegate / ES: Handle para limpieza del delegate del subsistema */
	FDelegateHandle CompletedHandle;
};
