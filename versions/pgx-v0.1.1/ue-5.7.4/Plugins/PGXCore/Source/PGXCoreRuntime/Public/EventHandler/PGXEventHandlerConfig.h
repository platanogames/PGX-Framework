// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "Observability/PGXObservable.h"
#include "PGXEventHandlerConfig.generated.h"

class UDataTable;

/**
 * EN: Configuration DataAsset for the PGX Event Handler System.
 *     Auto-discovered via AssetRegistry during subsystem initialization.
 *     References DataTables containing FPGXEventHandlerRow entries.
 * ES: DataAsset de configuracion para el Sistema de Event Handler PGX.
 *     Auto-descubierto via AssetRegistry durante inicializacion del subsistema.
 *     Referencia DataTables con entradas FPGXEventHandlerRow.
 */
UCLASS(BlueprintType)
class PGXCORERUNTIME_API UPGXEventHandlerConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	UPGXEventHandlerConfig();

	//~ Begin IPGXObservable (delegates to PGXCoreObservability helpers)
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: DataTables containing handler registrations / ES: DataTables con registros de handlers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler")
	TArray<TSoftObjectPtr<UDataTable>> HandlerTables;

	/** EN: Maximum number of cached handler instances / ES: Maximo de instancias de handler cacheadas */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Cache",
		meta = (AdvancedDisplay, ClampMin = "16", ClampMax = "1024"))
	int32 MaxCachedHandlers = 128;

	/** EN: Maximum recursion depth for sub-handler execution / ES: Profundidad maxima de recursion */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Safety",
		meta = (AdvancedDisplay, ClampMin = "1", ClampMax = "32"))
	int32 MaxExecutionDepth = 8;

	/** EN: Per-category memory budgets / ES: Presupuestos de memoria por categoria */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Cache", meta = (AdvancedDisplay))
	TArray<FPGXCategoryBudget> CategoryBudgets;

	/** EN: Size of the blackbox execution history buffer / ES: Tamano del buffer de historial de blackbox */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Observability",
		meta = (AdvancedDisplay, ClampMin = "32", ClampMax = "2048"))
	int32 BlackboxBufferSize = 256;

	// ─── Logging ───

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Logging", meta = (AdvancedDisplay))
	bool bLogExecutions = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Logging", meta = (AdvancedDisplay))
	bool bLogCacheOperations = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Logging", meta = (AdvancedDisplay))
	bool bLogRegistration = false;

	// ─── Auto Export ───

	/** EN: Automatically export a report when PIE ends / ES: Exportar reporte automaticamente al terminar PIE */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Report", meta = (AdvancedDisplay))
	bool bAutoExportOnPIEEnd = false;

	/** EN: Number of blackbox entries included in reports / ES: Cantidad de entradas de blackbox en reportes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Report",
		meta = (AdvancedDisplay, ClampMin = "10", ClampMax = "500"))
	int32 BlackboxEntriesInReport = 50;

	/** EN: Redact Instigator/Target object names in blackbox/export for privacy-sensitive builds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Observability", meta = (AdvancedDisplay))
	bool bRedactBlackboxObjectNames = false;

	/** EN: Prefer stable object paths over short names in blackbox when not redacted. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|EventHandler|Observability", meta = (AdvancedDisplay))
	bool bUseObjectPathInBlackbox = true;
};
