// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXMGOSTypes.generated.h"

// ============================================================================
// Enums
// ============================================================================

/**
 * EN: Operating mode for the GC Observer subsystem.
 * ES: Modo de operacion del subsistema GC Observer.
 */
UENUM(BlueprintType)
enum class EPGXGCObserverMode : uint8
{
	/** EN: Disabled / ES: Desactivado */
	Off				UMETA(DisplayName = "Off"),
	/** EN: Lightweight — global counts only, zero iteration / ES: Ligero — solo conteos globales, sin iteracion */
	Passive			UMETA(DisplayName = "Passive"),
	/** EN: Per-class tracking via TrackedClasses list / ES: Rastreo por clase via lista TrackedClasses */
	Snapshot		UMETA(DisplayName = "Snapshot"),
	/** EN: Full analysis — global iteration + per-class health / ES: Analisis completo — iteracion global + salud por clase */
	DeepTrack		UMETA(DisplayName = "DeepTrack")
};

/**
 * EN: Behavioral profile state inferred from GC pattern analysis.
 * ES: Estado de perfil comportamental inferido del analisis de patrones GC.
 */
UENUM(BlueprintType)
enum class EPGXGCProfileState : uint8
{
	/** EN: Normal operation / ES: Operacion normal */
	Stable					UMETA(DisplayName = "Stable"),
	/** EN: Growing UObject count over baseline / ES: Conteo de UObjects creciendo sobre baseline */
	Accumulation			UMETA(DisplayName = "Accumulation"),
	/** EN: Persistent growth confirmed — probable leak / ES: Crecimiento persistente confirmado — probable leak */
	LeakSuspected			UMETA(DisplayName = "LeakSuspected"),
	/** EN: Massive destruction in single cycle / ES: Destruccion masiva en un solo ciclo */
	BurstClean				UMETA(DisplayName = "BurstClean"),
	/** EN: High ratio of PendingKill objects / ES: Alta proporcion de objetos PendingKill */
	PendingKillSaturation	UMETA(DisplayName = "PendingKillSaturation"),
	/** EN: Root set expanding without matching destruction / ES: Root set expandiendose sin destruccion correspondiente */
	RootExpansion			UMETA(DisplayName = "RootExpansion")
};

/**
 * EN: Phase of a GC cycle at which a snapshot was captured.
 * ES: Fase del ciclo GC en que se capturo un snapshot.
 */
UENUM(BlueprintType)
enum class EPGXGCPhase : uint8
{
	PreGC		UMETA(DisplayName = "PreGC"),
	PostGC		UMETA(DisplayName = "PostGC")
};

/**
 * EN: Severity level for GC incidents.
 * ES: Nivel de severidad para incidentes GC.
 */
UENUM(BlueprintType)
enum class EPGXGCSeverity : uint8
{
	Info		UMETA(DisplayName = "Info"),
	Warning		UMETA(DisplayName = "Warning"),
	Critical	UMETA(DisplayName = "Critical")
};

/**
 * EN: State of the baseline measurement.
 * ES: Estado de la medicion baseline.
 */
UENUM(BlueprintType)
enum class EPGXGCBaselineState : uint8
{
	/** EN: No baseline captured yet / ES: Aun no se captura baseline */
	Uninitialized	UMETA(DisplayName = "Uninitialized"),
	/** EN: Collecting warmup cycles / ES: Recolectando ciclos de calentamiento */
	Warmup			UMETA(DisplayName = "Warmup"),
	/** EN: Baseline is valid and current / ES: Baseline valido y actual */
	Valid			UMETA(DisplayName = "Valid"),
	/** EN: Baseline invalidated (map change, etc.) / ES: Baseline invalidado (cambio de mapa, etc.) */
	Stale			UMETA(DisplayName = "Stale")
};

/**
 * EN: Health classification for a tracked class.
 * ES: Clasificacion de salud para una clase rastreada.
 */
UENUM(BlueprintType)
enum class EPGXGCClassHealth : uint8
{
	Stable					UMETA(DisplayName = "Stable"),
	Accumulating			UMETA(DisplayName = "Accumulating"),
	PersistentOverBaseline	UMETA(DisplayName = "PersistentOverBaseline"),
	LeakSuspected			UMETA(DisplayName = "LeakSuspected")
};

/**
 * EN: Execution mode in which the observer is running.
 * ES: Modo de ejecucion en que el observador esta corriendo.
 */
UENUM(BlueprintType)
enum class EPGXExecutionMode : uint8
{
	PIE			UMETA(DisplayName = "PIE"),
	Standalone	UMETA(DisplayName = "Standalone"),
	Shipping	UMETA(DisplayName = "Shipping")
};

