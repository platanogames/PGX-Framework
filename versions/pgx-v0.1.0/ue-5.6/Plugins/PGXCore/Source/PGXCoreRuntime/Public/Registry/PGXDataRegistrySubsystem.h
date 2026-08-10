// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Interfaces/PGXTaggedRegistry.h"
#include "Registry/PGXRegistryTypes.h"
#include "Registry/PGXDataDatabase.h"
#include "PGXDataRegistrySubsystem.generated.h"

class UPGXDataAsset;
class UPGXRegistryDefinition;
class UPGXRegistrySettings;
class UDataTable;

DECLARE_LOG_CATEGORY_EXTERN(LogPGXRegistry, Log, All);

/**
 * EN: Central data asset registry. Organizes DataAssets into typed "databases"
 *     indexed by GameplayTag, with metadata for queries without loading assets.
 *     Provides O(1) lookup, category queries, async loading, and telemetry.
 *
 * ES: Registro central de data assets. Organiza DataAssets en "bases de datos"
 *     tipadas indexadas por GameplayTag, con metadata para queries sin cargar assets.
 *     Proporciona lookup O(1), queries por categoria, carga asincrona y telemetria.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXDataRegistrySubsystem : public UGameInstanceSubsystem, public IPGXTaggedRegistry
{
	GENERATED_BODY()

public:
	// ═══════════════════════════════════════════════════════════════
	// Lifecycle / Ciclo de Vida
	// ═══════════════════════════════════════════════════════════════

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	/** EN: Static cached access / ES: Acceso cacheado estatico */
	static UPGXDataRegistrySubsystem* GetCached();

	// ═══════════════════════════════════════════════════════════════
	// Registration API / API de Registro
	// ═══════════════════════════════════════════════════════════════

	/**
	 * EN: Create a new typed database. If bAutoDiscover is true, performs an
	 *     AssetRegistry scan for all assets of AssetClass and registers them.
	 * ES: Crear una nueva base de datos tipada. Si bAutoDiscover es true, ejecuta
	 *     un escaneo de AssetRegistry para todos los assets de AssetClass y los registra.
	 */
	bool CreateDatabase(FGameplayTag DatabaseTag, UClass* AssetClass, bool bAutoDiscover = false);

	/**
	 * EN: Register an asset in a database. Extracts metadata from the asset.
	 *     The ItemTag is derived from the asset's AssetId if not explicitly provided.
	 * ES: Registrar un asset en una base de datos. Extrae metadata del asset.
	 *     El ItemTag se deriva del AssetId del asset si no se provee explicitamente.
	 */
	bool RegisterAsset(FGameplayTag DatabaseTag, UPGXDataAsset* Asset, FGameplayTag ItemTag);

	/**
	 * EN: Unregister an asset from a database by its ItemTag.
	 * ES: Desregistrar un asset de una base de datos por su ItemTag.
	 */
	bool UnregisterAsset(FGameplayTag DatabaseTag, FGameplayTag ItemTag);

	/**
	 * EN: Check if a database exists.
	 * ES: Verificar si una base de datos existe.
	 */
	bool HasDatabase(FGameplayTag DatabaseTag) const;

	// ═══════════════════════════════════════════════════════════════
	// Query API / API de Consulta
	// ═══════════════════════════════════════════════════════════════

	/**
	 * EN: Find an entry by its ItemTag. Returns nullptr if not found.
	 * ES: Encontrar una entrada por su ItemTag. Retorna nullptr si no se encuentra.
	 */
	const FPGXRegistryEntry* FindEntry(FGameplayTag DatabaseTag, FGameplayTag ItemTag) const;

	/**
	 * EN: Find all entries matching a category.
	 * ES: Encontrar todas las entradas de una categoria.
	 */
	TArray<FPGXRegistryEntry> FindByCategory(FGameplayTag DatabaseTag, FGameplayTag CategoryTag) const;

	/**
	 * EN: Get all entries in a database.
	 * ES: Obtener todas las entradas de una base de datos.
	 */
	TArray<FPGXRegistryEntry> GetAllEntries(FGameplayTag DatabaseTag) const;

	/**
	 * EN: Get a soft reference for deferred loading.
	 * ES: Obtener una referencia soft para carga diferida.
	 */
	FSoftObjectPath GetSoftReference(FGameplayTag DatabaseTag, FGameplayTag ItemTag) const;

	/**
	 * EN: Synchronously resolve an asset. Loads if not cached.
	 * ES: Resolver un asset sincronamente. Carga si no esta en cache.
	 */
	UPGXDataAsset* ResolveAsset(FGameplayTag DatabaseTag, FGameplayTag ItemTag);

	/**
	 * EN: Typed resolve with cast.
	 * ES: Resolve tipado con cast.
	 */
	template<typename T>
	T* ResolveAsset(FGameplayTag DatabaseTag, FGameplayTag ItemTag)
	{
		return Cast<T>(ResolveAsset(DatabaseTag, ItemTag));
	}

	/**
	 * EN: Get all database tags currently registered.
	 * ES: Obtener todos los tags de bases de datos registradas.
	 */
	TArray<FGameplayTag> GetAllDatabaseTags() const;

	/** EN: IPGXTaggedRegistry facade over registered database tags.
	 *  ES: Fachada IPGXTaggedRegistry sobre tags de bases de datos registradas. */
	bool HasEntryByTag(FGameplayTag Tag) const override;
	int32 GetCount() const override;
	void GetSnapshot(TArray<FGameplayTag>& OutTags) const override;

	// ═══════════════════════════════════════════════════════════════
	// Cache / Cache
	// ═══════════════════════════════════════════════════════════════

	/**
	 * EN: Request async load of an asset. Callback fires when loaded.
	 * ES: Solicitar carga asincrona de un asset. Callback se dispara al cargar.
	 */
	void RequestLoad(FGameplayTag DatabaseTag, FGameplayTag ItemTag, FOnPGXAssetLoaded Callback);

	/**
	 * EN: Invalidate all cached assets in a database (releases strong refs).
	 * ES: Invalidar todos los assets cacheados en una base de datos.
	 */
	void InvalidateCache(FGameplayTag DatabaseTag);

	/**
	 * EN: Get a cached asset without triggering a load.
	 * ES: Obtener un asset cacheado sin disparar una carga.
	 */
	UPGXDataAsset* GetCachedAsset(FGameplayTag DatabaseTag, FGameplayTag ItemTag) const;

	// ═══════════════════════════════════════════════════════════════
	// Telemetry / Telemetria
	// ═══════════════════════════════════════════════════════════════

	/**
	 * EN: Get statistics for a specific database.
	 * ES: Obtener estadisticas de una base de datos especifica.
	 */
	FPGXDatabaseStats GetDatabaseStats(FGameplayTag DatabaseTag) const;

	/**
	 * EN: Export metadata for a single database as JSON string.
	 * ES: Exportar metadata de una base de datos como string JSON.
	 */
	FString ExportMetadata(FGameplayTag DatabaseTag) const;

	/**
	 * EN: Export metadata for ALL databases as JSON string.
	 * ES: Exportar metadata de TODAS las bases de datos como string JSON.
	 */
	FString ExportAllMetadata() const;

	// ═══════════════════════════════════════════════════════════════
	// v2.0 DataTable Ingest API / API de Ingesta DataTable v2.0
	// ═══════════════════════════════════════════════════════════════

	/**
	 * EN: Discover and ingest all UPGXRegistryDefinition DAs found via AssetRegistry.
	 *     Called automatically during Initialize() if settings allow.
	 * ES: Descubrir e ingestar todos los DAs UPGXRegistryDefinition encontrados via AssetRegistry.
	 *     Llamado automaticamente durante Initialize() si los settings lo permiten.
	 */
	void IngestAllDefinitions();

	/**
	 * EN: Ingest DataTables from a specific registry definition.
	 * ES: Ingestar DataTables de una definicion de registro especifica.
	 */
	bool IngestDefinition(const UPGXRegistryDefinition* Definition);

	/**
	 * EN: Ingest a single DataTable into a database with a specific conflict policy.
	 * ES: Ingestar un DataTable individual en una BD con una politica de conflicto especifica.
	 */
	bool IngestDataTable(FGameplayTag DatabaseTag, const UDataTable* DataTable,
		EPGXRegistryConflictPolicy Policy = EPGXRegistryConflictPolicy::FailOnConflict);

	// ═══════════════════════════════════════════════════════════════
	// v2.0 Composite Key Query / Consulta por Clave Compuesta v2.0
	// ═══════════════════════════════════════════════════════════════

	/**
	 * EN: O(1) lookup by composite key (Database + Category + Item).
	 * ES: Lookup O(1) por clave compuesta (Database + Category + Item).
	 */
	const FPGXRegistryEntry* FindByCompositeKey(const FPGXCompositeRegistryKey& Key) const;

	/**
	 * EN: Get all entries in a category bucket for a specific database.
	 * ES: Obtener todas las entradas en un bucket de categoria para una BD especifica.
	 */
	TArray<FPGXRegistryEntry> GetCategoryEntries(FGameplayTag DatabaseTag, FGameplayTag CategoryTag) const;

	/**
	 * EN: Get all active registry definitions that have been ingested.
	 * ES: Obtener todas las definiciones activas que han sido ingestadas.
	 */
	TArray<const UPGXRegistryDefinition*> GetActiveDefinitions() const;

	/**
	 * EN: Get total number of entries in the v2.0 global EntryIndex.
	 * ES: Obtener el numero total de entradas en el EntryIndex global v2.0.
	 */
	int32 GetEntryIndexCount() const { return EntryIndex.Num(); }

	/** EN: Get total reverse-index links for performance diagnostics / ES: Conteo del indice inverso */
	int32 GetCompositeReverseIndexCount() const { return CompositeReverseIndex.Num(); }

	// ═══════════════════════════════════════════════════════════════
	// Delegates / Delegados
	// ═══════════════════════════════════════════════════════════════

	// ─── v1.0 Delegates ───
	FOnPGXAssetRegistered OnAssetRegistered;
	FOnPGXAssetUnregistered OnAssetUnregistered;
	FOnPGXDatabaseCreated OnDatabaseCreated;
	FOnPGXCacheInvalidated OnCacheInvalidated;

	// ─── v2.0 Delegates ───
	FOnPGXTablesIngested OnTablesIngested;
	FOnPGXValidationComplete OnValidationComplete;

