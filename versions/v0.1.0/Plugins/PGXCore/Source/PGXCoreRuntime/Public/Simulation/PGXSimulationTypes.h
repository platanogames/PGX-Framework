// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXSimulationTypes.generated.h"

// ============================================================================
// EN: Simulation Types for PGX Simulation & Profiling Harness (PSPH)
//     Core types that live in PGXCoreRuntime so ANY L2 subsystem can reference
//     them without depending on the Harness plugin.
//
//     These types define the shared vocabulary between the Harness and
//     participating systems: lifecycle phases, profile categories,
//     simulation context, and per-system metrics.
//
// ES: Tipos de Simulacion para el Arnes de Simulacion y Profiling PGX (PSPH)
//     Tipos core que viven en PGXCoreRuntime para que CUALQUIER subsistema L2
//     pueda referenciarlos sin depender del plugin Harness.
//
//     Estos tipos definen el vocabulario compartido entre el Harness y los
//     sistemas participantes: fases del ciclo de vida, categorias de perfil,
//     contexto de simulacion, y metricas por sistema.
// ============================================================================

// ── Enums ───────────────────────────────────────────────────────────────────

/**
 * EN: Current phase of the simulation lifecycle.
 *     Follows a linear progression: Idle → Loading → Warmup → ReadyValidation → Measuring → Teardown → Complete
 *     Can jump to Failed from any active phase.
 *
 * ES: Fase actual del ciclo de vida de la simulacion.
 *     Sigue una progresion lineal: Idle → Loading → Warmup → ReadyValidation → Measuring → Teardown → Complete
 *     Puede saltar a Failed desde cualquier fase activa.
 */
UENUM(BlueprintType)
enum class EPGXSimulationPhase : uint8
{
	/** EN: No simulation active / ES: No hay simulacion activa */
	Idle,

	/** EN: Loading profile and configuring systems / ES: Cargando perfil y configurando sistemas */
	Loading,

	/** EN: Stabilization period — no metrics collected / ES: Periodo de estabilizacion — no se recolectan metricas */
	Warmup,

	/** EN: Waiting for all systems to report IsReadyForSimulation() / ES: Esperando que todos los sistemas reporten IsReadyForSimulation() */
	ReadyValidation,

	/** EN: Active measurement — telemetry ON, Insights ON / ES: Medicion activa — telemetria ON, Insights ON */
	Measuring,

	/** EN: Cleanup, metric collection, and export / ES: Limpieza, recoleccion de metricas y exportacion */
	Teardown,

	/** EN: Simulation finished successfully / ES: Simulacion terminada exitosamente */
	Complete,

	/** EN: Simulation failed (timeout, error, system not ready) / ES: Simulacion fallida (timeout, error, sistema no listo) */
	Failed
};

/**
 * EN: Category of simulation profile. Determines how the Harness orchestrates
 *     the simulation and what kind of report is generated.
 *
 *     Technical: Single-system isolated test (e.g., "Save throughput under load")
 *     GameSimulation: Multi-system realistic game patterns (e.g., "RPG session")
 *     Stress: Maximum load stress test across one or more systems
 *     Custom: User-defined category for project-specific needs
 *
 * ES: Categoria de perfil de simulacion. Determina como el Harness orquesta
 *     la simulacion y que tipo de reporte se genera.
 *
 *     Technical: Test aislado de sistema individual
 *     GameSimulation: Patrones de juego realistas multi-sistema
 *     Stress: Test de estres con carga maxima
 *     Custom: Categoria definida por el usuario
 */
UENUM(BlueprintType)
enum class EPGXSimProfileCategory : uint8
{
	/** EN: Single-system isolated test / ES: Test aislado de sistema individual */
	Technical,

	/** EN: Multi-system game pattern simulation / ES: Simulacion de patron de juego multi-sistema */
	GameSimulation,

	/** EN: Maximum load stress test / ES: Test de estres con carga maxima */
	Stress,

	/** EN: User-defined category / ES: Categoria definida por el usuario */
	Custom
};

// ── Structs ─────────────────────────────────────────────────────────────────

