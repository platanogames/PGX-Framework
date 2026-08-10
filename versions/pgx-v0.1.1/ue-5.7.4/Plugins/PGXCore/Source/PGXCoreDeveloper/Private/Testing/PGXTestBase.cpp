// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Testing/PGXTestBase.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

// EN: Test-world helpers currently return conservative stub values.
// ES: Los helpers de test-world actualmente devuelven valores stub conservadores.

UWorld* FPGXTestUtils::CreateTestWorld()
{
	// EN: Stub - will create a minimal test world
	// ES: Stub - creara un mundo de test minimo
	return nullptr;
}

void FPGXTestUtils::DestroyTestWorld(UWorld* World)
{
	// EN: Stub - will properly tear down test world
	// ES: Stub - destruira correctamente el mundo de test
}

UGameInstance* FPGXTestUtils::GetTestGameInstance()
{
	// EN: Stub - will create or get a test game instance
	// ES: Stub - creara u obtendra una instancia de juego de test
	return nullptr;
}
