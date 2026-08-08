// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "PGXEngineSubsystem.generated.h"

/**
 * EN: Base class for all PGX Engine subsystems.
 *     Provides ordered initialization, dependency declaration,
 *     integrated logging, and configuration support.
 *     Engine subsystems live for the entire application lifetime.
 * ES: Clase base para todos los subsistemas PGX de Engine.
 *     Proporciona inicializacion ordenada, declaracion de dependencias,
 *     logging integrado y soporte de configuracion.
 *     Los subsistemas de Engine viven durante toda la vida de la aplicacion.
 */
UCLASS(Abstract)
class PGXCORERUNTIME_API UPGXEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface
};
