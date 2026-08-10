// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXMGOSTypes.h"
#include "PGXGCObserverConfig.generated.h"

/**
 * EN: Configuration DataAsset for the MGOS (GC Observability System).
 *     Controls observer mode, thresholds, tracked classes, and all tunable parameters.
 * ES: DataAsset de configuracion para el MGOS (Sistema de Observabilidad GC).
 *     Controla el modo del observador, umbrales, clases rastreadas y todos los parametros ajustables.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXMGOSRUNTIME_API UPGXGCObserverConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ====================================================================
	// General
	// ====================================================================

	/** EN: Default operating mode / ES: Modo de operacion por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|General")
	EPGXGCObserverMode DefaultMode = EPGXGCObserverMode::Passive;

	/** EN: Enable incident detection and reporting / ES: Habilitar deteccion y reporte de incidentes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|General")
	bool bEnableIncidents = true;

	/** EN: Allow DeepTrack mode in Shipping builds / ES: Permitir modo DeepTrack en builds Shipping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|General", meta = (AdvancedDisplay))
	bool bAllowDeepTrackInShipping = false;

	// ====================================================================
	// Windows
	// ====================================================================

	/** EN: History window size for production (ring buffer capacity) / ES: Tamano de ventana de historial para produccion (capacidad del buffer circular) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Windows", meta = (AdvancedDisplay, ClampMin = "4", ClampMax = "256"))
	int32 HistoryWindowSize = 64;

	/** EN: Reduced history window for test harness / ES: Ventana de historial reducida para test harness */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Windows", meta = (AdvancedDisplay, ClampMin = "2", ClampMax = "16"))
	int32 TestWindowSize = 4;

	/** EN: Number of warmup cycles before baseline can be captured / ES: Numero de ciclos de calentamiento antes de capturar baseline */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Windows", meta = (AdvancedDisplay, ClampMin = "1", ClampMax = "128"))
	int32 WarmupCycles = 16;

	/** EN: Consecutive cycles to confirm an incident (N_Confirm) / ES: Ciclos consecutivos para confirmar un incidente (N_Confirm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Windows", meta = (AdvancedDisplay, ClampMin = "1", ClampMax = "32"))
	int32 ConfirmCycles = 8;

	/** EN: Number of top classes to track (K) / ES: Numero de top clases a rastrear (K) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Windows", meta = (AdvancedDisplay, ClampMin = "1", ClampMax = "50"))
	int32 TopKClasses = 10;

	/** EN: Capture snapshot every N cycles (>1 for time-slicing) / ES: Capturar snapshot cada N ciclos (>1 para time-slicing) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Windows", meta = (AdvancedDisplay, ClampMin = "1", ClampMax = "10"))
	int32 SnapshotFrequency = 1;

	// ====================================================================
	// Thresholds
	// ====================================================================

	/** EN: Accumulation threshold (fraction per cycle, 0.005 = 0.5%) / ES: Umbral de acumulacion (fraccion por ciclo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Thresholds", meta = (AdvancedDisplay, ClampMin = "0.001", ClampMax = "0.1"))
	float ThresholdAccumulation = 0.005f;

	/** EN: Leak threshold (fraction per cycle, 0.01 = 1%) / ES: Umbral de leak (fraccion por ciclo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Thresholds", meta = (AdvancedDisplay, ClampMin = "0.005", ClampMax = "0.5"))
	float ThresholdLeak = 0.01f;

	/** EN: Variance threshold for GC duration in ms / ES: Umbral de varianza para duracion GC en ms */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Thresholds", meta = (AdvancedDisplay, ClampMin = "0.1", ClampMax = "100.0"))
	float ThresholdVarianceDuration = 5.0f;

	/** EN: PendingKill ratio threshold (PK / Total) / ES: Umbral de ratio PendingKill (PK / Total) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Thresholds", meta = (AdvancedDisplay, ClampMin = "0.01", ClampMax = "0.5"))
	float ThresholdPKRatio = 0.1f;

	/** EN: Z-score threshold for duration anomaly / ES: Umbral de Z-score para anomalia de duracion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Thresholds", meta = (AdvancedDisplay, ClampMin = "1.0", ClampMax = "10.0"))
	float ThresholdZScoreDuration = 3.0f;

	/** EN: Z-score threshold for destruction anomaly / ES: Umbral de Z-score para anomalia de destruccion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Thresholds", meta = (AdvancedDisplay, ClampMin = "1.0", ClampMax = "10.0"))
	float ThresholdZScoreDestruction = 2.5f;

	/** EN: Drift threshold — UObjects over baseline / ES: Umbral de drift — UObjects sobre baseline */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Thresholds", meta = (AdvancedDisplay, ClampMin = "10.0", ClampMax = "10000.0"))
	float ThresholdDrift = 500.0f;

	/** EN: Absolute epsilon for count comparisons / ES: Epsilon absoluto para comparaciones de conteo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Thresholds", meta = (AdvancedDisplay, ClampMin = "0.0", ClampMax = "100.0"))
	float EpsilonAbsolute = 5.0f;

	/** EN: Relative epsilon for ratio comparisons / ES: Epsilon relativo para comparaciones de ratio */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Thresholds", meta = (AdvancedDisplay, ClampMin = "0.001", ClampMax = "0.1"))
	float EpsilonRelative = 0.01f;

	// ====================================================================
	// Incidents
	// ====================================================================

	/** EN: Maximum number of incidents to retain (FIFO trim). 0 = unlimited.
	 *  ES: Maximo de incidentes a retener (trim FIFO). 0 = ilimitado. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Incidents", meta = (AdvancedDisplay, ClampMin = "0", ClampMax = "10000"))
	int32 MaxIncidentHistory = 256;

	// ====================================================================
	// Classes
	// ====================================================================

	/** EN: Classes to always monitor via TObjectIterator<T> / ES: Clases a monitorear siempre via TObjectIterator<T> */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Classes", meta = (AdvancedDisplay))
	TArray<TSoftClassPtr<UObject>> TrackedClasses;

	/** EN: Classes exempt from accumulation incidents / ES: Clases exentas de incidentes de acumulacion */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|Classes", meta = (AdvancedDisplay))
	TArray<TSoftClassPtr<UObject>> ExcludedClasses;

	// ====================================================================
	// InterCycle
	// ====================================================================

	/** EN: Enable periodic inter-cycle monitoring / ES: Habilitar monitoreo periodico inter-ciclo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|InterCycle", meta = (AdvancedDisplay))
	bool bEnableInterCycleMonitoring = true;

	/** EN: Interval in seconds for inter-cycle checks / ES: Intervalo en segundos para chequeos inter-ciclo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|InterCycle", meta = (AdvancedDisplay, ClampMin = "1.0", ClampMax = "60.0"))
	float InterCycleCheckInterval = 5.0f;

	/** EN: UObject growth without GC that triggers warning / ES: Crecimiento de UObjects sin GC que dispara advertencia */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|InterCycle", meta = (AdvancedDisplay, ClampMin = "100.0", ClampMax = "100000.0"))
	float CriticalGrowthThreshold = 10000.0f;

	// ====================================================================
	// SystemMemory
	// ====================================================================

	/** EN: Enable process memory tracking for non-UObject leak detection / ES: Habilitar rastreo de memoria de proceso para deteccion de leaks no-UObject */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|SystemMemory", meta = (AdvancedDisplay))
	bool bEnableProcessMemoryTracking = true;

	/** EN: Process memory drift threshold in MB (growth with stable UObjects) / ES: Umbral de drift de memoria de proceso en MB (crecimiento con UObjects estables) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|MGOS|SystemMemory", meta = (AdvancedDisplay, ClampMin = "10.0", ClampMax = "2048.0"))
	float ProcessMemoryDriftThresholdMB = 100.0f;
};
