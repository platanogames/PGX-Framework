// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PGXWorldSubsystem.generated.h"

/**
 * EN: Base class for all PGX World subsystems.
 *     Per-world lifetime, with ordered init and integrated logging.
 * ES: Clase base para todos los subsistemas PGX de World.
 *     Lifetime por mundo, con init ordenada y logging integrado.
 */
UCLASS(Abstract)
class PGXCORERUNTIME_API UPGXWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface
};
