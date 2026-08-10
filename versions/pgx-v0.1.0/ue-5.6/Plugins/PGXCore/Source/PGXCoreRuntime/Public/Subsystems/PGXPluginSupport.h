// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXPluginSupport.generated.h"

/**
 * EN: Conditional plugin support system. Manages optional plugin detection,
 *     feature flags based on available plugins, and graceful degradation
 *     when optional PGX plugins are not loaded.
 *
 * ES: Sistema de soporte de plugins condicionales. Gestiona deteccion de plugins opcionales,
 *     feature flags basados en plugins disponibles, y degradacion elegante
 *     cuando plugins PGX opcionales no estan cargados.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXPluginSupport : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface
};
