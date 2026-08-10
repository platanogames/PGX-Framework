// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXConstructionTestUtility.generated.h"

class UPGXWorldConstructionSubsystem;
class UPGXConstructionSettings;

/**
 * EN: Test utility for the PGX Construction system (v1.1).
 *     Provides Blueprint-callable test/debug helpers for validating
 *     construction settings, class sources, resolver priority chain,
 *     world overrides, DA validation, and full resolution reports.
 *     Designed for PIE testing and automation.
 *
 * ES: Utilidad de test para el sistema PGX Construction (v1.1).
 *     Provee helpers de test/debug llamables desde Blueprint para validar
 *     settings de construccion, class sources, cadena de prioridad del
 *     resolver, overrides de mundo, validacion de DAs, y reportes de resolucion completos.
 *     Disenado para testing PIE y automatizacion.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXConstructionTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// =================================================================
	// EN: Test 1 — Quick Test (settings + subsystem + class sources + resolve)
	// ES: Test 1 — Quick Test (settings + subsistema + class sources + resolucion)
	// =================================================================

	/**
	 * EN: Validate that settings are accessible, WorldSubsystem exists,
	 *     class sources are valid, and resolve each of the 7 types.
	 * ES: Validar que settings son accesibles, WorldSubsystem existe,
	 *     class sources son validos, y resolver cada uno de los 7 tipos.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunQuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 2 — Resolver Priority (WorldOverride vs ProjectSettings)
	// ES: Test 2 — Prioridad del Resolver (WorldOverride vs ProjectSettings)
	// =================================================================

	/**
	 * EN: For each of the 7 types, log whether the resolved DA comes from
	 *     WorldOverride, ProjectSettings, or is nullptr.
	 * ES: Para cada uno de los 7 tipos, loguear si el DA resuelto viene de
	 *     WorldOverride, ProjectSettings, o es nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestResolverPriority(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 3 — Class Source Validation (source vs class/DA consistency)
	// ES: Test 3 — Validacion de Class Source (consistencia source vs clase/DA)
	// =================================================================

	/**
	 * EN: Call ValidateClassSources() and log each warning.
	 *     Verify consistency between ClassSource and class/DA assignment per slot.
	 * ES: Llamar ValidateClassSources() y loguear cada warning.
	 *     Verificar consistencia entre ClassSource y asignacion de clase/DA por slot.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestWorkflowValidation(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 4 — Settings Integrity (7 slots validation)
	// ES: Test 4 — Integridad de Settings (validacion de 7 slots)
	// =================================================================

	/**
	 * EN: Read all 7 slots from UPGXConstructionSettings and verify that
	 *     each ClassSource has a class/DA assigned if Source != Default.
	 * ES: Leer los 7 slots de UPGXConstructionSettings y verificar que
	 *     cada ClassSource tiene clase/DA asignado si Source != Default.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestSettingsIntegrity(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 5 — World Overrides (per-level override status)
	// ES: Test 5 — Overrides de Mundo (estado de overrides por nivel)
	// =================================================================

	/**
	 * EN: Check GetOverride<T>() for all 7 types in the WorldSubsystem.
	 *     Log which have active overrides and which are nullptr.
	 * ES: Verificar GetOverride<T>() para los 7 tipos en el WorldSubsystem.
	 *     Loguear cuales tienen overrides activos y cuales son nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestWorldOverrides(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 6 — DA Discovery and Validation (AssetRegistry scan)
	// ES: Test 6 — Descubrimiento y Validacion de DAs (scan AssetRegistry)
	// =================================================================

	/**
	 * EN: Scan AssetRegistry for all UPGXClassConstruction subclass DAs,
	 *     load each one, call Validate(), and log any issues.
	 * ES: Escanear AssetRegistry por todos los DAs subclase de UPGXClassConstruction,
	 *     cargar cada uno, llamar Validate(), y loguear cualquier issue.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestDADiscoveryAndValidation(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Test 7 — Full Resolution Report (diagnostic dashboard)
	// ES: Test 7 — Reporte Completo de Resolucion (dashboard diagnostico)
	// =================================================================

	/**
	 * EN: For each type (GM, PC, GS, PS, Char, Pawn, HUD): which DA is assigned,
	 *     where it comes from (Override/Settings/None), how many components it would
	 *     inject, and which tags it would grant. A full diagnostic dashboard.
	 * ES: Para cada tipo (GM, PC, GS, PS, Char, Pawn, HUD): que DA esta asignado,
	 *     de donde viene (Override/Settings/None), cuantos componentes inyectaria,
	 *     y que tags otorgaria. Un dashboard diagnostico completo.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestFullResolutionReport(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// =================================================================
	// EN: Run All Tests — aggregate suite
	// ES: Ejecutar Todos — suite agregada
	// =================================================================

	/**
	 * EN: Run all 7 construction tests and return aggregate result.
	 * ES: Ejecutar los 7 tests de construccion y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Construction|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);
};
