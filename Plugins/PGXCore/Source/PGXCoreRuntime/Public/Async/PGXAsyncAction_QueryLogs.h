// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Base/PGXAsyncAction.h"
#include "Logging/PGXLogTypes.h"
#include "PGXAsyncAction_QueryLogs.generated.h"

/**
 * EN: Latent Blueprint node to query PGX logs with filters.
 *     Returns filtered entries for Widget Developer real-time view.
 *     Runs the filter on a background thread to avoid blocking the game thread.
 *
 * ES: Nodo latente de Blueprint para consultar logs PGX con filtros.
 *     Retorna entradas filtradas para vista en tiempo real del Widget Developer.
 *     Ejecuta el filtro en un hilo background para no bloquear el hilo principal.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXAsyncAction_QueryLogs : public UPGXAsyncAction
{
	GENERATED_BODY()

public:
	/** EN: Fired with filtered results / ES: Disparado con resultados filtrados */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryResults, const TArray<FPGXLogEntry>&, Entries);

	UPROPERTY(BlueprintAssignable)
	FOnQueryResults OnResults;

	/**
	 * EN: Query logs with filters. CategoryFilter NAME_None = all categories.
	 * ES: Consultar logs con filtros. CategoryFilter NAME_None = todas las categorias.
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		WorldContext = "WorldContextObject"), Category = "PGX|Log")
	static UPGXAsyncAction_QueryLogs* QueryLogs(
		UObject* WorldContextObject,
		EPGXLogVerbosity MinVerbosity = EPGXLogVerbosity::Verbose,
		FName CategoryFilter = NAME_None,
		int32 MaxResults = 100);

	void Activate() override;

private:
	EPGXLogVerbosity QueryMinVerbosity = EPGXLogVerbosity::Verbose;
	FName QueryCategoryFilter = NAME_None;
	int32 QueryMaxResults = 100;
};
