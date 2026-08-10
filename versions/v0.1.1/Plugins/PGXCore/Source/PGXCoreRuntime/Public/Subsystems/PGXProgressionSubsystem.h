// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXProgressionSubsystem.generated.h"

/**
 * EN: General unlock and progression system. Manages unlockable content, achievement tracking,
 *     progression milestones, and integrates with PGXSave for persistence.
 *     Independent from GameFlow - this handles WHAT is unlocked, not WHEN game states change.
 * ES: Sistema general de desbloqueo y progresion. Gestiona contenido desbloqueable, tracking de logros,
 *     hitos de progresion, e integra con PGXSave para persistencia.
 *     Independiente de GameFlow - esto maneja QUE esta desbloqueado, no CUANDO cambian estados del juego.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface
};
