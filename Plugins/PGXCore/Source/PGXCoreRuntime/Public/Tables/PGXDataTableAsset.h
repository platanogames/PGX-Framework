// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "PGXDataTableAsset.generated.h"

/**
 * EN: PGX DataTable subclass for registry authoring.
 *     Pre-configures the row struct to FPGXRegistryCategoryRow and adds
 *     a DatabaseTag for self-documentation and discovery.
 *     Designers create these in Content Browser instead of plain UDataTable.
 *
 * ES: Subclase de DataTable PGX para autoria del registro.
 *     Pre-configura el row struct a FPGXRegistryCategoryRow y agrega
 *     un DatabaseTag para auto-documentacion y descubrimiento.
 *     Los disenadores crean estos en Content Browser en vez de UDataTable plano.
 */
UCLASS(BlueprintType)
class PGXCORERUNTIME_API UPGXDataTableAsset : public UDataTable
{
	GENERATED_UCLASS_BODY()

public:
	/**
	 * EN: Optional database tag — if set, subsystem can auto-associate this table
	 *     with a UPGXRegistryDefinition. If empty, the table must be explicitly
	 *     referenced by a definition.
	 * ES: Tag de base de datos opcional — si esta establecido, el subsistema puede
	 *     auto-asociar esta tabla con un UPGXRegistryDefinition. Si esta vacio,
	 *     la tabla debe ser referenciada explicitamente por una definicion.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry",
		meta = (Categories = "PGX.DB"))
	FGameplayTag DatabaseTag;

	/**
	 * EN: Human-readable description for this table.
	 * ES: Descripcion legible para esta tabla.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry",
		meta = (MultiLine = true))
	FText TableDescription;
};
