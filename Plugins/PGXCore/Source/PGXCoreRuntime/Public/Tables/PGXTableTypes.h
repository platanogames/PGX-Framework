// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "PGXTableTypes.generated.h"

class UPGXDataAsset;

// ═══════════════════════════════════════════════════════════════
// EN: DataTable row structs for PGX Registry v2.0
// ES: Structs de fila DataTable para PGX Registry v2.0
// ═══════════════════════════════════════════════════════════════

/**
 * EN: Primary row struct for DataTable-first registry authoring.
 *     Each row represents one category within a database, containing
 *     a map of items keyed by GameplayTag with soft references to DataAssets.
 *     Soft refs avoid forced asset load during table parse.
 *
 * ES: Struct de fila principal para autoria DataTable-first del registro.
 *     Cada fila representa una categoria dentro de una base de datos, conteniendo
 *     un mapa de items indexados por GameplayTag con referencias soft a DataAssets.
 *     Las refs soft evitan carga forzada de assets durante el parseo de la tabla.
 *
 * Constraints / Restricciones:
 *   1. CategoryTag must be valid and from approved roots.
 *   2. No duplicate CategoryTag rows in same DataTable.
 *   3. No duplicate ItemTag within a category map.
 *   4. Soft object references only — no hard refs.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXRegistryCategoryRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Category identifier for this row / ES: Identificador de categoria para esta fila */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry",
		meta = (Categories = "PGX.Category"))
	FGameplayTag CategoryTag;

	/**
	 * EN: Items in this category — key is the item tag, value is a soft ref
	 *     to the DataAsset. Soft refs ensure zero asset loads during table parse.
	 * ES: Items en esta categoria — la clave es el tag del item, el valor es una
	 *     ref soft al DataAsset. Refs soft aseguran cero cargas durante parseo.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	TMap<FGameplayTag, TSoftObjectPtr<UPGXDataAsset>> ItemsByTag;
};
