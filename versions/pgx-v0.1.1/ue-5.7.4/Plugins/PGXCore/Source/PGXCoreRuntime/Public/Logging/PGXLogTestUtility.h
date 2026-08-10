// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/PGXLogTypes.h"
#include "PGXLogTestUtility.generated.h"

// ═══════════════════════════════════════════════════════════════
// EN: Test utility for generating realistic PGX log entries across systems.
//     Provides per-system simulation, bulk stress testing, and a quick test
//     to validate the Log Viewer visually. All functions are BlueprintCallable.
//
//     Use cases:
//     - Visual validation of Log Viewer (search, filters, colors, expansion)
//     - Profiling ring buffer, broadcast, and UI performance under load
//     - Demo/showcase of the structured logging system capabilities
//
// ES: Utilidad de test para generar entradas de log PGX realistas entre sistemas.
//     Provee simulacion por sistema, stress test masivo, y un test rapido
//     para validar el Log Viewer visualmente. Todas las funciones son BlueprintCallable.
//
//     Casos de uso:
//     - Validacion visual del Log Viewer (busqueda, filtros, colores, expansion)
//     - Profiling del ring buffer, broadcast, y rendimiento de UI bajo carga
//     - Demo/showcase de las capacidades del sistema de logging estructurado
// ═══════════════════════════════════════════════════════════════

UCLASS()
class PGXCORERUNTIME_API UPGXLogTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ═══════════════════════════════════════
	// Quick Tests / Tests Rapidos
	// ═══════════════════════════════════════

	/**
	 * EN: Generate ~35 varied log entries across all systems. Perfect for a quick
	 *     visual test of the Log Viewer: colors, search, filters, params, tags.
	 * ES: Genera ~35 entradas de log variadas entre todos los sistemas. Perfecto
	 *     para un test visual rapido del Log Viewer: colores, busqueda, filtros, params, tags.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunQuickTest(const UObject* WorldContextObject);

	// ═══════════════════════════════════════
	// Per-System Simulation / Simulacion por Sistema
	// ═══════════════════════════════════════

	/** EN: Simulate Save system logs / ES: Simular logs del sistema de Save */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateSaveSystem(const UObject* WorldContextObject, int32 Count = 10);

	/** EN: Simulate GameFlow logs / ES: Simular logs del sistema GameFlow */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateGameFlow(const UObject* WorldContextObject, int32 Count = 10);

	/** EN: Simulate Audio system logs / ES: Simular logs del sistema de Audio */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateAudioSystem(const UObject* WorldContextObject, int32 Count = 10);

	/** EN: Simulate AI system logs / ES: Simular logs del sistema de AI */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateAISystem(const UObject* WorldContextObject, int32 Count = 10);

	/** EN: Simulate Network/Online logs / ES: Simular logs de Network/Online */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateNetworkSystem(const UObject* WorldContextObject, int32 Count = 10);

	/** EN: Simulate UI system logs / ES: Simular logs del sistema UI */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateUISystem(const UObject* WorldContextObject, int32 Count = 10);

	/** EN: Simulate Inventory system logs / ES: Simular logs del sistema de Inventario */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateInventorySystem(const UObject* WorldContextObject, int32 Count = 10);

	// ═══════════════════════════════════════
	// Domain-Aware Tests (v3.0)
	// Tests con Dominio (v3.0)
	// ═══════════════════════════════════════

	/**
	 * EN: Generate log entries WITH DomainTag set for all 13 built-in domains.
	 *     Each entry includes domain-specific Params (the keys that domain renderers extract).
	 *     Use to validate polymorphic rendering in the v3.0 Log Viewer.
	 * ES: Generar entradas de log CON DomainTag para los 13 dominios built-in.
	 *     Cada entrada incluye Params especificos del dominio (las keys que los renderers extraen).
	 *     Usar para validar rendering polimorfico en el Log Viewer v3.0.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateDomainLogs(const UObject* WorldContextObject, int32 CountPerDomain = 3);

	/**
	 * EN: Quick validation of the domain system: register check + one entry per domain + renderer lookup.
	 *     Reports results via OutIssues. Returns true if all checks passed.
	 * ES: Validacion rapida del sistema de dominios: check de registro + una entrada por dominio + lookup de renderer.
	 *     Reporta resultados via OutIssues. Retorna true si todos los checks pasaron.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunDomainQuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// ═══════════════════════════════════════
	// Bulk / Masivo
	// ═══════════════════════════════════════

	/**
	 * EN: Run all system simulations. Good for filling the buffer with diverse, realistic data.
	 * ES: Ejecutar todas las simulaciones de sistemas. Ideal para llenar el buffer con datos diversos y realistas.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunAllSystems(const UObject* WorldContextObject, int32 CountPerSystem = 10);

	/**
	 * EN: Stress test — generate N entries as fast as possible with mixed systems and verbosities.
	 *     Use for profiling ring buffer throughput, delegate broadcast cost, and Log Viewer UI performance.
	 * ES: Stress test — generar N entradas lo mas rapido posible con sistemas y verbosities mixtos.
	 *     Usar para profiling de throughput del ring buffer, costo de broadcast de delegados, y rendimiento UI del Log Viewer.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void RunStressTest(const UObject* WorldContextObject, int32 Count = 1000);

	/**
	 * EN: Simulate a realistic 60-second game session: startup, gameplay events, save, shutdown.
	 *     Entries are submitted immediately (no real timer) but with realistic timestamp offsets.
	 * ES: Simular una sesion de juego realista de 60 segundos: startup, eventos gameplay, save, shutdown.
	 *     Las entradas se envian inmediatamente (sin timer real) pero con offsets de timestamp realistas.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static void SimulateGameSession(const UObject* WorldContextObject);

	/**
	 * EN: Run all validation tests and return aggregate result.
	 * ES: Ejecutar todas las pruebas de validacion y retornar resultado agregado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Log|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	/**
	 * EN: Internal: Submit an entry directly to subsystem (faster than going through BP library).
	 * ES: Interno: Enviar entrada directamente al subsistema (mas rapido que pasar por libreria BP).
	 */
	static void Submit(const UObject* WorldContextObject, FName Category, EPGXLogVerbosity Verbosity, const FString& Message, const TArray<TPair<FString, FString>>& Params = {});

	/**
	 * EN: Internal: Submit an entry with DomainTag directly set.
	 * ES: Interno: Enviar entrada con DomainTag directamente establecido.
	 */
	static void SubmitWithDomain(const UObject* WorldContextObject, FName Category, EPGXLogVerbosity Verbosity, const FGameplayTag& DomainTag, const FString& Message, const TArray<TPair<FString, FString>>& Params = {});
};
