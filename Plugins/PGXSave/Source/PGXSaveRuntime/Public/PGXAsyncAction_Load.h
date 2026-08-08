// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXAsyncAction.h"
#include "GameplayTagContainer.h"
#include "PGXSaveTypes.h"
#include "PGXAsyncAction_Load.generated.h"

class UPGXSaveGame;

/**
 * EN: Async latent Blueprint action for loading with auto-resolved slot name.
 *     Resolves the QuickSaveSlotName from the context's SaveConfig DA,
 *     then delegates to LoadContextAsync. Exposes OnCompleted / OnFailed pins.
 *
 * ES: Accion latente async de Blueprint para cargar con slot auto-resuelto.
 *     Resuelve QuickSaveSlotName del DA SaveConfig del contexto,
 *     luego delega a LoadContextAsync. Expone pins OnCompleted / OnFailed.
 */
UCLASS(meta = (ExposedAsyncProxy = "AsyncAction"))
class PGXSAVERUNTIME_API UPGXAsyncAction_Load : public UPGXAsyncAction
{
	GENERATED_BODY()

public:
	/** EN: Fired when load completes successfully / ES: Disparado cuando la carga completa exitosamente */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadCompleted);
	UPROPERTY(BlueprintAssignable)
	FOnLoadCompleted OnCompleted;

	/** EN: Fired when load fails / ES: Disparado cuando la carga falla */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadFailed, EPGXSaveResult, Result);
	UPROPERTY(BlueprintAssignable)
	FOnLoadFailed OnFailed;

	/**
	 * EN: Load a context asynchronously using the configured slot name.
	 *     Returns a latent node with Completed/Failed pins.
	 * ES: Cargar un contexto asincronamente usando el nombre de slot configurado.
	 *     Retorna un nodo latente con pins Completed/Failed.
	 *
	 * @param WorldContextObject  EN: World context / ES: Contexto del mundo
	 * @param ContextTag          EN: Save context identifier / ES: Identificador del contexto de guardado
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		WorldContext = "WorldContextObject", DisplayName = "Load (Async)"),
		Category = "PGX|Save")
	static UPGXAsyncAction_Load* LoadAsync(
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
