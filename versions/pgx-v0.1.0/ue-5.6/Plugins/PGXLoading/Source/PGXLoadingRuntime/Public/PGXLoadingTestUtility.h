// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXLoadingTestUtility.generated.h"

class UPGXLoadingSubsystem;

/**
 * EN: Test utility for the PGX Loading Screen system. Provides 7 test functions accessible
 *     from Blueprint or C++ to validate subsystem init, profiles, fade system,
 *     strategy switching, input blocking, stress, and full session simulation.
 *
 * ES: Utilidad de pruebas para el sistema PGX Loading Screen. Provee 7 funciones de prueba
 *     accesibles desde Blueprint o C++ para validar init del subsistema, perfiles, sistema de fade,
 *     cambio de estrategia, bloqueo de input, stress, y simulacion de sesion completa.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXLoadingTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Quick Test (auto-discovery validation)
	// ES: Test Rapido (validacion auto-descubrimiento)
	// ========================================================================

	/**
	 * EN: Validate subsystem init, config discovery, profile count, context resolution, state machine.
	 * ES: Validar init, descubrimiento de config, conteo de perfiles, resolucion de contexto, maquina de estados.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunQuickTest(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Targeted Tests
	// ES: Tests Dirigidos
	// ========================================================================

	/**
	 * EN: Resolve all registered context tags, validate profiles exist and have valid visual types.
	 * ES: Resolver todos los tags registrados, validar que los perfiles existen y tienen tipos visuales validos.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestProfileResolution(const UObject* WorldContextObject);

	/**
	 * EN: Request loading with short MinTime, verify fade in/out cycle completes.
	 * ES: Solicitar loading con MinTime corto, verificar que el ciclo fade in/out se completa.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestFadeSystem(const UObject* WorldContextObject);

	/**
	 * EN: Test RequestLoading/ForceClose cycle, verify state transitions are valid.
	 * ES: Test de ciclo RequestLoading/ForceClose, verificar que las transiciones de estado son validas.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestStrategySwitch(const UObject* WorldContextObject);

	/**
	 * EN: Verify input capture, block, and restore cycle during loading.
	 * ES: Verificar captura de input, bloqueo, y ciclo de restauracion durante carga.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestInputBlocking(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Stress / Performance
	// ES: Stress / Rendimiento
	// ========================================================================

	/**
	 * EN: Rapid-fire RequestLoading/ForceClose 50x, verify no leaks, no stuck state.
	 * ES: RequestLoading/ForceClose rapido 50x, verificar sin leaks, sin estado atascado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunStressTest(const UObject* WorldContextObject);

	// ========================================================================
	// EN: Simulation
	// ES: Simulacion
	// ========================================================================

	/**
	 * EN: Full lifecycle: init → request → fade → active → wait → close → history check.
	 * ES: Ciclo completo: init → request → fade → active → espera → cierre → check historial.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateGameSession(const UObject* WorldContextObject);

	/**
	 * EN: Run all validation tests and return aggregate result.
	 * ES: Ejecutar todas las pruebas de validacion y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Loading|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	static UPGXLoadingSubsystem* GetSubsystem(const UObject* WorldContextObject);
	static void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};
