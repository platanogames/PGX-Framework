// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXPSOTypes.h"
#include "PGXPSOWarmUpConfig.generated.h"

/**
 * EN: Configuration DataAsset for PSO warm-up. Each instance defines a set of
 *     materials/entries to precache, along with activation mode, batching,
 *     persistence, and concurrency settings.
 *     Discovered automatically via AssetRegistry at subsystem initialization.
 * ES: DataAsset de configuracion para warm-up de PSO. Cada instancia define un
 *     conjunto de materiales/entradas a precachear, junto con modo de activacion,
 *     batching, persistencia y configuracion de concurrencia.
 *     Descubierto automaticamente via AssetRegistry al inicializar el subsistema.
 */
UCLASS(BlueprintType)
class PGXPSORUNTIME_API UPGXPSOWarmUpConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ========================================================================
	// EN: Catalog Entries
	// ES: Entradas de Catalogo
	// ========================================================================

	/** EN: List of PSO entries to precache / ES: Lista de entradas PSO a precachear */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Catalog")
	TArray<FPGXPSOEntry> Entries;

	// ========================================================================
	// EN: Activation
	// ES: Activacion
	// ========================================================================

	/** EN: When to trigger warm-up for this config / ES: Cuando activar el warm-up para esta config */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Activation")
	EPGXPSOActivationMode ActivationMode = EPGXPSOActivationMode::OnSubsystemInit;

	/** EN: GameFlow tag that triggers warm-up (only for OnGameFlowTag mode) / ES: Tag de GameFlow que activa el warm-up (solo para modo OnGameFlowTag) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Activation",
		meta = (AdvancedDisplay, Categories = "PGX.GameFlow.Phase", EditCondition = "ActivationMode == EPGXPSOActivationMode::OnGameFlowTag"))
	FGameplayTag TriggerGameFlowTag;

	// ========================================================================
	// EN: Batching
	// ES: Batching
	// ========================================================================

	/** EN: Number of entries per batch (0 = all at once) / ES: Numero de entradas por lote (0 = todas a la vez) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Batching",
		meta = (AdvancedDisplay, ClampMin = "0", ClampMax = "100"))
	int32 BatchSize = 0;

	/** EN: Delay between batches in seconds / ES: Delay entre lotes en segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Batching",
		meta = (AdvancedDisplay, ClampMin = "0.0", EditCondition = "BatchSize > 0"))
	float BatchDelaySeconds = 0.016f;

	// ========================================================================
	// EN: Persistence
	// ES: Persistencia
	// ========================================================================

	/** EN: Whether to save the PSO cache to disk after warm-up / ES: Si guardar el cache PSO a disco tras warm-up */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Persistence", meta = (AdvancedDisplay))
	bool bSaveCacheAfterWarmUp = true;

	/** EN: When to save the cache / ES: Cuando guardar el cache */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Persistence",
		meta = (AdvancedDisplay, EditCondition = "bSaveCacheAfterWarmUp"))
	EPGXPSOSavePolicy SavePolicy = EPGXPSOSavePolicy::OnCompleteOnly;

	/** EN: Interval for periodic saves (only for Periodic policy) / ES: Intervalo para guardados periodicos (solo para politica Periodic) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Persistence",
		meta = (AdvancedDisplay, EditCondition = "SavePolicy == EPGXPSOSavePolicy::Periodic", ClampMin = "5.0"))
	float PeriodicSaveIntervalSeconds = 30.0f;

	// ========================================================================
	// EN: Concurrency
	// ES: Concurrencia
	// ========================================================================

	/** EN: How to handle new warm-up requests when one is already active / ES: Como manejar nuevas solicitudes de warm-up cuando ya hay una activa */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Activation", meta = (AdvancedDisplay))
	EPGXPSOConcurrencyPolicy ConcurrencyPolicy = EPGXPSOConcurrencyPolicy::MergeAndContinue;

	// ========================================================================
	// EN: Memory
	// ES: Memoria
	// ========================================================================

	/** EN: Max materials loaded simultaneously (GC management) / ES: Max materiales cargados simultaneamente (gestion de GC) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|PSO|Memory",
		meta = (AdvancedDisplay, ClampMin = "1", ClampMax = "100"))
	int32 MaxSimultaneousLoads = 20;
};
