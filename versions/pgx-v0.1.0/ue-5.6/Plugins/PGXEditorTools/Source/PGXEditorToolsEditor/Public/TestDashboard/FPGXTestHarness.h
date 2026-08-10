// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "Construction/PGXConstructionSettings.h"

class UWorld;

/**
 * EN: Test harness for injecting transient test data into subsystems.
 *     Creates minimal valid DAs, injects them into 6 subsystem caches
 *     (Audio, PSO, Save, Construction, Message, EventHandler),
 *     and cleans up on teardown.
 *     All objects use RF_Transient + GetTransientPackage().
 *     Philosophy: creates a COMPLETE simulated environment — does not validate user config.
 * ES: Harness de test para inyectar datos transient en subsistemas.
 *     Crea DAs validos minimos, los inyecta en 6 caches de subsistemas
 *     (Audio, PSO, Save, Construction, Message, EventHandler),
 *     y limpia en teardown.
 *     Todos los objetos usan RF_Transient + GetTransientPackage().
 *     Filosofia: crea un entorno simulado COMPLETO — no valida config del usuario.
 */
class FPGXTestHarness
{
public:
	/** EN: Create transient test data and inject into subsystem caches / ES: Crear datos de test transient e inyectar en caches de subsistemas */
	void Setup(UWorld* PIEWorld);

	/** EN: Remove injected data from subsystem caches and release strong refs / ES: Remover datos inyectados de caches de subsistemas y liberar strong refs */
	void Teardown();

	/** EN: Whether harness data is currently injected / ES: Si los datos del harness estan actualmente inyectados */
	bool IsActive() const { return bIsActive; }

private:
	void SetupAudio(UWorld* World);
	void SetupPSO(UWorld* World);
	void SetupSave(UWorld* World);
	void SetupConstruction();
	void SetupMessage(UWorld* World);
	void SetupEventHandler(UWorld* World);

	void TeardownAudio(UWorld* World);
	void TeardownPSO(UWorld* World);
	void TeardownSave(UWorld* World);
	void TeardownConstruction();
	void TeardownMessage(UWorld* World);
	void TeardownEventHandler(UWorld* World);

	/** EN: Strong refs to keep transient objects alive during test / ES: Refs fuertes para mantener objetos transient vivos durante test */
	TArray<TStrongObjectPtr<UObject>> CreatedObjects;

	/** EN: Saved Construction class sources — restored on teardown / ES: Fuentes de clase de Construction guardadas — restauradas en teardown */
	struct FSavedClassSources
	{
		EPGXClassSourceMode GameMode = EPGXClassSourceMode::Default;
		EPGXClassSourceMode PlayerController = EPGXClassSourceMode::Default;
		EPGXClassSourceMode GameState = EPGXClassSourceMode::Default;
		EPGXClassSourceMode PlayerState = EPGXClassSourceMode::Default;
		EPGXClassSourceMode Character = EPGXClassSourceMode::Default;
		EPGXClassSourceMode Pawn = EPGXClassSourceMode::Default;
		EPGXClassSourceMode HUD = EPGXClassSourceMode::Default;
		bool bSaved = false;
	};
	FSavedClassSources SavedSources;

	bool bIsActive = false;
};
