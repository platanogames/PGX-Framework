// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Profile/PGXProfileTypes.h"
#include "PGXProfileTestUtility.generated.h"

class UPGXProfileSubsystem;

/**
 * EN: Test utility for the PGX Profile system.
 *     Provides 7 callable test functions for verifying profile resolution,
 *     override application, capability/budget/feature queries, and Shipping lock.
 *     All tests are self-contained and log results via LogPGX.
 *
 * ES: Utilidad de prueba para el sistema PGX Profile.
 *     Provee 7 funciones de prueba invocables para verificar resolucion de profile,
 *     aplicacion de overrides, consultas de capability/budget/feature, y lock de Shipping.
 *     Todas las pruebas son autocontenidas y registran resultados via LogPGX.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXProfileTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * EN: Quick test — verify init, layers populated, query functional.
	 *     Entry point for rapid validation.
	 * ES: Test rapido — verificar init, capas pobladas, consulta funcional.
	 *     Punto de entrada para validacion rapida.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunQuickTest(const UObject* WorldContextObject);

	/**
	 * EN: Test resolution rules — verify AND/MIN/MostRestrictive behavior.
	 * ES: Test de reglas de resolucion — verificar comportamiento AND/MIN/MasRestrictivo.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestResolutionRules(const UObject* WorldContextObject);

	/**
	 * EN: Test platform override application.
	 * ES: Test de aplicacion de overrides de plataforma.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestPlatformOverrides(const UObject* WorldContextObject);

	/**
	 * EN: Test build context overrides (Development/Test/Shipping).
	 * ES: Test de overrides de contexto de build (Development/Test/Shipping).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestBuildOverrides(const UObject* WorldContextObject);

	/**
	 * EN: Test named capability queries.
	 * ES: Test de consultas de capabilities nombradas.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestCapabilityQueries(const UObject* WorldContextObject);

	/**
	 * EN: Test multi-platform resolution — verify intersection behavior.
	 * ES: Test de resolucion multi-plataforma — verificar comportamiento de interseccion.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestMultiPlatformResolution(const UObject* WorldContextObject);

	/**
	 * EN: Test Shipping lock — verify State::Locked prevents elevation.
	 * ES: Test de lock Shipping — verificar que State::Locked previene elevacion.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|Test", meta = (WorldContext = "WorldContextObject"))
	static void TestShippingLock(const UObject* WorldContextObject);

	/**
	 * EN: Run all validation tests and return aggregate result.
	 * ES: Ejecutar todas las pruebas de validacion y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	static UPGXProfileSubsystem* GetSubsystem(const UObject* WorldContextObject);
	static void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
};
