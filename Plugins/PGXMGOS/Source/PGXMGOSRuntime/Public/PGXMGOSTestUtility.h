// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXMGOSTypes.h"
#include "PGXMGOSTestUtility.generated.h"

/**
 * EN: Test utility for the MGOS (GC Observability System).
 *     Provides 7 test functions covering initialization, mode switching,
 *     snapshot capture, baseline management, history, stress, and leak detection.
 *     Uses forced GC (CollectGarbage) and deterministic observation window for deterministic testing.
 *
 *     **Shipping gate (UHT-safe form)**:
 *     UHT forbids placing UCLASS/UFUNCTION declarations inside arbitrary
 *     preprocessor blocks. The reflected class is therefore declared in all
 *     configurations, while the .cpp provides inert shipping stubs for every
 *     function and keeps the force-GC/test implementation in non-shipping only.
 *     This preserves UHT legality and prevents the dangerous runtime behavior:
 *     - `RunStressTest` performs 50 rapid forced GC cycles — significant frame-time
 *       spikes that have no place in shipping builds.
 *     - `SimulateLeakDetection` spawns persistent actors via authored test pipeline
 *       — leaks state across world if invoked accidentally in production.
 *     - `ForceGCCycle` invokes `CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS, true)`
 *       — full-blocking GC, never appropriate for a shipping-build BP-callable.
 *     The runtime gate covers Shipping; class remains functional in Editor +
 *     Development + DebugGame for manual + automation testing.
 *
 * ES: Utilidad de test para el MGOS (Sistema de Observabilidad GC).
 *     Proporciona 7 funciones de test cubriendo inicializacion, cambio de modo,
 *     captura de snapshot, gestion de baseline, historial, stress y deteccion de leaks.
 *     Usa GC forzado (CollectGarbage) y ventana de observacion determinista para testing deterministico.
 *
 *     **Gate de shipping**: la declaracion reflejada
 *     queda visible por restricciones UHT, pero en Shipping todas las funciones
 *     devuelven stubs inertes. Las funciones force-GC + spawn-actor +
 *     50-ciclos-GC no ejecutan comportamiento peligroso en builds de produccion.
 *     Implementacion real disponible en Editor + Development + DebugGame.
 */
UCLASS()
class PGXMGOSRUNTIME_API UPGXMGOSTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * EN: Quick test — validates subsystem exists, is initialized, mode valid, config discovered, baseline state valid.
	 * ES: Test rapido — valida que el subsistema existe, esta inicializado, modo valido, config descubierto, estado baseline valido.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Test", meta = (DisplayName = "MGOS: Run Quick Test"))
	static bool RunQuickTest();

	/**
	 * EN: Test mode switching — cycles Off→Passive→Snapshot→DeepTrack→Off, verifies consistency.
	 * ES: Test de cambio de modo — cicla Off→Passive→Snapshot→DeepTrack→Off, verifica consistencia.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Test", meta = (DisplayName = "MGOS: Test Mode Switch"))
	static bool TestModeSwitch();

	/**
	 * EN: Test snapshot capture — forces GC, verifies snapshot has valid data including ProcessMemoryMB.
	 * ES: Test de captura de snapshot — fuerza GC, verifica que el snapshot tiene datos validos incluyendo ProcessMemoryMB.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Test", meta = (DisplayName = "MGOS: Test Snapshot Capture"))
	static bool TestSnapshotCapture();

	/**
	 * EN: Test baseline management — capture baseline, verify Valid, reset, verify Uninitialized. Uses the deterministic observation window.
	 * ES: Test de gestion de baseline — capturar baseline, verificar Valid, resetear, verificar Uninitialized. Usa la ventana de observacion determinista.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Test", meta = (DisplayName = "MGOS: Test Baseline Management"))
	static bool TestBaselineManagement();

	/**
	 * EN: Test history store — forces multiple GC cycles, verifies ring buffer and aggregate computation.
	 * ES: Test del store de historial — fuerza multiples ciclos GC, verifica buffer circular y calculo de agregados.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Test", meta = (DisplayName = "MGOS: Test History Store"))
	static bool TestHistoryStore();

	/**
	 * EN: Stress test — 50 rapid forced GC cycles, verifies no crashes and no MGOS memory growth.
	 * ES: Test de stress — 50 ciclos GC forzados rapidos, verifica sin crashes y sin crecimiento de memoria MGOS.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Test", meta = (DisplayName = "MGOS: Run Stress Test"))
	static bool RunStressTest();

	/**
	 * EN: Simulate leak detection — spawns persistent actors, forces GC with deterministic observation window,
	 *     runs enough cycles, compares final snapshot vs baseline, verifies profile state transitions.
	 * ES: Simular deteccion de leak — crea actores persistentes, fuerza GC con ventana de observacion determinista,
	 *     ejecuta suficientes ciclos, compara snapshot final vs baseline, verifica transiciones de estado de perfil.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Test", meta = (DisplayName = "MGOS: Simulate Leak Detection"))
	static bool SimulateLeakDetection();

	/**
	 * EN: Run all validation tests and return aggregate result.
	 *     Note: MGOS is a UEngineSubsystem — no WorldContext required.
	 * ES: Ejecutar todas las pruebas de validacion y retornar resultado agregado.
	 *     Nota: MGOS es un UEngineSubsystem — no requiere WorldContext.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Test", meta = (DisplayName = "MGOS: Run All Tests"))
	static bool RunAllTests(TArray<FString>& OutIssues);

private:
	/** EN: Helper to force GC and wait for post-GC processing / ES: Helper para forzar GC y esperar procesamiento post-GC */
	static void ForceGCCycle();

	/** EN: Log test result / ES: Registrar resultado de test */
	static void LogTestResult(const FString& TestName, bool bPassed);
};
