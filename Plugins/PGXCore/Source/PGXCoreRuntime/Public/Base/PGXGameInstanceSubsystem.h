// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXGameInstanceSubsystem.generated.h"

/**
 * EN: Base class for all PGX GameInstance subsystems.
 *     Provides ordered initialization, dependency declaration,
 *     integrated logging, and configuration support.
 * ES: Clase base para todos los subsistemas PGX de GameInstance.
 *     Proporciona inicializacion ordenada, declaracion de dependencias,
 *     logging integrado y soporte de configuracion.
 */
UCLASS(Abstract)
class PGXCORERUNTIME_API UPGXGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface
};
