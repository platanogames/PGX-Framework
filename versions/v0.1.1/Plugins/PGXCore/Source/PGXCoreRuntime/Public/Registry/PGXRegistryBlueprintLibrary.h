// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Registry/PGXRegistryTypes.h"
#include "PGXRegistryBlueprintLibrary.generated.h"

class UPGXDataAsset;

/**
 * EN: Blueprint function library for PGX Data Registry queries.
 *     Provides static functions accessible from any Blueprint graph.
 * ES: Libreria de funciones Blueprint para consultas al PGX Data Registry.
 *     Proporciona funciones estaticas accesibles desde cualquier grafo Blueprint.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXRegistryBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ═══════════════════════════════════════════════════════════════
	// Query / Consulta
	// ═══════════════════════════════════════════════════════════════

	/** EN: Find an entry by database + item tag / ES: Encontrar una entrada por tag de base de datos + item */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Query", meta = (WorldContext = "WorldContextObject"))
	static FPGXRegistryEntry FindRegistryEntry(const UObject* WorldContextObject, FGameplayTag DatabaseTag, FGameplayTag ItemTag, bool& bFound);

	/** EN: Find all entries matching a category / ES: Encontrar todas las entradas de una categoria */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXRegistryEntry> FindRegistryEntriesByCategory(const UObject* WorldContextObject, FGameplayTag DatabaseTag, FGameplayTag CategoryTag);

	/** EN: Get all entries in a database / ES: Obtener todas las entradas de una base de datos */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXRegistryEntry> GetAllRegistryEntries(const UObject* WorldContextObject, FGameplayTag DatabaseTag);

	/** EN: Resolve (sync load) an asset from registry / ES: Resolver (carga sincrona) un asset del registro */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry", meta = (WorldContext = "WorldContextObject"))
	static UPGXDataAsset* ResolveRegistryAsset(const UObject* WorldContextObject, FGameplayTag DatabaseTag, FGameplayTag ItemTag);

	/**
	 * EN: Resolve an asset with typed output — Blueprint node pin shows the expected class.
	 * ES: Resolver un asset con salida tipada — el pin del nodo Blueprint muestra la clase esperada.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry",
		meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "ExpectedClass",
			DisplayName = "Resolve Registry Asset (Typed)"))
	static UPGXDataAsset* ResolveRegistryAssetTyped(const UObject* WorldContextObject,
		FGameplayTag DatabaseTag, FGameplayTag ItemTag, TSubclassOf<UPGXDataAsset> ExpectedClass);

	/**
	 * EN: Resolve an asset using a single full tag (parses parent as DatabaseTag, leaf as ItemTag).
	 *     Example: "PGX.Registry.Weapons.Sword" → Database=PGX.Registry.Weapons, Item=PGX.Registry.Weapons.Sword
	 * ES: Resolver un asset usando un solo tag completo (parsea padre como DatabaseTag, hoja como ItemTag).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Resolve Asset By Tag"))
	static UPGXDataAsset* ResolveRegistryAssetByTag(const UObject* WorldContextObject, FGameplayTag FullItemTag);

	// ═══════════════════════════════════════════════════════════════
	// Telemetry / Telemetria
	// ═══════════════════════════════════════════════════════════════

	/** EN: Get database statistics / ES: Obtener estadisticas de base de datos */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Debug", meta = (WorldContext = "WorldContextObject"))
	static FPGXDatabaseStats GetRegistryDatabaseStats(const UObject* WorldContextObject, FGameplayTag DatabaseTag);

	/** EN: Check if a database exists / ES: Verificar si existe una base de datos */
	UFUNCTION(BlueprintPure, Category = "PGX|Registry|Query", meta = (WorldContext = "WorldContextObject"))
	static bool HasRegistryDatabase(const UObject* WorldContextObject, FGameplayTag DatabaseTag);

	/** EN: Get all database tags / ES: Obtener todos los tags de bases de datos */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FGameplayTag> GetAllRegistryDatabaseTags(const UObject* WorldContextObject);

	// ═══════════════════════════════════════════════════════════════
	// v2.0 Composite Key Query / Consulta por Clave Compuesta v2.0
	// ═══════════════════════════════════════════════════════════════

	/**
	 * EN: Find an entry by composite key (Database + Category + Item). O(1) lookup.
	 * ES: Encontrar una entrada por clave compuesta (Database + Category + Item). Lookup O(1).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Query", meta = (WorldContext = "WorldContextObject"))
	static FPGXRegistryEntry FindByCompositeKey(const UObject* WorldContextObject,
		FGameplayTag DatabaseTag, FGameplayTag CategoryTag, FGameplayTag ItemTag, bool& bFound);

	/**
	 * EN: Get all entries in a category for a specific database (v2.0 bucket query).
	 * ES: Obtener todas las entradas en una categoria para una BD especifica (query de bucket v2.0).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXRegistryEntry> GetCategoryEntries(const UObject* WorldContextObject,
		FGameplayTag DatabaseTag, FGameplayTag CategoryTag);

	/**
	 * EN: Force re-ingest all registry definitions.
	 * ES: Forzar re-ingesta de todas las definiciones de registro.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Advanced", meta = (WorldContext = "WorldContextObject"))
	static void ReIngestDefinitions(const UObject* WorldContextObject);

	/**
	 * EN: Get the total number of entries in the v2.0 global index.
	 * ES: Obtener el numero total de entradas en el indice global v2.0.
	 */
	UFUNCTION(BlueprintPure, Category = "PGX|Registry|Debug", meta = (WorldContext = "WorldContextObject"))
	static int32 GetTotalIndexedEntries(const UObject* WorldContextObject);
};