private:
	// ═══════════════════════════════════════════════════════════════
	// Internal / Interno
	// ═══════════════════════════════════════════════════════════════

	// ─── v1.0 State ───

	/** EN: All databases, keyed by tag / ES: Todas las bases de datos, indexadas por tag */
	TMap<FGameplayTag, TUniquePtr<FPGXDataDatabase>> Databases;

	/** EN: Static cached instance / ES: Instancia cacheada estatica */
	static TWeakObjectPtr<UPGXDataRegistrySubsystem> CachedInstance;

	// ─── v2.0 State ───

	/** EN: Global entry index — O(1) lookup by composite key / ES: Indice global de entradas — lookup O(1) por clave compuesta */
	TMap<FPGXCompositeRegistryKey, FPGXRegistryEntry> EntryIndex;

	/** EN: Reverse index (DatabaseTag, ItemTag) -> CompositeKey(s), avoids O(N) global scans. */
	TMultiMap<FPGXRegistryReverseKey, FPGXCompositeRegistryKey> CompositeReverseIndex;

	/**
	 * EN: Category index — keyed by hash of (DatabaseTag + CategoryTag).
	 *     Each bucket contains all item tags in that category.
	 * ES: Indice de categorias — indexado por hash de (DatabaseTag + CategoryTag).
	 *     Cada bucket contiene todos los tags de items en esa categoria.
	 */
	TMap<uint32, FPGXRegistryCategoryBucket> V2CategoryIndex;

	/** EN: Active definitions that have been ingested / ES: Definiciones activas que han sido ingestadas */
	TArray<TWeakObjectPtr<const UPGXRegistryDefinition>> ActiveDefinitions;

	// ─── Internal Methods ───

	/** EN: Perform AssetRegistry scan for a database (v1.0) / ES: Ejecutar escaneo de AssetRegistry para una BD (v1.0) */
	void AutoDiscoverAssets(FPGXDataDatabase& Database);

	/** EN: Extract entry metadata from an asset / ES: Extraer metadata de entrada desde un asset */
	FPGXRegistryEntry BuildEntryFromAsset(UPGXDataAsset* Asset, FGameplayTag ItemTag) const;

	/** EN: Build category index key from DB + Category tags / ES: Construir clave de indice de categoria desde tags DB + Category */
	static uint32 MakeCategoryKey(const FGameplayTag& DatabaseTag, const FGameplayTag& CategoryTag);

	static FPGXRegistryReverseKey MakeReverseKey(const FGameplayTag& DatabaseTag, const FGameplayTag& ItemTag);
	void AddCompositeIndexEntry(const FPGXCompositeRegistryKey& Key, const FPGXRegistryEntry& Entry, const UDataTable* SourceTable = nullptr);
	void RemoveCompositeIndexEntry(const FPGXCompositeRegistryKey& Key);
	TArray<FPGXCompositeRegistryKey> FindCompositeKeysForItem(const FGameplayTag& DatabaseTag, const FGameplayTag& ItemTag) const;
	void RemoveCompositeIndexesForItem(const FGameplayTag& DatabaseTag, const FGameplayTag& ItemTag);
	void SyncCompositeCachedAsset(const FGameplayTag& DatabaseTag, const FGameplayTag& ItemTag, UPGXDataAsset* Asset);

	/** EN: Console command handles / ES: Handles de comandos de consola */
	TArray<IConsoleObject*> ConsoleCommands;

	/** EN: Register console commands / ES: Registrar comandos de consola */
	void RegisterConsoleCommands();

	/** EN: Unregister console commands / ES: Desregistrar comandos de consola */
	void UnregisterConsoleCommands();

	// ── Profile Integration ──

	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);
};
