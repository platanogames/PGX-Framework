// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Base/PGXAsyncAction.h"
#include "Logging/PGXLogTypes.h"
#include "PGXAsyncAction_LoadLogs.generated.h"

/**
 * EN: Latent Blueprint node to load historical PGX logs from a .sav slot.
 *     Returns the loaded entries on success or error message on failure.
 *     Used by Widget Developer for historical log viewing.
 *
 * ES: Nodo latente de Blueprint para cargar logs PGX historicos desde un slot .sav.
 *     Retorna las entradas cargadas en exito o mensaje de error en fallo.
 *     Usado por Widget Developer para visualizacion de logs historicos.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXAsyncAction_LoadLogs : public UPGXAsyncAction
{
	GENERATED_BODY()

public:
	/** EN: Fired when logs are loaded / ES: Disparado cuando los logs se cargan */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogsLoaded, const TArray<FPGXLogEntry>&, Entries);

	UPROPERTY(BlueprintAssignable)
	FOnLogsLoaded OnLogsLoaded;

	/** EN: Fired on load failure / ES: Disparado al fallar la carga */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadFailure, const FString&, ErrorMessage);

	UPROPERTY(BlueprintAssignable)
	FOnLoadFailure OnFailure;

	/**
	 * EN: Load logs from .sav slot. SessionId -1 = latest session.
	 * ES: Cargar logs desde slot .sav. SessionId -1 = sesion mas reciente.
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		WorldContext = "WorldContextObject"), Category = "PGX|Log")
	static UPGXAsyncAction_LoadLogs* LoadLogsFromSlot(
		UObject* WorldContextObject,
		int32 SessionId = -1);

	void Activate() override;

private:
	int32 RequestedSessionId = -1;
};
