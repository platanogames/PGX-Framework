// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXLevelTransitionSubsystem.generated.h"

/**
 * EN: Level transition subsystem.
 *     Handles level loading with transition screens and
 *     persistent data between levels.
 *
 * ES: Subsistema de transicion de niveles.
 *     Maneja carga de niveles con pantallas de transicion
 *     y datos persistentes entre niveles.
 */
UCLASS()
class PGXGAMEFLOWRUNTIME_API UPGXLevelTransitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface
};
