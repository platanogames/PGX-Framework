// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Construction/PGXHUDConstruction.h"

UPGXHUDConstruction::UPGXHUDConstruction()
{
	// EN: HUD listens to all bridges by default (UI coordinator)
	// ES: HUD escucha todos los bridges por defecto (coordinador de UI)
	bListenToGameFlowChanges = true;
	bListenToLevelTransitions = true;
	bListenToLoadingScreenState = true;
}
