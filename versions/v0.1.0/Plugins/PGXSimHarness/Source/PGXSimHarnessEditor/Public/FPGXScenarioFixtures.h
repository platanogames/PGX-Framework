// Copyright PGX Framework. All Rights Reserved.
//
// EN: Live Level A / Level B scenarios are runtime fixtures spawned via SpawnActor
//     into a caller-supplied UWorld, so they do not require authored .umap assets. Level A = light scenario (ground marker + spawn
//     points + camera rails + NPC waypoints). Level B = stress scenario (denser
//     geometry + simulated streaming-zone markers). The A->B "transition" is
//     teardown-A + spawn-B in the same world; UPGXLevelFlowSubsystem is smoke-checked
//     (presence + queryable state) rather than driven through a real .umap OpenLevel,
//     because no level tag resolves to a real map in this harness.
//
//     This type is independent of its simulation controller: every entry point
//     takes the UWorld explicitly, so the controller (or a console command / test)
//     owns world resolution and simply calls these fixtures.
//
// ES: Los escenarios Live Level A / Level B son fixtures runtime via SpawnActor
//     sobre un UWorld dado por el llamador y no requieren assets .umap. La transición A->B es teardown-A +
//     spawn-B en el mismo world; el subsistema LevelFlow se smoke-chequea (presencia +
//     estado), no se conduce un OpenLevel real de .umap. Independiente del controller
//     de tool-specific: cada método recibe el UWorld explícito.
//

#pragma once

#include "CoreMinimal.h"

class UWorld;
class AActor;

/**
 * EN: Spawns and tears down the Level A / Level B scenario fixtures for the Live
 *     full-simulation harness. Spawned actors are tracked as weak pointers so Teardown
 *     destroys exactly what was spawned, without leaking or double-freeing.
 * ES: Spawnea y destruye los fixtures de escenario Level A / Level B del harness de
 *     simulación completa Live. Actores trackeados como weak pointers.
 */
class PGXSIMHARNESSEDITOR_API FPGXScenarioFixtures
{
public:
	// ─── Level A (light) ───

	/**
	 * EN: Spawn Level A: 1 ground marker, spawn points for waves, camera rail points,
	 *     NPC waypoints. Returns the number of actors actually spawned.
	 * ES: Spawnea Level A. Devuelve el número de actores spawneados.
	 */
	int32 SpawnLevelA(UWorld* World);

	/** EN: Destroy all Level A actors. ES: Destruye todos los actores de Level A. */
	void TeardownLevelA();

	// ─── Level B (stress) ───

	/**
	 * EN: Transition A->B: tears down Level A, smoke-checks the LevelFlow subsystem,
	 *     then spawns denser Level B geometry + simulated streaming-zone markers.
	 *     Returns the number of Level B actors spawned.
	 * ES: Transición A->B: teardown A, smoke-check LevelFlow, spawnea Level B.
	 */
	int32 SpawnLevelB(UWorld* World);

	/** EN: Destroy all Level B actors. ES: Destruye todos los actores de Level B. */
	void TeardownLevelB();

	// ─── Full teardown ───

	/** EN: Destroy everything spawned by either level. ES: Destruye todo lo spawneado. */
	void TeardownAll();

	// ─── Query (verification / console status) ───

	int32 GetLevelAActorCount() const;
	int32 GetLevelBActorCount() const;
	bool IsLevelASpawned() const { return GetLevelAActorCount() > 0; }
	bool IsLevelBSpawned() const { return GetLevelBActorCount() > 0; }

	/**
	 * EN: Whether the LevelFlow subsystem responded to the last A->B smoke check
	 *     (present + queryable). False if no GameInstance/subsystem was available.
	 * ES: Si el subsistema LevelFlow respondió al último smoke check A->B.
	 */
	bool DidLevelFlowRespond() const { return bLevelFlowResponded; }

private:
	/**
	 * EN: Spawn one lightweight marker AActor at Location, label it (editor only), and
	 *     track it in OutList. Returns the spawned actor (or nullptr on failure).
	 * ES: Spawnea un marcador AActor lightweight en Location, lo etiqueta y lo trackea.
	 */
	AActor* SpawnMarker(UWorld* World, const FVector& Location, const TCHAR* Label,
		TArray<TWeakObjectPtr<AActor>>& OutList);

	/** EN: Destroy every still-valid actor in List, then empty it. ES: Destruye y vacía. */
	static void DestroyTracked(TArray<TWeakObjectPtr<AActor>>& List);

	/**
	 * EN: Smoke-check the LevelFlow subsystem: resolve it from the world's GameInstance
	 *     and query its state. No real OpenLevel (no .umap tag resolves here). Sets
	 *     bLevelFlowResponded.
	 * ES: Smoke-check del subsistema LevelFlow (presencia + estado), sin OpenLevel real.
	 */
	void SmokeCheckLevelFlow(UWorld* World);

	/** EN: Count still-valid actors in a tracked list. ES: Cuenta actores válidos. */
	static int32 CountValid(const TArray<TWeakObjectPtr<AActor>>& List);

	TArray<TWeakObjectPtr<AActor>> LevelAActors;
	TArray<TWeakObjectPtr<AActor>> LevelBActors;
	bool bLevelFlowResponded = false;
};