// ============================================================================
// Structs
// ============================================================================

/**
 * EN: Per-class instance count at a point in time.
 * ES: Conteo de instancias por clase en un punto en el tiempo.
 */
USTRUCT(BlueprintType)
struct PGXMGOSRUNTIME_API FPGXGCClassCount
{
	GENERATED_BODY()

	/** EN: Class name / ES: Nombre de la clase */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	FName ClassName;

	/** EN: Number of instances / ES: Numero de instancias */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int32 InstanceCount = 0;
};

/**
 * EN: Per-class delta between two snapshots.
 * ES: Delta por clase entre dos snapshots.
 */
USTRUCT(BlueprintType)
struct PGXMGOSRUNTIME_API FPGXGCClassDelta
{
	GENERATED_BODY()

	/** EN: Class name / ES: Nombre de la clase */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	FName ClassName;

	/** EN: Change in instance count (positive = growth) / ES: Cambio en conteo (positivo = crecimiento) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int32 DeltaCount = 0;
};

/**
 * EN: Complete snapshot of GC-relevant metrics at a point in time.
 * ES: Snapshot completo de metricas relevantes al GC en un punto en el tiempo.
 */
USTRUCT(BlueprintType)
struct PGXMGOSRUNTIME_API FPGXGCSnapshot
{
	GENERATED_BODY()

	/** EN: Monotonic cycle identifier / ES: Identificador monotonico de ciclo */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 CycleID = 0;

	/** EN: Timestamp (FPlatformTime::Seconds) / ES: Marca temporal */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	double Timestamp = 0.0;

	/** EN: Phase at which this snapshot was taken / ES: Fase en que se tomo este snapshot */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	EPGXGCPhase Phase = EPGXGCPhase::PreGC;

	/** EN: Execution mode / ES: Modo de ejecucion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	EPGXExecutionMode ExecMode = EPGXExecutionMode::PIE;

	/** EN: Build config (Debug/Development/Shipping) / ES: Config de build */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	FName BuildConfig;

	/** EN: Total UObject count from GUObjectArray / ES: Conteo total de UObjects */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 TotalUObjectCount = 0;

	/** EN: Total Actor count / ES: Conteo total de Actors */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 TotalActorCount = 0;

	/** EN: Total ActorComponent count / ES: Conteo total de ActorComponents */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 TotalComponentCount = 0;

	/** EN: PendingKill object count / ES: Conteo de objetos PendingKill */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 PendingKillCount = 0;

	/** EN: Estimated root set size / ES: Tamano estimado del root set */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 RootSetEstimate = 0;

	/** EN: Top K classes by instance count / ES: Top K clases por conteo de instancias */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	TArray<FPGXGCClassCount> TopClasses;

	/** EN: Process memory usage in MB / ES: Uso de memoria del proceso en MB */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	float ProcessMemoryMB = 0.0f;
};

/**
 * EN: Diff between pre-GC and post-GC snapshots for a single cycle.
 * ES: Diff entre snapshots pre-GC y post-GC para un solo ciclo.
 */
USTRUCT(BlueprintType)
struct PGXMGOSRUNTIME_API FPGXGCSnapshotDiff
{
	GENERATED_BODY()

	/** EN: Cycle this diff belongs to / ES: Ciclo al que pertenece este diff */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 CycleID = 0;

	/** EN: GC cycle duration in seconds / ES: Duracion del ciclo GC en segundos */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	double DurationSeconds = 0.0;

	/** EN: Change in total UObject count / ES: Cambio en conteo total de UObjects */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 DeltaTotalUObject = 0;

	/** EN: Change in Actor count / ES: Cambio en conteo de Actors */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 DeltaActorCount = 0;

	/** EN: Change in Component count / ES: Cambio en conteo de Components */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 DeltaComponentCount = 0;

	/** EN: Change in PendingKill count / ES: Cambio en conteo de PendingKill */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 DeltaPendingKill = 0;

	/** EN: Change in RootSet estimate / ES: Cambio en estimacion de RootSet */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 DeltaRootSet = 0;

	/** EN: Per-class deltas / ES: Deltas por clase */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	TArray<FPGXGCClassDelta> ClassDeltas;

	/** EN: Change in process memory MB / ES: Cambio en memoria del proceso MB */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	float ProcessMemoryDeltaMB = 0.0f;
};

/**
 * EN: Ring buffer for GC cycle history with aggregate computation.
 * ES: Buffer circular para historial de ciclos GC con calculo de agregados.
 */
USTRUCT()
struct PGXMGOSRUNTIME_API FPGXGCHistoryStore
{
	GENERATED_BODY()

