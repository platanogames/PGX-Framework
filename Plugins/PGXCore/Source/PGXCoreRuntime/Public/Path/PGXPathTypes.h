// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXPathTypes.generated.h"

// ============================================================================
// EN: Path Resolution Types for PGX Infrastructure v0.4.0
//     Shared by all systems that write to disk (Save, Log, PSO, etc.).
// ES: Tipos de Resolucion de Paths para Infraestructura PGX v0.4.0
//     Compartidos por todos los sistemas que escriben a disco (Save, Log, PSO, etc.).
// ============================================================================

/**
 * EN: How the path for a save/export type is auto-generated.
 *     Each Config DA selects a strategy for its system's disk output.
 * ES: Como se auto-genera el path para un tipo de guardado/exportacion.
 *     Cada Config DA selecciona una estrategia para la salida a disco de su sistema.
 */
UENUM(BlueprintType)
enum class EPGXPathStrategy : uint8
{
	/** EN: Single static path, overwritten each time / ES: Path estatico unico, sobreescrito cada vez */
	Unique,

	/** EN: Timestamped subfolder (Sessions/2026-02-14_143022/) / ES: Subcarpeta con timestamp */
	Session,

	/** EN: Auto-incremented ID (Profiles/Save_0, Save_1...) / ES: ID auto-incremental */
	Profile,

	/** EN: Named slots (Slot_01, Slot_02...) / ES: Slots con nombre */
	NamedSlot,

	/** EN: Custom pattern resolved by the system / ES: Patron personalizado resuelto por el sistema */
	Custom
};

/**
 * EN: Context for a path resolution request. Each system fills this
 *     with its own parameters, and the resolver generates the full path.
 * ES: Contexto para una solicitud de resolucion de path. Cada sistema
 *     lo llena con sus propios parametros, y el resolver genera el path completo.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXPathContext
{
	GENERATED_BODY()

	/** EN: Which system owns this path (PGX.System.Save, PGX.System.Log, etc.) / ES: Que sistema posee este path */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|Path")
	FGameplayTag SystemTag;

	/** EN: Domain or sub-context tag / ES: Tag de dominio o sub-contexto */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|Path")
	FGameplayTag DomainTag;

	/** EN: How to generate the path / ES: Como generar el path */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|Path")
	EPGXPathStrategy Strategy = EPGXPathStrategy::Unique;

	/** EN: Base directory (from Config DA, relative to platform save dir) / ES: Directorio base (del Config DA, relativo al dir de guardado de plataforma) */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|Path")
	FString BaseDirectory;

	/** EN: File extension (including dot) / ES: Extension de archivo (incluyendo punto) */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|Path")
	FString FileExtension = TEXT(".sav");

	/** EN: Prefix for auto-generated names / ES: Prefijo para nombres auto-generados */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|Path")
	FString Prefix;

	/** EN: Optional explicit slot name (overrides auto-generation) / ES: Nombre de slot explicito opcional (sobreescribe auto-generacion) */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|Path")
	FString ExplicitSlotName;

	/** EN: Subfolder name for Session strategy / ES: Nombre de subcarpeta para estrategia Session */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|Path")
	FString SessionFolderName = TEXT("Sessions");

	/** EN: Subfolder name for Profile strategy / ES: Nombre de subcarpeta para estrategia Profile */
	UPROPERTY(BlueprintReadWrite, Category = "PGX|Path")
	FString ProfileFolderName = TEXT("Profiles");
};

/**
 * EN: Composite key for the path cache. Two entries with the same key
 *     share the same resolved path.
 * ES: Clave compuesta para el cache de paths. Dos entradas con la misma
 *     clave comparten el mismo path resuelto.
 */
USTRUCT()
struct PGXCORERUNTIME_API FPGXPathCacheKey
{
	GENERATED_BODY()

	FGameplayTag SystemTag;
	FGameplayTag DomainTag;

	/** EN: Empty for Unique, timestamp for Session, ID for Profile / ES: Vacio para Unique, timestamp para Session, ID para Profile */
	FString SlotIdentifier;

	bool operator==(const FPGXPathCacheKey& Other) const
	{
		return SystemTag == Other.SystemTag
			&& DomainTag == Other.DomainTag
			&& SlotIdentifier == Other.SlotIdentifier;
	}

	friend uint32 GetTypeHash(const FPGXPathCacheKey& Key)
	{
		uint32 Hash = GetTypeHash(Key.SystemTag);
		Hash = HashCombine(Hash, GetTypeHash(Key.DomainTag));
		Hash = HashCombine(Hash, GetTypeHash(Key.SlotIdentifier));
		return Hash;
	}
};
