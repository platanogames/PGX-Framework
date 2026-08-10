// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

/**
 * EN: Base macros and utilities for PGX automation tests.
 *     All PGX tests should use these macros for consistent test registration,
 *     naming, and reporting across the framework.
 *
 * ES: Macros base y utilidades para tests de automatizacion PGX.
 *     Todos los tests PGX deben usar estos macros para registro consistente,
 *     naming, y reportes a traves del framework.
 */

// EN: UE 5.5+ changed EAutomationTestFlags to enum class.
//     ApplicationContextMask is now EAutomationTestFlags_ApplicationContextMask (inline constexpr).
// ES: UE 5.5+ cambio EAutomationTestFlags a enum class.
//     ApplicationContextMask es ahora EAutomationTestFlags_ApplicationContextMask (inline constexpr).

// EN: Macro for simple PGX unit tests (runs in any application context).
//     Usage: PGX_TEST(FMyTest) { TestTrue(...); return true; }
// ES: Macro para tests unitarios PGX simples (corren en cualquier contexto).
//     Uso: PGX_TEST(FMyTest) { TestTrue(...); return true; }
#define PGX_TEST(TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "PGX." #TestName, \
		EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter) \
	bool TestName::RunTest(const FString& /*Parameters*/)

// EN: Macro for PGX tests that need a game world
// ES: Macro para tests PGX que necesitan un mundo de juego
#define PGX_TEST_GAME(TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "PGX.Game." #TestName, \
		EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter) \
	bool TestName::RunTest(const FString& /*Parameters*/)

// EN: Macro for PGX editor tests
// ES: Macro para tests de editor PGX
#define PGX_TEST_EDITOR(TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "PGX.Editor." #TestName, \
		EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter) \
	bool TestName::RunTest(const FString& /*Parameters*/)

// EN: Macro for PGX performance/stress tests
// ES: Macro para tests de rendimiento/stress PGX
#define PGX_TEST_PERF(TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, "PGX.Performance." #TestName, \
		EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::PerfFilter) \
	bool TestName::RunTest(const FString& /*Parameters*/)

/**
 * EN: Utility class with common test helpers for PGX.
 * ES: Clase utilitaria con helpers comunes de test para PGX.
 */
class FPGXTestUtils
{
public:
	// EN: Create a temporary world for testing / ES: Crear un mundo temporal para testing
	static UWorld* CreateTestWorld();

	// EN: Destroy a test world / ES: Destruir un mundo de test
	static void DestroyTestWorld(UWorld* World);

	// EN: Get or create a GameInstance for testing / ES: Obtener o crear un GameInstance para testing
	static UGameInstance* GetTestGameInstance();
};
