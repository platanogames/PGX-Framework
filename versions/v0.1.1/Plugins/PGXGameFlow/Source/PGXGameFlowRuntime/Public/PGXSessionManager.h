// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXSessionManager.generated.h"

/**
 * EN: Game session manager.
 *     Tracks session lifecycle, play time, and session statistics.
 *
 * ES: Manager de sesion de juego.
 *     Trackea ciclo de vida de sesion, tiempo de juego
 *     y estadisticas de sesion.
 */
UCLASS()
class PGXGAMEFLOWRUNTIME_API UPGXSessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface
};
