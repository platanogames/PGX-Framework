// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"

/**
 * EN: Test fixture for PGX subsystem tests. Provides a controlled environment
 *     with a GameInstance and World for testing subsystems that need initialization.
 *     Creates isolated test contexts to avoid cross-test contamination.
 *
 * ES: Fixture de test para tests de subsistemas PGX. Proporciona un entorno controlado
 *     con GameInstance y World para testear subsistemas que necesitan inicializacion.
 *     Crea contextos de test aislados para evitar contaminacion entre tests.
 */
class FPGXTestSubsystemFixture
{
public:
	FPGXTestSubsystemFixture();
	~FPGXTestSubsystemFixture();

	// EN: Setup and teardown / ES: Setup y teardown
	void Setup();
	void Teardown();

	// EN: Access the test world / ES: Acceder al mundo de test
	UWorld* GetWorld() const { return TestWorld; }

private:
	UWorld* TestWorld = nullptr;
};
