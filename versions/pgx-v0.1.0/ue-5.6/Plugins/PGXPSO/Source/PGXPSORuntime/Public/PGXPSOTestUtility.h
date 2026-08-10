// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXPSOTestUtility.generated.h"

class UPGXPSOSubsystem;

/**
 * EN: Test utility for the PGX PSO system. Provides 7 test functions accessible
 *     from Blueprint or C++ to validate subsystem init, warm-up pipeline,
 *     context filtering, control flow, and performance characteristics.
 *
 *     All test functions follow the canonical contract:
 *       static bool Test(WCO, TArray<FString>& OutIssues, ...)
 *     Returns true on full pass, false on any failure. OutIssues is populated
 *     with [PASS]/[FAIL]/[INFO] lines per assertion so callers (Automation
 *     wrappers, Test Dashboard) can surface granular results. Automation wrappers propagate the returned failure state and surface
 *     each issue instead of relying only on log side effects.
 *
 * ES: Utilidad de pruebas para el sistema PGX PSO. Provee 7 funciones de prueba
 *     accesibles desde Blueprint o C++ para validar init del subsistema, pipeline
 *     de warm-up, filtrado de contexto, flujo de control y caracteristicas de rendimiento.
 *
 *     Todas las funciones de test siguen el contrato canonico:
 *       static bool Test(WCO, TArray<FString>& OutIssues, ...)
 *     Retorna true en pase total, false en cualquier fallo.
 */
UCLASS()
class PGXPSORUNTIME_API UPGXPSOTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Quick Test (auto-discovery)
	// ES: Test Rapido (auto-descubrimiento)
	// ========================================================================

	/**
	 * EN: Auto-discover configs, validate subsystem init, check state/progress.
	 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines.
	 * ES: Auto-descubrir configs, validar init del subsistema, verificar estado/progreso.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunQuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// ========================================================================
	// EN: Targeted Tests
	// ES: Tests Dirigidos
	// ========================================================================

	/**
	 * EN: Request warm-up for context, verify state transitions.
	 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines.
	 * ES: Solicitar warm-up para contexto, verificar transiciones de estado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestSingleEntryWarmUp(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag);

	/**
	 * EN: Request warm-up, monitor batch progress.
	 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines.
	 * ES: Solicitar warm-up, monitorear progreso de lotes.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestBatchWarmUp(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag);

	/**
	 * EN: Add/Remove contexts, verify GetActiveContexts returns expected values.
	 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines.
	 * ES: Agregar/Remover contextos, verificar que GetActiveContexts retorna valores esperados.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestContextFiltering(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag);

	/**
	 * EN: Request->Pause->Resume->Cancel flow, verify state at each step.
	 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines.
	 * ES: Flujo Request->Pause->Resume->Cancel, verificar estado en cada paso.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestWarmUpControl(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag);

	// ========================================================================
	// EN: Stress / Performance
	// ES: Stress / Rendimiento
	// ========================================================================

	/**
	 * EN: Large batch warm-up, measure timing. Uses all discovered entries.
	 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines.
	 * ES: Warm-up de lote grande, medir tiempos. Usa todas las entradas descubiertas.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunStressTest(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag, int32 EntryCount = 100);

	// ========================================================================
	// EN: Simulation
	// ES: Simulacion
	// ========================================================================

	/**
	 * EN: Full lifecycle: init -> warm-up -> progress -> save cache -> report.
	 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines.
	 * ES: Ciclo de vida completo: init -> warm-up -> progreso -> guardar cache -> reporte.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Test", meta = (WorldContext = "WorldContextObject"))
	static bool SimulateGameSession(const UObject* WorldContextObject, TArray<FString>& OutIssues, FGameplayTag ContextTag);

	/**
	 * EN: Run all validation tests and return aggregate result.
	 *     Returns true on full pass; populates OutIssues with [PASS]/[FAIL] lines.
	 * ES: Ejecutar todas las pruebas de validacion y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|PSO|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	/** EN: Get PSO subsystem from world context / ES: Obtener subsistema PSO desde world context */
	static UPGXPSOSubsystem* GetSubsystem(const UObject* WorldContextObject);

	/** EN: Append a [PASS]/[FAIL] line to OutIssues. Also logs via UE_LOG for live visibility. */
	static void RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};
