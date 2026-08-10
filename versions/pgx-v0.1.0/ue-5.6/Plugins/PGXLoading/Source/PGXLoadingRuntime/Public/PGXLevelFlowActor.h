// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "PGXLevelFlowActor.generated.h"

/**
 * EN: Level Flow Actor — placed by designers in each level map.
 *     Serves as the level's representative: the bridge between the level's content
 *     and the LevelFlow subsystem. Provides entry points, managed sub-levels,
 *     and Blueprint-implementable events for level lifecycle.
 *
 *     Without it, the subsystem still works (transitions complete normally),
 *     but loses in-level coordination capabilities.
 *
 * ES: Actor de Level Flow — colocado por disenadores en cada mapa de nivel.
 *     Sirve como representante del nivel: el puente entre el contenido del nivel
 *     y el subsistema LevelFlow. Proporciona puntos de entrada, sub-niveles gestionados,
 *     y eventos Blueprint-implementable para el ciclo de vida del nivel.
 *
 *     Sin el, el subsistema sigue funcionando (transiciones se completan normalmente),
 *     pero pierde capacidades de coordinacion in-level.
 */
UCLASS(BlueprintType, Blueprintable)
class PGXLOADINGRUNTIME_API APGXLevelFlowActor : public AActor
{
	GENERATED_BODY()

public:
	APGXLevelFlowActor();

	// ── Identity / Identidad ──

	/** EN: Which level tag this actor represents / ES: Que tag de nivel representa este actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|LevelFlow")
	FGameplayTag LevelTag;

	// ── Entry Points / Puntos de Entrada ──

	/**
	 * EN: Named spawn transforms — used by other systems to know WHERE to place the player.
	 *     Key = entry point name (e.g., "Default", "FromCave", "FromTown").
	 * ES: Transforms de spawn nombrados — usados por otros sistemas para saber DONDE colocar al jugador.
	 *     Key = nombre del punto de entrada (e.g., "Default", "FromCave", "FromTown").
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|LevelFlow")
	TMap<FName, FTransform> EntryPoints;

	// ── Sub-Levels / Sub-Niveles ──

	/** EN: Sub-level tags this actor manages within its level / ES: Tags de sub-nivel que este actor gestiona */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|LevelFlow")
	TArray<FGameplayTag> ManagedSubLevels;

	// ── Events / Eventos ──

	/** EN: Called by subsystem when this level is fully ready (post timing wait) / ES: Llamado por el subsistema cuando el nivel esta completamente listo */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|LevelFlow")
	void OnLevelReady();

	/** EN: Called by subsystem when a transition away from this level starts / ES: Llamado por el subsistema cuando comienza una transicion fuera de este nivel */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|LevelFlow")
	void OnLevelExiting();

	/** EN: Called when a managed sub-level finishes loading / ES: Llamado cuando un sub-nivel gestionado termina de cargar */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|LevelFlow")
	void OnSubLevelLoaded(FGameplayTag SubLevelTag);

	/** EN: Called when a managed sub-level is unloaded / ES: Llamado cuando un sub-nivel gestionado es descargado */
	UFUNCTION(BlueprintImplementableEvent, Category = "PGX|LevelFlow")
	void OnSubLevelUnloaded(FGameplayTag SubLevelTag);

protected:
	//~ Begin AActor Interface
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};
