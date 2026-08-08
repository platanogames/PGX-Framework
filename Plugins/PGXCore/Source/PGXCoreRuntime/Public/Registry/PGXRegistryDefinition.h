// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXDataAsset.h"
#include "Observability/PGXObservable.h"
#include "Registry/PGXRegistryTypes.h"
#include "PGXRegistryDefinition.generated.h"

class UDataTable;

/**
 * EN: DataAsset defining a logical registry database.
 *     Groups DataTables, sets conflict policy, and optionally whitelists categories.
 *     Auto-discovered via AssetRegistry at subsystem Initialize().
 *     Designers create one per database (e.g., DA_RegistryDef_Weapons).
 *
 * ES: DataAsset que define una base de datos logica del registro.
 *     Agrupa DataTables, establece politica de conflicto, y opcionalmente whitelist de categorias.
 *     Auto-descubierto via AssetRegistry al Initialize() del subsistema.
 *     Los disenadores crean uno por base de datos (ej., DA_RegistryDef_Weapons).
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXRegistryDefinition : public UPGXDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable (delegates to PGXCoreObservability helpers)
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/**
	 * EN: Logical database identifier — unique across the project.
	 *     Maps to the DatabaseTag used in all query APIs.
	 * ES: Identificador logico de base de datos — unico en el proyecto.
	 *     Mapea al DatabaseTag usado en todas las APIs de consulta.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry",
		meta = (Categories = "PGX.DB"))
	FGameplayTag DatabaseTag;

	/**
	 * EN: DataTables to ingest for this database. Order matters when
	 *     conflict policy is FirstWins or LastWins.
	 * ES: DataTables a ingestar para esta BD. El orden importa cuando
	 *     la politica de conflicto es FirstWins o LastWins.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	TArray<TSoftObjectPtr<UDataTable>> DataTables;

	/**
	 * EN: Optional category whitelist — if non-empty, only rows with matching
	 *     CategoryTag will be ingested. Empty = accept all categories.
	 * ES: Whitelist opcional de categorias — si no esta vacia, solo filas con
	 *     CategoryTag coincidente seran ingestadas. Vacia = aceptar todas.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry", meta = (AdvancedDisplay))
	TArray<FGameplayTag> AllowedCategories;

	/**
	 * EN: Conflict resolution policy for this database.
	 *     Applies when duplicate composite keys are found across DataTables.
	 * ES: Politica de resolucion de conflictos para esta BD.
	 *     Se aplica cuando se encuentran claves compuestas duplicadas entre DataTables.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	EPGXRegistryConflictPolicy ConflictPolicy = EPGXRegistryConflictPolicy::FailOnConflict;

	/**
	 * EN: Expected asset class for type validation. All soft refs in DataTable
	 *     rows should point to assets of this class (or subclasses).
	 * ES: Clase de asset esperada para validacion de tipo. Todas las refs soft
	 *     en filas de DataTable deben apuntar a assets de esta clase (o subclases).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry",
		meta = (AdvancedDisplay, AllowAbstract = false))
	TSubclassOf<UPGXDataAsset> ExpectedAssetClass;

	/**
	 * EN: Human-readable description for this database definition.
	 * ES: Descripcion legible para esta definicion de base de datos.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry",
		meta = (AdvancedDisplay, MultiLine = true))
	FText Description;

	// ─── Helpers / Utilidades ───

	/** EN: Check if a category tag is allowed by this definition / ES: Verificar si un tag de categoria esta permitido */
	bool IsCategoryAllowed(const FGameplayTag& CategoryTag) const
	{
		// EN: Empty whitelist = all allowed / ES: Whitelist vacia = todo permitido
		if (AllowedCategories.Num() == 0)
		{
			return true;
		}
		return AllowedCategories.Contains(CategoryTag);
	}

	/** EN: Check if this definition is valid for ingestion / ES: Verificar si esta definicion es valida para ingestion */
	bool IsValidForIngestion() const
	{
		return DatabaseTag.IsValid() && DataTables.Num() > 0;
	}
};
