// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "PGXLevelFlowTypes.h"
#include "PGXLevelProfile.generated.h"

/**
 * EN: Level catalog DataAsset. Maps GameplayTags to level data.
 *     Multiple profiles can coexist (e.g., DA_Levels_Campaign, DA_Levels_DLC1).
 *     The subsystem merges all discovered profiles at initialization.
 *     Auto-discovered via AssetRegistry scan — same pattern as PGXSave/PGXGameFlow/PGXPSO.
 *
 * ES: DataAsset catalogo de niveles. Mapea GameplayTags a datos de nivel.
 *     Multiples perfiles pueden coexistir (e.g., DA_Levels_Campaign, DA_Levels_DLC1).
 *     El subsistema fusiona todos los perfiles descubiertos en la inicializacion.
 *     Auto-descubierto via AssetRegistry scan — mismo patron que PGXSave/PGXGameFlow/PGXPSO.
 */
UCLASS(BlueprintType)
class PGXLOADINGRUNTIME_API UPGXLevelProfile : public UPGXConfigDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * EN: Map of GameplayTag → level data. Each entry fully describes a loadable level.
	 *     Tags should follow the convention: PGX.Level.<Name> (e.g., PGX.Level.MainMenu, PGX.Level.Gameplay).
	 *
	 * ES: Mapa de GameplayTag → datos de nivel. Cada entrada describe completamente un nivel cargable.
	 *     Los tags deben seguir la convencion: PGX.Level.<Name> (e.g., PGX.Level.MainMenu, PGX.Level.Gameplay).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|LevelFlow")
	TMap<FGameplayTag, FPGXLevelEntry> LevelCatalog;
};
