// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "PGXRegistryTypes.generated.h"

class UPGXDataAsset;

// ═══════════════════════════════════════════════════════════════
// Delegates / Delegados
// ═══════════════════════════════════════════════════════════════

/** EN: Broadcast when an asset is registered in a database / ES: Se emite cuando un asset se registra en una base de datos */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXAssetRegistered, const FGameplayTag& /*DatabaseTag*/, const FGameplayTag& /*ItemTag*/);

/** EN: Broadcast when an asset is unregistered from a database / ES: Se emite cuando un asset se desregistra de una base de datos */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXAssetUnregistered, const FGameplayTag& /*DatabaseTag*/, const FGameplayTag& /*ItemTag*/);

/** EN: Broadcast when a new database is created / ES: Se emite cuando se crea una nueva base de datos */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXDatabaseCreated, const FGameplayTag& /*DatabaseTag*/);

/** EN: Broadcast when a database cache is invalidated / ES: Se emite cuando se invalida el cache de una base de datos */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXCacheInvalidated, const FGameplayTag& /*DatabaseTag*/);

/** EN: Callback for async load completion / ES: Callback para completar carga asincrona */
DECLARE_DELEGATE_TwoParams(FOnPGXAssetLoaded, const FGameplayTag& /*ItemTag*/, UPGXDataAsset* /*LoadedAsset*/);

// ─── v2.0 Delegates / Delegados v2.0 ───

/** EN: Broadcast when DataTables are ingested into a database / ES: Se emite cuando DataTables son ingestados en una BD */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXTablesIngested, const FGameplayTag& /*DatabaseTag*/, int32 /*EntryCount*/);

/** EN: Broadcast when a validation pass completes (editor) / ES: Se emite cuando un pase de validacion completa (editor) */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXValidationComplete, const FGameplayTag& /*DatabaseTag*/, int32 /*IssueCount*/);

// ═══════════════════════════════════════════════════════════════
// Enums (v2.0) / Enumeraciones (v2.0)
// ═══════════════════════════════════════════════════════════════

/**
 * EN: Conflict resolution policy when a composite key (Database+Category+Item)
 *     collides across multiple DataTables within the same registry definition.
 * ES: Politica de resolucion de conflictos cuando una clave compuesta
 *     (Database+Category+Item) colisiona entre multiples DataTables dentro
 *     de la misma definicion de registro.
 */
UENUM(BlueprintType)
enum class EPGXRegistryConflictPolicy : uint8
{
	/** EN: Emit error, reject duplicate / ES: Emitir error, rechazar duplicado */
	FailOnConflict  UMETA(DisplayName = "Fail On Conflict"),

	/** EN: Keep first-registered entry / ES: Mantener primera entrada registrada */
	FirstWins       UMETA(DisplayName = "First Wins"),

	/** EN: Overwrite with latest entry / ES: Sobrescribir con entrada mas reciente */
	LastWins        UMETA(DisplayName = "Last Wins")
};

// ═══════════════════════════════════════════════════════════════
// Structs / Estructuras
// ═══════════════════════════════════════════════════════════════

/**
 * EN: Single entry in a registry database. Contains metadata extracted at
 *     registration time plus a soft reference for deferred loading.
 * ES: Entrada individual en una base de datos del registro. Contiene metadata
 *     extraida en tiempo de registro mas una referencia soft para carga diferida.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXRegistryEntry
{
	GENERATED_BODY()

	/** EN: Primary key tag / ES: Tag clave primaria */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FGameplayTag ItemTag;

	/** EN: Category tag (from UPGXObjectDataAsset or custom) / ES: Tag de categoria */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FGameplayTag CategoryTag;

	/** EN: Asset identifier from UPGXDataAsset::AssetId / ES: Identificador del asset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FName AssetId;

	/** EN: Version from UPGXDataAsset::Version / ES: Version del asset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	int32 Version = 0;

	/** EN: Display name extracted at registration / ES: Nombre para mostrar */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FText DisplayName;

	/** EN: Soft reference path (no forced load) / ES: Ruta de referencia soft */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FSoftObjectPath AssetPath;

	/** EN: Cached strong ref, null until loaded / ES: Referencia fuerte cacheada, null hasta que se carga */
	UPROPERTY(Transient)
	TObjectPtr<UPGXDataAsset> CachedAsset = nullptr;

	bool IsValid() const { return ItemTag.IsValid() && !AssetPath.IsNull(); }
	bool IsLoaded() const { return CachedAsset != nullptr; }
};

