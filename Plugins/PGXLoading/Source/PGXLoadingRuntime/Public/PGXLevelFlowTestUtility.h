// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXLevelFlowTestUtility.generated.h"

class UPGXLevelFlowSubsystem;

/**
 * EN: Test utility for the PGX LevelFlow system. Provides 7 test functions accessible
 *     from Blueprint or C++ to validate subsystem init, transition pipeline,
 *     tag resolution, cancel flow, sub-level tracking, and performance.
 *
 * ES: Utilidad de pruebas para el sistema PGX LevelFlow. Provee 7 funciones de prueba
 *     accesibles desde Blueprint o C++ para validar init del subsistema, pipeline de
 *     transicion, resolucion de tags, flujo de cancel, tracking de sub-niveles y rendimiento.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXLevelFlowTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Quick Test (auto-discovery validation)
	// ES: Test Rapido (validacion auto-descubrimiento)
	// ========================================================================

	/**
	 * EN: Validate subsystem init, check state is Idle, log discovered profiles/levels.
	 * ES: Validar init del subsistema, verificar estado es Idle, log de perfiles/niveles descubiertos.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunQuickTest(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Targeted Tests
	// ES: Tests Dirigidos
	// ========================================================================

	/**
	 * EN: Resolve all registered tags, verify each returns valid FPGXLevelEntry.
	 * ES: Resolver todos los tags registrados, verificar que cada uno retorna FPGXLevelEntry valido.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestResolveTags(const UObject* WorldContextObject);

	/**
	 * EN: Request transition to a level tag, verify state changes to non-Idle.
	 *     Does NOT wait for completion (transition may be blocked in editor).
	 * ES: Solicitar transicion a un tag de nivel, verificar que el estado cambia a no-Idle.
	 *     NO espera completar (transicion puede estar bloqueada en editor).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestTransitionRequest(const UObject* WorldContextObject, FGameplayTag LevelTag);

	/**
	 * EN: Request then immediately cancel, verify state returns to Idle.
	 * ES: Solicitar y luego cancelar inmediatamente, verificar que el estado vuelve a Idle.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestCancelFlow(const UObject* WorldContextObject, FGameplayTag LevelTag);

	/**
	 * EN: Test sub-level tracking: verify load/unload state changes and queries.
	 * ES: Test de tracking de sub-niveles: verificar cambios de estado y consultas de carga/descarga.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestSubLevelTracking(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Stress / Performance
	// ES: Stress / Rendimiento
	// ========================================================================

	/**
	 * EN: Attempt rapid-fire transitions, log timing and rejection behavior.
	 * ES: Intentar transiciones rapidas, log de timing y comportamiento de rechazo.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunStressTest(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Simulation
	// ES: Simulacion
	// ========================================================================

	/**
	 * EN: Full lifecycle: init check → resolve tags → query state → verify history → report.
	 * ES: Ciclo de vida completo: check init → resolver tags → consultar estado → verificar historial → reporte.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateGameSession(const UObject* WorldContextObject);

	/**
	 * EN: Run all validation tests and return aggregate result.
	 * ES: Ejecutar todas las pruebas de validacion y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|LevelFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	static UPGXLevelFlowSubsystem* GetSubsystem(const UObject* WorldContextObject);
	static void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};
