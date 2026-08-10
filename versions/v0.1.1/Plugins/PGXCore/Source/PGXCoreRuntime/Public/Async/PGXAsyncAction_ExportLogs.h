// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Base/PGXAsyncAction.h"
#include "PGXAsyncAction_ExportLogs.generated.h"

/**
 * EN: Latent Blueprint node to export PGX logs to a JSON file asynchronously.
 *     Returns the file path on success or error message on failure.
 *     Used by Widget Developer and Editor Utility Widgets.
 *
 * ES: Nodo latente de Blueprint para exportar logs PGX a archivo JSON asincronamente.
 *     Retorna la ruta del archivo en exito o mensaje de error en fallo.
 *     Usado por Widget Developer y Editor Utility Widgets.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXAsyncAction_ExportLogs : public UPGXAsyncAction
{
	GENERATED_BODY()

public:
	/** EN: Fired on successful export / ES: Disparado al exportar exitosamente */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExportSuccess, const FString&, FilePath);

	UPROPERTY(BlueprintAssignable)
	FOnExportSuccess OnSuccess;

	/** EN: Fired on export failure / ES: Disparado al fallar la exportacion */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExportFailure, const FString&, ErrorMessage);

	UPROPERTY(BlueprintAssignable)
	FOnExportFailure OnFailure;

	/**
	 * EN: Export PGX logs to JSON file. Empty path uses default from Config DA.
	 * ES: Exportar logs PGX a archivo JSON. Path vacio usa default del Config DA.
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true",
		WorldContext = "WorldContextObject"), Category = "PGX|Log")
	static UPGXAsyncAction_ExportLogs* ExportLogsToJson(
		UObject* WorldContextObject,
		const FString& ExportPath = TEXT(""));

	void Activate() override;

private:
	FString RequestedPath;
};
