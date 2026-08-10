// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXEventHandlerTestUtility.generated.h"

/**
 * EN: Test utility for the PGX Event Handler System.
 *     Provides Blueprint-callable test/debug helpers for validating
 *     handler registration, execution, lifecycle, and observability.
 *     Designed for PIE testing and automation.
 * ES: Utilidad de test para el Sistema de Event Handler PGX.
 *     Provee helpers de test/debug llamables desde Blueprint para validar
 *     registro, ejecucion, ciclo de vida y observabilidad de handlers.
 *     Disenado para testing PIE y automatizacion.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXEventHandlerTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** EN: Quick system health check / ES: Verificacion rapida de salud del sistema */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool QuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test handler registration and unregistration / ES: Probar registro y desregistro de handlers */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool LifecycleTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test payload delivery to handlers / ES: Probar entrega de payload a handlers */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool PayloadTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test CanExecute condition checking / ES: Probar verificacion de condicion CanExecute */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool ConditionTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test ExecuteSequence with multiple handlers / ES: Probar ExecuteSequence con multiples handlers */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool SequenceTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Test cache hit/miss and eviction / ES: Probar cache hit/miss y eviccion */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool CacheTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: P0 contract coverage for validation reasons, redaction, disabled handlers. */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool P0ContractTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/** EN: Run all event handler tests / ES: Ejecutar todas las pruebas del event handler */
	UFUNCTION(BlueprintCallable, Category = "PGX|EventHandler|Test",
		meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);
};
