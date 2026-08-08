// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Path/PGXPathTypes.h"

/**
 * EN: Centralized path resolution utility.
 *     All systems that write to disk use this instead of computing paths themselves.
 *     Provides caching, collision detection, and validation.
 *     Thread-safe via FCriticalSection.
 *
 * ES: Utilidad centralizada de resolucion de paths.
 *     Todos los sistemas que escriben a disco usan esto en vez de computar paths ellos mismos.
 *     Proporciona caching, deteccion de colisiones y validacion.
 *     Thread-safe via FCriticalSection.
 */
class PGXCORERUNTIME_API FPGXPathResolver
{
public:
	// ========================================================================
	// EN: Core API / ES: API Principal
	// ========================================================================

	/**
	 * EN: Resolve a full filesystem path from a PathContext.
	 *     Does NOT cache — use ResolveAndCache for caching behavior.
	 * ES: Resolver un path completo del filesystem desde un PathContext.
	 *     NO cachea — usar ResolveAndCache para comportamiento con cache.
	 */
	static FString ResolvePath(const FPGXPathContext& Context);

	/**
	 * EN: Resolve and store in the path cache. If already cached, returns cached value.
	 *     This is the primary API for all systems.
	 * ES: Resolver y almacenar en el cache de paths. Si ya esta cacheado, retorna valor cacheado.
	 *     Esta es la API principal para todos los sistemas.
	 */
	static FString ResolveAndCache(const FPGXPathContext& Context);

	/**
	 * EN: Get a previously cached path. Returns nullptr if not cached.
	 * ES: Obtener un path previamente cacheado. Retorna nullptr si no esta cacheado.
	 */
	static const FString* GetCachedPath(FGameplayTag SystemTag, FGameplayTag DomainTag,
		const FString& SlotIdentifier = FString());

	/**
	 * EN: Invalidate all cached paths for a system (e.g., on config reload).
	 * ES: Invalidar todos los paths cacheados para un sistema (e.g., al recargar config).
	 */
	static void InvalidateCache(FGameplayTag SystemTag);

	/** EN: Clear entire path cache / ES: Limpiar todo el cache de paths */
	static void ClearCache();

	// ========================================================================
	// EN: Utilities / ES: Utilidades
	// ========================================================================

	/**
	 * EN: Validate that a path is writable (directory exists or can be created).
	 * ES: Validar que un path es escribible (directorio existe o se puede crear).
	 */
	static bool ValidatePath(const FString& Path);

	/**
	 * EN: Ensure directory exists, creating intermediate dirs as needed.
	 * ES: Asegurar que el directorio existe, creando dirs intermedios si es necesario.
	 */
	static bool EnsureDirectoryExists(const FString& DirectoryPath);

	/**
	 * EN: Get the base save directory (platform-aware, maps to FPaths::ProjectSavedDir()).
	 * ES: Obtener el directorio base de guardado (consciente de plataforma, mapea a FPaths::ProjectSavedDir()).
	 */
	static FString GetPlatformBasePath();

	/**
	 * EN: Get the number of cached entries for diagnostics.
	 * ES: Obtener el numero de entradas en cache para diagnosticos.
	 */
	static int32 GetCacheSize();

private:
	// ========================================================================
	// EN: Strategy Implementations / ES: Implementaciones de Estrategia
	// ========================================================================

	static FString FormatUniquePath(const FPGXPathContext& Context);
	static FString FormatSessionPath(const FPGXPathContext& Context);
	static FString FormatProfilePath(const FPGXPathContext& Context);
	static FString FormatNamedSlotPath(const FPGXPathContext& Context);

	/**
	 * EN: Handle path collision for Profile strategy (auto-increment).
	 * ES: Manejar colision de path para estrategia Profile (auto-incremento).
	 */
	static FString HandleCollision(const FString& BasePath, const FPGXPathContext& Context);

	/**
	 * EN: Count existing files matching a prefix pattern.
	 * ES: Contar archivos existentes que coincidan con un patron de prefijo.
	 */
	static int32 CountExistingFiles(const FString& Directory, const FString& Prefix,
		const FString& Extension);

	// ========================================================================
	// EN: Cache Storage / ES: Almacenamiento de Cache
	// ========================================================================

	static TMap<FPGXPathCacheKey, FString> PathCache;
	static FCriticalSection CacheLock;
};