	/** EN: Maximum number of cycles to keep / ES: Maximo numero de ciclos a mantener */
	int32 MaxCycles = 64;

	/** EN: Stored snapshots (post-GC) / ES: Snapshots almacenados (post-GC) */
	TArray<FPGXGCSnapshot> Snapshots;

	/** EN: Stored diffs / ES: Diffs almacenados */
	TArray<FPGXGCSnapshotDiff> Diffs;

	/** EN: Current head index in ring buffer / ES: Indice actual del head en buffer circular */
	int32 Head = 0;

	/** EN: Number of entries stored / ES: Numero de entradas almacenadas */
	int32 Count = 0;

	/** EN: Moving average of GC duration / ES: Promedio movil de duracion GC */
	double MovingAvgDuration = 0.0;

	/** EN: Moving average of UObject delta / ES: Promedio movil de delta UObject */
	double MovingAvgDelta = 0.0;

	/** EN: Moving average of PendingKill count / ES: Promedio movil de conteo PendingKill */
	double MovingAvgPendingKill = 0.0;

	// EN: Initialize the store with capacity / ES: Inicializar el store con capacidad
	void Init(int32 InMaxCycles)
	{
		MaxCycles = InMaxCycles;
		Snapshots.SetNum(MaxCycles);
		Diffs.SetNum(MaxCycles);
		Head = 0;
		Count = 0;
		MovingAvgDuration = 0.0;
		MovingAvgDelta = 0.0;
		MovingAvgPendingKill = 0.0;
	}

	// EN: Push a new snapshot+diff pair / ES: Insertar un nuevo par snapshot+diff
	void Push(const FPGXGCSnapshot& Snapshot, const FPGXGCSnapshotDiff& Diff)
	{
		Snapshots[Head] = Snapshot;
		Diffs[Head] = Diff;
		Head = (Head + 1) % MaxCycles;
		if (Count < MaxCycles)
		{
			Count++;
		}

		// EN: Update moving averages / ES: Actualizar promedios moviles
		UpdateAggregates();
	}

	// EN: Get the N most recent diffs / ES: Obtener los N diffs mas recientes
	TArray<FPGXGCSnapshotDiff> GetRecentDiffs(int32 N) const
	{
		TArray<FPGXGCSnapshotDiff> Result;
		const int32 Num = FMath::Min(N, Count);
		Result.Reserve(Num);
		for (int32 i = 0; i < Num; ++i)
		{
			const int32 Idx = (Head - 1 - i + MaxCycles) % MaxCycles;
			Result.Add(Diffs[Idx]);
		}
		return Result;
	}

	// EN: Get the N most recent snapshots / ES: Obtener los N snapshots mas recientes
	TArray<FPGXGCSnapshot> GetRecentSnapshots(int32 N) const
	{
		TArray<FPGXGCSnapshot> Result;
		const int32 Num = FMath::Min(N, Count);
		Result.Reserve(Num);
		for (int32 i = 0; i < Num; ++i)
		{
			const int32 Idx = (Head - 1 - i + MaxCycles) % MaxCycles;
			Result.Add(Snapshots[Idx]);
		}
		return Result;
	}

	// EN: Get the most recent snapshot / ES: Obtener el snapshot mas reciente
	const FPGXGCSnapshot* GetLatestSnapshot() const
	{
		if (Count == 0) return nullptr;
		const int32 Idx = (Head - 1 + MaxCycles) % MaxCycles;
		return &Snapshots[Idx];
	}

private:
	void UpdateAggregates()
	{
		if (Count == 0) return;

		double SumDuration = 0.0;
		double SumDelta = 0.0;
		double SumPK = 0.0;

		for (int32 i = 0; i < Count; ++i)
		{
			const int32 Idx = (Head - 1 - i + MaxCycles) % MaxCycles;
			SumDuration += Diffs[Idx].DurationSeconds;
			SumDelta += static_cast<double>(Diffs[Idx].DeltaTotalUObject);
			SumPK += static_cast<double>(Snapshots[Idx].PendingKillCount);
		}

		MovingAvgDuration = SumDuration / Count;
		MovingAvgDelta = SumDelta / Count;
		MovingAvgPendingKill = SumPK / Count;
	}
};

/**
 * EN: Baseline measurement for drift detection.
 * ES: Medicion baseline para deteccion de drift.
 */
USTRUCT(BlueprintType)
struct PGXMGOSRUNTIME_API FPGXGCBaseline
{
	GENERATED_BODY()

	/** EN: Whether baseline is valid / ES: Si el baseline es valido */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	bool bValid = false;

