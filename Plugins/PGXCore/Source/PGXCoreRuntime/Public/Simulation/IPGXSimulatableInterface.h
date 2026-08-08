// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Simulation/PGXSimulationTypes.h"
#include "IPGXSimulatableInterface.generated.h"

/**
 * EN: Contract for PGX systems that expose deterministic simulation and profiling hooks.
 *     Any UGameInstanceSubsystem (or other UObject) can implement this interface. A
 *     project-owned controller may discover or register implementations through its own
 *     lifecycle without adding feature-to-feature dependencies.
 *
 * ES: Contrato para sistemas PGX que exponen hooks deterministas de simulacion y profiling.
 *     Cualquier UGameInstanceSubsystem (u otro UObject) puede implementar esta interfaz. Un
 *     controlador del proyecto puede descubrir o registrar implementaciones mediante su
 *     propio lifecycle sin anadir dependencias entre plugins funcionales.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UPGXSimulatableInterface : public UInterface
{
	GENERATED_BODY()
};

class PGXCORERUNTIME_API IPGXSimulatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * EN: Called when the measurement phase starts. The system should begin
	 *     tracking its internal metrics and adapt to the simulation context
	 *     (e.g., stress mode, deterministic seed, agent count).
	 *
	 * ES: Llamado cuando inicia la fase de medicion. El sistema debe comenzar
	 *     a rastrear sus metricas internas y adaptarse al contexto de simulacion
	 *     (e.g., modo estres, seed determinista, conteo de agentes).
	 *
	 * @param Context  EN: Read-only simulation context with profile configuration / ES: Contexto read-only de simulacion con configuracion del perfil
	 */
	virtual void OnSimulationStart(const FPGXSimulationContext& Context) = 0;

	/**
	 * EN: Called when the simulation ends (before teardown). The system should
	 *     stop tracking metrics and prepare for GetSimulationMetrics() collection.
	 *
	 * ES: Llamado cuando la simulacion termina (antes del teardown). El sistema debe
	 *     dejar de rastrear metricas y prepararse para la recoleccion GetSimulationMetrics().
	 */
	virtual void OnSimulationStop() = 0;

	/**
	 * EN: Called every simulation tick during the measurement phase.
	 *     Systems can use this for periodic operations specific to the simulation
	 *     (e.g., injecting synthetic save/load operations during stress tests).
	 *
	 * ES: Llamado cada tick de simulacion durante la fase de medicion.
	 *     Los sistemas pueden usar esto para operaciones periodicas especificas de la simulacion
	 *     (e.g., inyectar operaciones sinteticas de save/load durante tests de estres).
	 *
	 * @param DeltaTime  EN: Time since last simulation tick / ES: Tiempo desde el ultimo tick de simulacion
	 */
	virtual void OnSimulationTick(float DeltaTime) = 0;

	/**
	 * EN: Reports whether this system is ready to begin measurement.
	 *     Called during the ReadyValidation phase. The Harness waits for ALL
	 *     registered systems to return true before starting measurement.
	 *
	 *     Examples of "not ready": config not loaded, async init pending,
	 *     warm-up in progress, dependencies not resolved.
	 *
	 * ES: Reporta si este sistema esta listo para comenzar la medicion.
	 *     Llamado durante la fase ReadyValidation. El Harness espera que TODOS
	 *     los sistemas registrados retornen true antes de iniciar la medicion.
	 *
	 *     Ejemplos de "no listo": config no cargada, init async pendiente,
	 *     warm-up en progreso, dependencias no resueltas.
	 */
	virtual bool IsReadyForSimulation() const = 0;

	/**
	 * EN: Returns current metrics snapshot for this system.
	 *     Called by the Harness during teardown to collect per-system metrics
	 *     for the final report. Can also be called mid-simulation for live updates.
	 *
	 *     Systems should populate:
	 *     - AverageTickMs, PeakTickMs: from internal timing
	 *     - OperationCount, ErrorCount: from internal counters
	 *     - MemoryUsedBytes: from internal tracking or platform APIs
	 *     - CustomMetrics: system-specific named values
	 *
	 * ES: Retorna snapshot actual de metricas para este sistema.
	 *     Llamado por el Harness durante teardown para recolectar metricas por sistema
	 *     para el reporte final. Tambien puede ser llamado mid-simulacion para updates en vivo.
	 *
	 *     Los sistemas deben poblar:
	 *     - AverageTickMs, PeakTickMs: de timing interno
	 *     - OperationCount, ErrorCount: de contadores internos
	 *     - MemoryUsedBytes: de tracking interno o APIs de plataforma
	 *     - CustomMetrics: valores nombrados especificos del sistema
	 */
	virtual FPGXSystemMetrics GetSimulationMetrics() const = 0;

	/**
	 * EN: Returns the system's identifying gameplay tag.
	 *     Used by the Harness to match this system against the profile's
	 *     SystemsToActivate filter. Only systems whose tag is in the profile's
	 *     list will receive OnSimulationStart/Tick/Stop calls.
	 *
	 *     Convention: PGX.System.<Name> (e.g., PGX.System.Save, PGX.System.PSO)
	 *
	 * ES: Retorna el gameplay tag identificador del sistema.
	 *     Usado por el Harness para emparejar este sistema contra el filtro
	 *     SystemsToActivate del perfil. Solo sistemas cuyo tag esta en la lista
	 *     del perfil recibiran llamadas OnSimulationStart/Tick/Stop.
	 *
	 *     Convencion: PGX.System.<Name> (e.g., PGX.System.Save, PGX.System.PSO)
	 */
	virtual FGameplayTag GetSimulatableSystemTag() const = 0;
};
