// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXSaveTestUtility.generated.h"

class UPGXSaveSubsystem;

/**
 * EN: Test utility for validating the PGX Save system.
 *     Provides targeted tests (key-value roundtrip, slot CRUD, versioning, quick save)
 *     and performance tests (stress test with N keys). All functions are BlueprintCallable.
 *
 *     Use cases:
 *     - Validate save/load data integrity after code changes
 *     - Visual validation of the Save Inspector (contexts, slots, pipeline log)
 *     - Performance profiling of serialization, compression, and disk I/O
 *     - Verify version migration pipeline
 *     - Demo/showcase of the save system capabilities
 *
 *     All tests use "_PGXTest_" prefix for slot names to avoid colliding
 *     with real user data. Tests clean up test slots after verification.
 *
 * ES: Utilidad de test para validar el sistema PGX Save.
 *     Provee tests dirigidos (roundtrip key-value, CRUD de slots, versionado, quick save)
 *     y tests de rendimiento (stress test con N keys). Todas las funciones son BlueprintCallable.
 *
 *     Casos de uso:
 *     - Validar integridad de datos save/load despues de cambios de codigo
 *     - Validacion visual del Save Inspector (contextos, slots, pipeline log)
 *     - Profiling de rendimiento de serializacion, compresion, y I/O de disco
 *     - Verificar pipeline de migracion de version
 *     - Demo/showcase de las capacidades del sistema de guardado
 *
 *     Todos los tests usan prefijo "_PGXTest_" para nombres de slot para evitar
 *     colisiones con datos reales del usuario. Los tests limpian slots de test despues de verificar.
 */
UCLASS()
class PGXSAVERUNTIME_API UPGXSaveTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Quick Test (auto-discovers first context)
	// ES: Test Rapido (auto-descubre primer contexto)
	// ========================================================================

	/**
	 * EN: Full save/load cycle with verification. Auto-discovers the first available
	 *     context and domain, writes test data (all 8 types), saves, clears memory,
	 *     loads, and verifies every value. Logs PASS/FAIL per type.
	 * ES: Ciclo completo save/load con verificacion. Auto-descubre el primer contexto
	 *     y dominio disponible, escribe datos de test (los 8 tipos), guarda, limpia memoria,
	 *     carga, y verifica cada valor. Loguea PASS/FAIL por tipo.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Test",
		meta = (WorldContext = "WorldContextObject"))
	static void RunQuickTest(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Targeted Tests (require context tag)
	// ES: Tests Dirigidos (requieren context tag)
	// ========================================================================

	/**
	 * EN: Write all 8 typed key-value pairs, save, clear, load, verify each type matches.
	 *     Tests: String, Int, Float, Bool, Vector, Rotator, Transform, GameplayTag.
	 * ES: Escribir los 8 pares key-value tipados, guardar, limpiar, cargar, verificar cada tipo.
	 *     Tests: String, Int, Float, Bool, Vector, Rotator, Transform, GameplayTag.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Test",
		meta = (WorldContext = "WorldContextObject"))
	static void TestKeyValueRoundTrip(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/**
	 * EN: Test slot CRUD operations: create slots via save, copy slot, verify copy,
	 *     enumerate all slots, delete slot, verify deletion.
	 * ES: Test operaciones CRUD de slots: crear slots via save, copiar slot, verificar copia,
	 *     enumerar todos los slots, eliminar slot, verificar eliminacion.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Test",
		meta = (WorldContext = "WorldContextObject"))
	static void TestSlotOperations(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/**
	 * EN: QuickSave/QuickLoad roundtrip: write data, quick save, clear, quick load, verify.
	 * ES: Roundtrip QuickSave/QuickLoad: escribir datos, quick save, limpiar, quick load, verificar.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Test",
		meta = (WorldContext = "WorldContextObject"))
	static void TestQuickSaveLoad(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/**
	 * EN: Test version migration pipeline. Registers a temporary migration step,
	 *     manipulates SaveGame version, saves, loads (triggering migration), verifies.
	 *     Cleans up the migration registration after the test.
	 * ES: Test del pipeline de migracion de version. Registra un paso de migracion temporal,
	 *     manipula la version del SaveGame, guarda, carga (disparando migracion), verifica.
	 *     Limpia el registro de migracion despues del test.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Test",
		meta = (WorldContext = "WorldContextObject"))
	static void TestVersionMigration(const UObject* WorldContextObject, FGameplayTag ContextTag);

	// ========================================================================
	// EN: Stress / Performance
	// ES: Stress / Rendimiento
	// ========================================================================

	/**
	 * EN: Write KeyCount keys (int type), save, load, verify all. Measures and logs:
	 *     write time, save time, load time, verify time, total throughput.
	 * ES: Escribir KeyCount keys (tipo int), guardar, cargar, verificar todos. Mide y loguea:
	 *     tiempo de escritura, guardado, carga, verificacion, throughput total.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Test",
		meta = (WorldContext = "WorldContextObject"))
	static void RunStressTest(const UObject* WorldContextObject, FGameplayTag ContextTag, int32 KeyCount = 500);

	// ========================================================================
	// EN: Simulation
	// ES: Simulacion
	// ========================================================================

	/**
	 * EN: Simulate a realistic game session: populate diverse data across the first domain,
	 *     perform manual save, auto-save trigger, quick save, load from slot, and log
	 *     all operations. Useful for populating the Save Inspector with realistic data.
	 * ES: Simular una sesion de juego realista: poblar datos diversos en el primer dominio,
	 *     realizar save manual, trigger de auto-save, quick save, load desde slot, y loguear
	 *     todas las operaciones. Util para poblar el Save Inspector con datos realistas.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Test",
		meta = (WorldContext = "WorldContextObject"))
	static void SimulateGameSession(const UObject* WorldContextObject, FGameplayTag ContextTag);

	/**
	 * EN: Run all validation tests and return aggregate result.
	 * ES: Ejecutar todas las pruebas de validacion y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Save|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	// EN: Resolve subsystem from world context / ES: Resolver subsistema desde world context
	static UPGXSaveSubsystem* GetSubsystem(const UObject* WorldContextObject);

	// EN: Log test result with consistent formatting / ES: Loguear resultado de test con formato consistente
	static void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));

	// EN: Get the first domain tag from a context's config / ES: Obtener el primer domain tag de la config de un contexto
	static FGameplayTag GetFirstDomainTag(UPGXSaveSubsystem* Sub, FGameplayTag ContextTag);
};
