// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXInventorySubsystem.generated.h"

/**
 * EN: Global inventory manager.
 *     Tracks all inventories, handles item transfers,
 *     and manages item lifecycle.
 *
 * ES: Manager global de inventarios.
 *     Trackea todos los inventarios, maneja transferencias de items
 *     y gestiona ciclo de vida de items.
 */
UCLASS(BlueprintType)
class PGXINVENTORYRUNTIME_API UPGXInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface
};
