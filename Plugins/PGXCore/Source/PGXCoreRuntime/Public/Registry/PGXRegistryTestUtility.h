// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXRegistryTestUtility.generated.h"

class UPGXDataRegistrySubsystem;

/**
 * EN: Test utility for the PGX Data Registry system (v2.0).
 *     Provides Blueprint-callable test/debug helpers for validating
 *     database CRUD, category queries, composite key lookups, conflict
 *     policies, cache management, and telemetry.
 *     Designed for PIE testing and automation.
 *
 * ES: Utilidad de test para el sistema PGX Data Registry (v2.0).
 *     Provee helpers de test/debug llamables desde Blueprint para validar
 *     CRUD de databases, queries por categoria, lookups por clave compuesta,
 *     politicas de conflicto, gestion de cache, y telemetria.
 *     Disenado para testing PIE y automatizacion.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXRegistryTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// =================================================================
	// EN: Test 1 — Quick Test (subsystem health + database stats)
	// ES: Test 1 — Quick Test (salud del subsistema + stats de databases)
	// =================================================================

	/**
	 * EN: Validate that the registry subsystem exists, is initialized, and report
	 *     all database tags with their stats and total entry index count.
	 * ES: Validar que el subsistema de registry existe, esta inicializado, y reportar
	 *     todos los database tags con sus stats y conteo total del entry index.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunQuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 2 — Database CRUD (create, register, find, unregister)
	// ES: Test 2 — CRUD de Database (crear, registrar, buscar, desregistrar)
	// =================================================================

	/**
	 * EN: Create a temporary database, register a transient asset, find it,
	 *     unregister it, verify removal, and clean up.
	 * ES: Crear una database temporal, registrar un asset transient, buscarlo,
	 *     desregistrarlo, verificar remocion, y limpiar.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestDatabaseCRUD(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 3 — Category Queries (register with categories, query)
	// ES: Test 3 — Queries por Categoria (registrar con categorias, consultar)
	// =================================================================

	/**
	 * EN: Create a database, register 3 assets with 2 distinct categories,
	 *     verify FindByCategory and GetCategoryEntries return correct counts.
	 * ES: Crear una database, registrar 3 assets con 2 categorias distintas,
	 *     verificar que FindByCategory y GetCategoryEntries retornan conteos correctos.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestCategoryQueries(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 4 — Composite Key Lookup (O(1) global index)
	// ES: Test 4 — Lookup por Clave Compuesta (indice global O(1))
	// =================================================================

	/**
	 * EN: Create a database, register assets, verify FindByCompositeKey returns
	 *     correct entry, and that an invalid key returns nullptr.
	 * ES: Crear una database, registrar assets, verificar que FindByCompositeKey
	 *     retorna la entrada correcta, y que una clave invalida retorna nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestCompositeKeyLookup(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 5 — Conflict Policies (duplicate registration behavior)
	// ES: Test 5 — Politicas de Conflicto (comportamiento de registro duplicado)
	// =================================================================

	/**
	 * EN: Create databases, register same ItemTag twice to verify that duplicate
	 *     registration is correctly rejected. Tests the FailOnConflict behavior.
	 * ES: Crear databases, registrar mismo ItemTag dos veces para verificar que
	 *     el registro duplicado se rechaza correctamente. Testea el comportamiento FailOnConflict.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestConflictPolicies(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 6 — Stress Test (bulk registration + query throughput)
	// ES: Test 6 — Stress Test (registro masivo + throughput de queries)
	// =================================================================

	/**
	 * EN: Create a database, register N assets, measure registration time,
	 *     FindEntry x N, FindByCategory, GetAllEntries, and InvalidateCache.
	 *     Logs throughput metrics.
	 * ES: Crear una database, registrar N assets, medir tiempo de registro,
	 *     FindEntry x N, FindByCategory, GetAllEntries, e InvalidateCache.
	 *     Loguea metricas de throughput.
	 * @param EntryCount Number of entries to register (default 500)
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunStressTest(const UObject* WorldContextObject, int32 EntryCount, TArray<FString>& OutIssues);

	/**
	 * EN: AAA-scale benchmark wrapper for 10k/50k/100k entries without touching commandlet/shared Testing modules.
	 * ES: Wrapper de benchmark escala AAA para 10k/50k/100k sin tocar commandlets/modulos Testing compartidos.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunLargeRegistryBenchmark(const UObject* WorldContextObject, int32 EntryCount, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 7 — Cache and Telemetry (stats, invalidation, metadata)
	// ES: Test 7 — Cache y Telemetria (stats, invalidacion, metadata)
	// =================================================================

	/**
	 * EN: Create a database, register assets, verify GetDatabaseStats (TotalEntries,
	 *     LoadedEntries), InvalidateCache, verify LoadedEntries == 0, ExportMetadata non-empty.
	 * ES: Crear una database, registrar assets, verificar GetDatabaseStats (TotalEntries,
	 *     LoadedEntries), InvalidateCache, verificar LoadedEntries == 0, ExportMetadata no vacio.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestCacheAndTelemetry(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Run All Tests — aggregate suite
	// ES: Ejecutar Todos — suite agregada
	// =================================================================

	/**
	 * EN: Run all 7 registry tests and return aggregate result.
	 * ES: Ejecutar los 7 tests de registry y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Registry|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);
};
