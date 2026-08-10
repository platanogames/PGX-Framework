// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Testing/PGXTestSubsystemFixture.h"
#include "Testing/PGXTestBase.h"

FPGXTestSubsystemFixture::FPGXTestSubsystemFixture()
{
}

FPGXTestSubsystemFixture::~FPGXTestSubsystemFixture()
{
	Teardown();
}

void FPGXTestSubsystemFixture::Setup()
{
	// EN: Create isolated test world / ES: Crear mundo de test aislado
	TestWorld = FPGXTestUtils::CreateTestWorld();
}

void FPGXTestSubsystemFixture::Teardown()
{
	// EN: Clean up test world / ES: Limpiar mundo de test
	if (TestWorld)
	{
		FPGXTestUtils::DestroyTestWorld(TestWorld);
		TestWorld = nullptr;
	}
}