	/** EN: Cycle at which baseline was captured / ES: Ciclo en que se capturo el baseline */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 BaselineCycleID = 0;

	/** EN: Timestamp of baseline capture / ES: Marca temporal de captura del baseline */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	double Timestamp = 0.0;

	/** EN: Total UObject count at baseline / ES: Conteo total de UObjects en baseline */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 TotalUObjectCount = 0;

	/** EN: PendingKill at baseline / ES: PendingKill en baseline */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 PendingKillCount = 0;

	/** EN: Process memory at baseline in MB / ES: Memoria del proceso en baseline en MB */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	float BaselineProcessMemoryMB = 0.0f;

	/** EN: Per-class counts for TrackedClasses at baseline / ES: Conteos por clase para TrackedClasses en baseline */
	UPROPERTY()
	TMap<FName, int32> ClassCounts;

	/** EN: Current baseline state / ES: Estado actual del baseline */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	EPGXGCBaselineState BaselineState = EPGXGCBaselineState::Uninitialized;
};

/**
 * EN: Current behavioral profile of GC activity.
 * ES: Perfil comportamental actual de la actividad GC.
 */
USTRUCT(BlueprintType)
struct PGXMGOSRUNTIME_API FPGXGCProfile
{
	GENERATED_BODY()

	/** EN: Current classified state / ES: Estado clasificado actual */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	EPGXGCProfileState CurrentState = EPGXGCProfileState::Stable;

	/** EN: Confidence in current classification (0.0 - 1.0) / ES: Confianza en la clasificacion actual (0.0 - 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	float Confidence = 0.0f;

	/** EN: Number of consecutive cycles in current state / ES: Ciclos consecutivos en estado actual */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int32 CyclesInState = 0;
};

/**
 * EN: A notable GC event that warrants attention.
 * ES: Un evento GC notable que requiere atencion.
 */
USTRUCT(BlueprintType)
struct PGXMGOSRUNTIME_API FPGXGCIncident
{
	GENERATED_BODY()

	/** EN: Cycle at which incident was detected / ES: Ciclo en que se detecto el incidente */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 CycleID = 0;

	/** EN: Severity of the incident / ES: Severidad del incidente */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	EPGXGCSeverity Severity = EPGXGCSeverity::Info;

	/** EN: Category tag name (e.g. LeakSuspected, BurstClean) / ES: Nombre de tag de categoria */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	FName Category;

	/** EN: Human-readable description / ES: Descripcion legible */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	FString Description;
};

/**
 * EN: Health report for a specific tracked class.
 * ES: Reporte de salud para una clase rastreada especifica.
 */
USTRUCT(BlueprintType)
struct PGXMGOSRUNTIME_API FPGXGCClassHealthReport
{
	GENERATED_BODY()

	/** EN: Class name / ES: Nombre de la clase */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	FName ClassName;

	/** EN: Current instance count / ES: Conteo actual de instancias */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 CurrentCount = 0;

	/** EN: Baseline count for this class / ES: Conteo baseline para esta clase */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	int64 BaselineCount = 0;

	/** EN: Growth slope (instances per cycle) / ES: Pendiente de crecimiento (instancias por ciclo) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	float GrowthSlope = 0.0f;

	/** EN: Ratio of current count over baseline / ES: Ratio del conteo actual sobre baseline */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	float OverBaselineRatio = 0.0f;

	/** EN: Peak deviation from baseline / ES: Desviacion pico del baseline */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	float PeakDeviation = 0.0f;

	/** EN: True if count never returns to baseline / ES: True si el conteo nunca regresa al baseline */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	bool bNoReturn = false;

	/** EN: Health state classification / ES: Clasificacion del estado de salud */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|MGOS")
	EPGXGCClassHealth HealthState = EPGXGCClassHealth::Stable;
};

// ============================================================================
// Dynamic Delegates (BP-compatible)
// ============================================================================

/** EN: Fired when a GC cycle starts / ES: Se dispara cuando un ciclo GC inicia */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXGCStarted, int64, CycleID);

/** EN: Fired when a GC cycle completes with diff / ES: Se dispara cuando un ciclo GC completa con diff */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXGCCompleted, int64, CycleID, const FPGXGCSnapshotDiff&, Diff);

/** EN: Fired when profile state changes / ES: Se dispara cuando cambia el estado del perfil */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXGCProfileStateChanged, EPGXGCProfileState, NewState, EPGXGCProfileState, OldState);

/** EN: Fired when an incident is raised / ES: Se dispara cuando se genera un incidente */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXGCIncidentRaised, const FPGXGCIncident&, Incident);
