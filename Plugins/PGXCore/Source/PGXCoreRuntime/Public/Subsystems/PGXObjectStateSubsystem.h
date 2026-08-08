// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PGXObjectStateSubsystem.generated.h"

/**
 * EN: Manages persistent states of world objects. Tracks object states (active, inactive,
 *     destroyed, locked, etc.) across level loads and save/load cycles.
 * ES: Gestiona estados persistentes de objetos del mundo. Rastrea estados de objetos (activo, inactivo,
 *     destruido, bloqueado, etc.) a traves de cargas de nivel y ciclos de guardado/carga.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXObjectStateSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface
};
