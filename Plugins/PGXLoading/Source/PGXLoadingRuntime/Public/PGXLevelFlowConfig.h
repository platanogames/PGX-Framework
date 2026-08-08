// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Trace/PGXTraceTypes.h"
#include "PGXLevelFlowTypes.h"
#include "PGXLevelFlowConfig.generated.h"

/**
 * EN: Global configuration for the LevelFlow system.
 *     One per project — auto-discovered via AssetRegistry.
 *     Defines default timing, GameFlow integration settings, and limits.
 *
 * ES: Configuracion global para el sistema LevelFlow.
 *     Uno por proyecto — auto-descubierto via AssetRegistry.
 *     Define timing por defecto, integracion con GameFlow, y limites.
 */
UCLASS(BlueprintType)
class PGXLOADINGRUNTIME_API UPGXLevelFlowConfig : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:
	// ── Defaults / Por Defecto ──

	/** EN: Default timing for levels that don't specify their own / ES: Timing por defecto para niveles que no especifican el suyo */
	UPROPERTY(EditAnywhere, Category = "Defaults")
	FPGXTransitionTiming DefaultTiming;

	/** EN: Default visual transition mode / ES: Modo de transicion visual por defecto */
	UPROPERTY(EditAnywhere, Category = "Defaults")
	EPGXTransitionMode DefaultTransitionMode = EPGXTransitionMode::FadeOut_FadeIn;

	// ── GameFlow / GameFlow ──

	/** EN: If true, subsystem auto-sets GameFlow states during transitions / ES: Si true, el subsistema auto-establece estados GameFlow durante transiciones */
	UPROPERTY(EditAnywhere, Category = "GameFlow")
	bool bAutoIntegrateGameFlow = true;

	/** EN: GameFlow tag set on Global channel during loading (e.g., PGX.Flow.State.Loading) / ES: Tag GameFlow a setear en canal Global durante carga */
	UPROPERTY(EditAnywhere, Category = "GameFlow", meta = (AdvancedDisplay, EditCondition = "bAutoIntegrateGameFlow"))
	FGameplayTag GameFlowLoadingTag;

	// ── Limits / Limites ──

	/** EN: Max concurrent sub-level streaming operations / ES: Max operaciones concurrentes de streaming de sub-niveles */
	UPROPERTY(EditAnywhere, Category = "Limits", meta = (AdvancedDisplay, ClampMin = "1"))
	int32 MaxConcurrentSubLevelLoads = 3;

	/** EN: Max transition history entries to retain / ES: Max entradas de historial de transiciones a retener */
	UPROPERTY(EditAnywhere, Category = "Limits", meta = (AdvancedDisplay, ClampMin = "1"))
	int32 MaxHistoryDepth = 50;

	// ── Traceability / Trazabilidad ──

	UPROPERTY(EditAnywhere, Category = "PGX|Trace", meta = (AdvancedDisplay))
	FPGXTraceConfig TraceConfig;
};
