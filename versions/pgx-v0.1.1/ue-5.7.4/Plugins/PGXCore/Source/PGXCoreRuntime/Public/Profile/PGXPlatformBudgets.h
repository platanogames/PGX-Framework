// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXPlatformBudgets.generated.h"

// ============================================================================
// EN: Per-System Platform Budget Structs.
//     Each struct contains ONLY fields with REAL platform variation.
//     Convention: 0 = unlimited (no enforcement). Non-zero = hard limit.
//     Multi-platform resolution: MIN merge (most restrictive non-zero wins).
//
// ES: Structs de Presupuesto de Plataforma por Sistema.
//     Cada struct contiene SOLO campos con variacion REAL entre plataformas.
//     Convencion: 0 = ilimitado (sin enforcement). No-cero = limite duro.
//     Resolucion multi-plataforma: merge MIN (el no-cero mas restrictivo gana).
// ============================================================================

// ── Audio ───────────────────────────────────────────────────────────────────

/**
 * EN: Audio system platform budgets.
 *     Switch: 32MB/24/32. PS5: 128MB/64/96. PC: unlimited.
 * ES: Presupuestos de plataforma del sistema de Audio.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXAudioPlatformBudgets
{
	GENERATED_BODY()

	/** EN: Audio memory budget in MB (0 = unlimited) / ES: Presupuesto de memoria de audio en MB (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Audio", meta = (ClampMin = "0"))
	int32 AudioMemory_MB = 0;

	/** EN: Max concurrent sound instances (0 = unlimited) / ES: Instancias de sonido concurrentes maximas (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Audio", meta = (ClampMin = "0"))
	int32 MaxConcurrentSounds = 0;

	/** EN: Sound pool max size (0 = unlimited) / ES: Tamano maximo del pool de sonido (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Audio", meta = (ClampMin = "0"))
	int32 SoundPoolMaxSize = 0;
};

// ── Save ────────────────────────────────────────────────────────────────────

/**
 * EN: Save system platform budgets.
 *     Switch TCR, PS5 TRC, mobile storage limits.
 * ES: Presupuestos de plataforma del sistema de Save.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXSavePlatformBudgets
{
	GENERATED_BODY()

	/** EN: Max single save file size in KB (0 = unlimited) / ES: Tamano maximo de archivo de guardado en KB (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Save", meta = (ClampMin = "0"))
	int32 MaxSaveFileSize_KB = 0;

	/** EN: Max total storage in MB (0 = unlimited) / ES: Almacenamiento total maximo en MB (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Save", meta = (ClampMin = "0"))
	int32 MaxTotalStorage_MB = 0;

	/** EN: Max concurrent I/O operations (0 = unlimited) / ES: Operaciones I/O concurrentes maximas (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Save", meta = (ClampMin = "0"))
	int32 MaxConcurrentIO = 0;
};

// ── PSO ─────────────────────────────────────────────────────────────────────

/**
 * EN: PSO system platform budgets.
 *     Switch CPU limits shader compilation budget.
 * ES: Presupuestos de plataforma del sistema PSO.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXPSOPlatformBudgets
{
	GENERATED_BODY()

	/** EN: Max shader permutations in warm-up (0 = unlimited) / ES: Permutaciones maximas de shader en warm-up (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|PSO", meta = (ClampMin = "0"))
	int32 MaxShaderPermutations = 0;

	/** EN: Max warm-up entries (0 = unlimited) / ES: Entradas maximas de warm-up (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|PSO", meta = (ClampMin = "0"))
	int32 MaxWarmUpEntries = 0;

	/** EN: Warm-up time budget in ms per frame (0 = unlimited) / ES: Presupuesto de tiempo de warm-up en ms por frame (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|PSO", meta = (ClampMin = "0"))
	float WarmUpTimeBudgetMs = 0.f;
};

// ── Loading ─────────────────────────────────────────────────────────────────

/**
 * EN: Loading system platform budgets.
 *     I/O throughput (NVMe vs SD card), cert requirements.
 * ES: Presupuestos de plataforma del sistema de Loading.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXLoadingPlatformBudgets
{
	GENERATED_BODY()

	/** EN: Max concurrent async load operations (0 = unlimited) / ES: Operaciones de carga asincrona concurrentes maximas (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Loading", meta = (ClampMin = "0"))
	int32 MaxConcurrentAsyncLoads = 0;

	/** EN: Minimum loading screen duration in seconds for cert compliance (0 = no minimum) / ES: Duracion minima de pantalla de carga en segundos para compliance (0 = sin minimo) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Loading", meta = (ClampMin = "0"))
	float MinLoadingScreenDuration = 0.f;
};

// ── LevelFlow ───────────────────────────────────────────────────────────────

/**
 * EN: LevelFlow system platform budgets.
 *     Switch 4GB RAM limits streaming pool and sub-level count.
 * ES: Presupuestos de plataforma del sistema LevelFlow.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXLevelFlowPlatformBudgets
{
	GENERATED_BODY()

	/** EN: Streaming pool budget in MB (0 = unlimited) / ES: Presupuesto de streaming pool en MB (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|LevelFlow", meta = (ClampMin = "0"))
	int32 StreamingPool_MB = 0;

	/** EN: Max simultaneously loaded sub-levels (0 = unlimited) / ES: Sub-niveles cargados simultaneamente maximos (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|LevelFlow", meta = (ClampMin = "0"))
	int32 MaxLoadedSubLevels = 0;
};

// ── Log ─────────────────────────────────────────────────────────────────────

/**
 * EN: Log system platform budgets.
 *     RAM consumption and disk usage on limited storage platforms.
 * ES: Presupuestos de plataforma del sistema de Log.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXLogPlatformBudgets
{
	GENERATED_BODY()

	/** EN: Ring buffer capacity in entries (0 = unlimited) / ES: Capacidad del ring buffer en entradas (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Log", meta = (ClampMin = "0"))
	int32 RingBufferCapacity = 0;

	/** EN: Max log files on disk (0 = unlimited) / ES: Archivos de log maximos en disco (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Log", meta = (ClampMin = "0"))
	int32 MaxLogFiles = 0;
};

// ── GameFlow ────────────────────────────────────────────────────────────────

/**
 * EN: GameFlow system platform budgets.
 *     Memory per channel and history ring buffer.
 * ES: Presupuestos de plataforma del sistema GameFlow.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXGameFlowPlatformBudgets
{
	GENERATED_BODY()

	/** EN: Max flow channels (0 = unlimited) / ES: Canales de flujo maximos (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|GameFlow", meta = (ClampMin = "0"))
	int32 MaxFlowChannels = 0;

	/** EN: Max transition history entries (0 = unlimited) / ES: Entradas maximas de historial de transiciones (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|GameFlow", meta = (ClampMin = "0"))
	int32 MaxTransitionHistory = 0;
};

// ── MGOS ────────────────────────────────────────────────────────────────────

/**
 * EN: MGOS (GC Observer) system platform budgets.
 *     TObjectIterator CPU cost and memory buffers.
 * ES: Presupuestos de plataforma del sistema MGOS (Observador GC).
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXMGOSPlatformBudgets
{
	GENERATED_BODY()

	/** EN: History window size in samples (0 = unlimited) / ES: Tamano de ventana de historial en muestras (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|MGOS", meta = (ClampMin = "0"))
	int32 HistoryWindowSize = 0;

	/** EN: Max tracked UObject classes (0 = unlimited) / ES: Clases UObject rastreadas maximas (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|MGOS", meta = (ClampMin = "0"))
	int32 MaxTrackedClasses = 0;

	/** EN: Max incident history entries (0 = unlimited) / ES: Entradas maximas de historial de incidentes (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|MGOS", meta = (ClampMin = "0"))
	int32 MaxIncidentHistory = 0;
};

// ── Message ─────────────────────────────────────────────────────────────────

/**
 * EN: Message system platform budgets.
 *     Ring buffer RAM consumption.
 * ES: Presupuestos de plataforma del sistema de Message.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXMessagePlatformBudgets
{
	GENERATED_BODY()

	/** EN: Max message history entries (0 = unlimited) / ES: Entradas maximas de historial de mensajes (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Message", meta = (ClampMin = "0"))
	int32 MaxMessageHistory = 0;
};

// ── EventHandler ────────────────────────────────────────────────────────────

/**
 * EN: EventHandler system platform budgets.
 *     Cache memory and blackbox ring buffer.
 * ES: Presupuestos de plataforma del sistema EventHandler.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXEventHandlerPlatformBudgets
{
	GENERATED_BODY()

	/** EN: Max cached event handlers (0 = unlimited) / ES: Event handlers cacheados maximos (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|EventHandler", meta = (ClampMin = "0"))
	int32 MaxCachedHandlers = 0;

	/** EN: Blackbox buffer size in entries (0 = unlimited) / ES: Tamano del buffer blackbox en entradas (0 = ilimitado) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|EventHandler", meta = (ClampMin = "0"))
	int32 BlackboxBufferSize = 0;
};

// ============================================================================
// Platform Traits — characteristics (not budgets)
// EN: Boolean flags describing platform hardware/software characteristics.
// ES: Flags booleanos que describen caracteristicas de hardware/software de la plataforma.
// ============================================================================

USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXPlatformTraits
{
	GENERATED_BODY()

	/** EN: Platform has custom/sandboxed save paths (consoles = true) / ES: Plataforma tiene rutas de guardado propias/sandboxed (consolas = true) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Traits")
	bool bCustomSavePaths = false;

	/** EN: Platform supports native async I/O (all modern = true) / ES: Plataforma soporta I/O asincrono nativo (todas modernas = true) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Traits")
	bool bAsyncIONative = true;

	/** EN: Platform supports virtual textures / ES: Plataforma soporta texturas virtuales */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Traits")
	bool bSupportsVirtualTextures = true;

	/** EN: Platform supports hardware ray tracing / ES: Plataforma soporta ray tracing por hardware */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Traits")
	bool bSupportsRayTracing = true;

	/** EN: Platform requires certification compliance (console = true) / ES: Plataforma requiere compliance de certificacion (consola = true) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Traits")
	bool bRequiresCertCompliance = false;

	/** EN: Platform has limited storage (Switch/Mobile = true) / ES: Plataforma tiene almacenamiento limitado (Switch/Mobile = true) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Platform|Traits")
	bool bLimitedStorage = false;
};
