// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXGameFlowTypes.h"
#include "PGXGameFlowTestUtility.generated.h"

class UPGXGameFlowSubsystem;

/**
 * EN: Test utility for the PGX GameFlow system.
 *     Provides 7 callable test functions for verifying subsystem behavior in PIE.
 *     Logs results via LogPGXGameFlow. All tests are self-contained.
 *
 * ES: Utilidad de prueba para el sistema PGX GameFlow.
 *     Provee 7 funciones de prueba invocables para verificar comportamiento del subsistema en PIE.
 *     Registra resultados via LogPGXGameFlow. Todas las pruebas son autocontenidas.
 */
UCLASS()
class PGXGAMEFLOWRUNTIME_API UPGXGameFlowTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * EN: Quick test — auto-discover config, set states across channels, verify.
	 *     Entry point for rapid validation. Calls into other test functions.
	 * ES: Test rapido — auto-descubrir config, establecer estados en canales, verificar.
	 *     Punto de entrada para validacion rapida. Llama a otras funciones de prueba.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunQuickTest(const UObject* WorldContextObject);

	/**
	 * EN: Test set and validate roundtrip per channel.
	 * ES: Test de ida y vuelta set/validate por canal.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestSetAndValidate(const UObject* WorldContextObject);

	/**
	 * EN: Test batch operations (sequential and parallel).
	 * ES: Test de operaciones batch (secuencial y paralelo).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestBatchOperations(const UObject* WorldContextObject);

	/**
	 * EN: Test revert per channel.
	 * ES: Test de revert por canal.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestRevertFlow(const UObject* WorldContextObject);

	/**
	 * EN: Test validation rules enforcement (allowed/disallowed).
	 * ES: Test de aplicacion de reglas de validacion (allowed/disallowed).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestValidationRules(const UObject* WorldContextObject);

	/**
	 * EN: Stress test — rapid transitions, measure timing.
	 * ES: Test de estres — transiciones rapidas, medir tiempos.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunStressTest(const UObject* WorldContextObject, int32 Iterations = 1000);

	/**
	 * EN: Simulate realistic game session — multi-channel progression.
	 * ES: Simular sesion de juego realista — progresion multi-canal.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateGameSession(const UObject* WorldContextObject);

	/**
	 * EN: Run all validation tests and return aggregate result.
	 * ES: Ejecutar todas las pruebas de validacion y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|GameFlow|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	static UPGXGameFlowSubsystem* GetSubsystem(const UObject* WorldContextObject);
	static void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
	static FGameplayTag MakeTag(const FString& TagString);
};