/**
 * EN: Read-only context passed to all simulatable systems when a simulation is active.
 *     Systems use this to understand what kind of simulation is running and adapt
 *     their behavior accordingly (e.g., stress mode, deterministic seed, agent count).
 *
 *     The Harness populates this from the UPGXSimulationProfile DA and updates
 *     ElapsedTime/CurrentPhase as the simulation progresses.
 *
 * ES: Contexto read-only pasado a todos los sistemas simulables cuando una simulacion esta activa.
 *     Los sistemas usan esto para entender que tipo de simulacion esta corriendo y adaptar
 *     su comportamiento (e.g., modo estres, seed determinista, conteo de agentes).
 *
 *     El Harness puebla esto desde el DA UPGXSimulationProfile y actualiza
 *     ElapsedTime/CurrentPhase a medida que la simulacion progresa.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXSimulationContext
{
	GENERATED_BODY()

	// ── Identity / Identidad ──

	/** EN: Unique tag identifying this simulation profile / ES: Tag unico identificando este perfil de simulacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	FGameplayTag ProfileTag;

	/** EN: Current phase of the simulation / ES: Fase actual de la simulacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	EPGXSimulationPhase CurrentPhase = EPGXSimulationPhase::Idle;

	/** EN: Category of the active profile / ES: Categoria del perfil activo */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	EPGXSimProfileCategory Category = EPGXSimProfileCategory::Technical;

	// ── Timing / Temporalizacion ──

	/** EN: Total measurement duration configured in the profile (seconds) / ES: Duracion total de medicion configurada en el perfil (segundos) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	float SimulationDuration = 60.0f;

	/** EN: Warmup period before measurement starts (seconds) / ES: Periodo de calentamiento antes de que inicie la medicion (segundos) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	float WarmupDuration = 10.0f;

	/** EN: Time elapsed since measurement started (seconds) / ES: Tiempo transcurrido desde que inicio la medicion (segundos) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	float ElapsedTime = 0.0f;

	// ── Stress / Estres ──

	/** EN: Whether stress mode is active (higher load) / ES: Si el modo estres esta activo (carga mayor) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	bool bStressMode = false;

	/** EN: Multiplier for tick-based operations (1.0 = normal, 2.0 = double rate) / ES: Multiplicador para operaciones basadas en tick (1.0 = normal, 2.0 = doble rate) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	float TickMultiplier = 1.0f;

	/** EN: Number of simulated agents/players for load testing / ES: Numero de agentes/jugadores simulados para pruebas de carga */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	int32 SimulatedAgents = 0;

	// ── Determinism / Determinismo ──

	/** EN: Whether deterministic mode is enabled (fixed seed, fixed delta) / ES: Si el modo determinista esta habilitado (seed fija, delta fijo) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	bool bDeterministicMode = false;

	/** EN: Random seed for reproducible simulations / ES: Seed aleatoria para simulaciones reproducibles */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	int32 RandomSeed = 0;

	// ── Custom / Personalizado ──

	/** EN: Key-value pairs for profile-specific parameters / ES: Pares clave-valor para parametros especificos del perfil */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	TMap<FName, FString> CustomParams;
};

/**
 * EN: Metrics returned by each participating system during/after simulation.
 *     The Harness collects these from all registered IPGXSimulatableInterface
 *     implementors and aggregates them into the final report.
 *
 *     Systems should populate CustomMetrics with system-specific measurements
 *     (e.g., "SaveOperations", "PSOCompilations", "LogEntries").
 *
 * ES: Metricas retornadas por cada sistema participante durante/despues de la simulacion.
 *     El Harness recolecta estas de todos los implementadores registrados de
 *     IPGXSimulatableInterface y las agrega al reporte final.
 *
 *     Los sistemas deben poblar CustomMetrics con mediciones especificas
 *     (e.g., "SaveOperations", "PSOCompilations", "LogEntries").
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXSystemMetrics
{
	GENERATED_BODY()

	// ── Identity / Identidad ──

	/** EN: Tag identifying which system these metrics belong to / ES: Tag identificando a que sistema pertenecen estas metricas */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	FGameplayTag SystemTag;

	// ── Performance / Rendimiento ──

	/** EN: Average tick time during measurement phase (ms) / ES: Tiempo promedio de tick durante fase de medicion (ms) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	float AverageTickMs = 0.0f;

	/** EN: Peak (worst) tick time during measurement phase (ms) / ES: Tiempo pico (peor) de tick durante fase de medicion (ms) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	float PeakTickMs = 0.0f;

	/** EN: Total operations performed during simulation / ES: Total de operaciones realizadas durante la simulacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	int32 OperationCount = 0;

	/** EN: Number of errors encountered during simulation / ES: Numero de errores encontrados durante la simulacion */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	int32 ErrorCount = 0;

	/** EN: Total CPU time spent in this system (ms) / ES: Tiempo total de CPU gastado en este sistema (ms) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	float TotalTimeMs = 0.0f;

	/** EN: Peak memory used by this system (bytes) / ES: Memoria pico usada por este sistema (bytes) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	int64 MemoryUsedBytes = 0;

	// ── Custom / Personalizado ──

	/** EN: System-specific custom metrics (e.g., "SaveOps" → 142.0, "CacheHitRate" → 0.95) / ES: Metricas personalizadas del sistema (e.g., "SaveOps" → 142.0, "CacheHitRate" → 0.95) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Simulation")
	TMap<FName, float> CustomMetrics;
};