/**
 * EN: Statistics for a registry database, used for telemetry.
 * ES: Estadisticas de una base de datos del registro, usadas para telemetria.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXDatabaseStats
{
	GENERATED_BODY()

	/** EN: Database identifier tag / ES: Tag identificador de la base de datos */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FGameplayTag DatabaseTag;

	/** EN: Total registered entries / ES: Entradas registradas totales */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	int32 TotalEntries = 0;

	/** EN: Currently cached (loaded) entries / ES: Entradas actualmente cacheadas */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	int32 LoadedEntries = 0;

	/** EN: Number of unique categories / ES: Numero de categorias unicas */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	int32 CategoryCount = 0;

	/** EN: Asset class name for this database / ES: Nombre de clase de asset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FName AssetClassName;

	/** EN: Whether this database was auto-discovered / ES: Si fue auto-descubierta */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	bool bAutoDiscovered = false;
};

// ═══════════════════════════════════════════════════════════════
// v2.0 Structs / Estructuras v2.0
// ═══════════════════════════════════════════════════════════════

/**
 * EN: Three-part composite key for O(1) global lookup across the entire registry.
 *     Combines DatabaseTag + CategoryTag + ItemTag into a single hashable struct.
 * ES: Clave compuesta de tres partes para lookup O(1) global en todo el registro.
 *     Combina DatabaseTag + CategoryTag + ItemTag en un struct hasheable.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXCompositeRegistryKey
{
	GENERATED_BODY()

	/** EN: Database identifier / ES: Identificador de base de datos */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FGameplayTag DatabaseTag;

	/** EN: Category identifier / ES: Identificador de categoria */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FGameplayTag CategoryTag;

	/** EN: Item identifier / ES: Identificador de item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FGameplayTag ItemTag;

	bool IsValid() const
	{
		return DatabaseTag.IsValid() && CategoryTag.IsValid() && ItemTag.IsValid();
	}

	bool operator==(const FPGXCompositeRegistryKey& Other) const
	{
		return DatabaseTag == Other.DatabaseTag
			&& CategoryTag == Other.CategoryTag
			&& ItemTag == Other.ItemTag;
	}

	bool operator!=(const FPGXCompositeRegistryKey& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FPGXCompositeRegistryKey& Key)
	{
		return HashCombine(
			GetTypeHash(Key.DatabaseTag),
			HashCombine(GetTypeHash(Key.CategoryTag), GetTypeHash(Key.ItemTag)));
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%s::%s::%s"),
			*DatabaseTag.ToString(), *CategoryTag.ToString(), *ItemTag.ToString());
	}
};

/**
 * EN: Reverse lookup key for all composite keys owned by one (Database, Item) pair.
 *     Used to keep legacy registration paths O(K) instead of scanning EntryIndex.
 * ES: Clave inversa para todas las claves compuestas de un par (Database, Item).
 *     Usada para mantener paths legacy O(K) sin escanear EntryIndex.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXRegistryReverseKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FGameplayTag DatabaseTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Registry")
	FGameplayTag ItemTag;

	bool IsValid() const
	{
		return DatabaseTag.IsValid() && ItemTag.IsValid();
	}

	bool operator==(const FPGXRegistryReverseKey& Other) const
	{
		return DatabaseTag == Other.DatabaseTag && ItemTag == Other.ItemTag;
	}

	friend uint32 GetTypeHash(const FPGXRegistryReverseKey& Key)
	{
		return HashCombine(GetTypeHash(Key.DatabaseTag), GetTypeHash(Key.ItemTag));
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%s::%s"), *DatabaseTag.ToString(), *ItemTag.ToString());
	}
};

/**
 * EN: Aggregates all entries within a single (Database, Category) pair.
 *     Provides fast category-scoped queries without scanning EntryIndex.
 * ES: Agrega todas las entradas dentro de un par (Database, Category).
 *     Proporciona queries rapidas por categoria sin escanear EntryIndex.
 */
USTRUCT()
struct PGXCORERUNTIME_API FPGXRegistryCategoryBucket
{
	GENERATED_BODY()

	/** EN: All item tags in this category / ES: Todos los tags de items en esta categoria */
	TArray<FGameplayTag> ItemTags;

	/** EN: Source DataTable(s) that contributed entries / ES: DataTable(s) fuente que contribuyeron entradas */
	TArray<TSoftObjectPtr<UDataTable>> SourceTables;

	int32 Num() const { return ItemTags.Num(); }
};
