// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXSettingsSubsystem.generated.h"

/**
 * EN: Centralized settings management.
 *     Handles persistent user settings, per-module configuration,
 *     and change notification delegates.
 * ES: Gestion centralizada de settings.
 *     Maneja settings de usuario persistentes, configuracion por modulo,
 *     y delegados de notificacion de cambios.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface
};
