// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Registry/PGXRegistryTypes.h"

/**
 * EN: Internal typed collection of DataAssets. NOT a USTRUCT — managed by
 *     TUniquePtr inside UPGXDataRegistrySubsystem. Provides O(1) lookup
 *     by ItemTag and O(n) category queries via secondary index.
 *
 * ES: Coleccion tipada interna de DataAssets. NO es USTRUCT — gestionada por
 *     TUniquePtr dentro de UPGXDataRegistrySubsystem. Proporciona lookup O(1)
 *     por ItemTag y queries de categoria O(n) via indice secundario.
 */
struct FPGXDataDatabase
{
	/** EN: Tag identifying this database / ES: Tag que identifica esta base de datos */
	FGameplayTag DatabaseTag;

	/** EN: Expected asset class for type safety / ES: Clase de asset esperada */
	UClass* AssetClass = nullptr;

	/** EN: Primary index: ItemTag → Entry / ES: Indice primario: ItemTag → Entry */
	TMap<FGameplayTag, FPGXRegistryEntry> Entries;

	/** EN: Secondary index: CategoryTag → ItemTags / ES: Indice secundario: CategoryTag → ItemTags */
	TMultiMap<FGameplayTag, FGameplayTag> CategoryIndex;

	/** EN: Whether this database was populated via AssetRegistry scan / ES: Si fue poblada via escaneo de AssetRegistry */
	bool bAutoDiscovered = false;

	// ─── Helpers ───

	int32 GetLoadedCount() const
	{
		int32 Count = 0;
		for (const auto& Pair : Entries)
		{
			if (Pair.Value.CachedAsset != nullptr)
			{
				++Count;
			}
		}
		return Count;
	}

	int32 GetCategoryCount() const
	{
		TSet<FGameplayTag> UniqueCategories;
		for (const auto& Pair : CategoryIndex)
		{
			UniqueCategories.Add(Pair.Key);
		}
		return UniqueCategories.Num();
	}

	FPGXDatabaseStats GetStats() const
	{
		FPGXDatabaseStats Stats;
		Stats.DatabaseTag = DatabaseTag;
		Stats.TotalEntries = Entries.Num();
		Stats.LoadedEntries = GetLoadedCount();
		Stats.CategoryCount = GetCategoryCount();
		Stats.AssetClassName = AssetClass ? AssetClass->GetFName() : NAME_None;
		Stats.bAutoDiscovered = bAutoDiscovered;
		return Stats;
	}
};
