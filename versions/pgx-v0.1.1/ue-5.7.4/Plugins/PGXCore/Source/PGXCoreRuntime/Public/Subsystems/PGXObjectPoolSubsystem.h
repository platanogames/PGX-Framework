// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXObjectPoolSubsystem.generated.h"

/**
 * EN: Central object pool manager.
 *     Manages pools by class type, handles budget and lifecycle
 *     of pooled objects.
 * ES: Manager central de object pools.
 *     Gestiona pools por tipo de clase, maneja budget y ciclo de vida
 *     de objetos en pool.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXObjectPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface
};
