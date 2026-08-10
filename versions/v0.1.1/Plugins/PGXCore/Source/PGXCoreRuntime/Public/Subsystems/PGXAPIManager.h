// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXAPIManager.generated.h"

/**
 * EN: Base manager for external API integrations. Provides standardized patterns
 *     for HTTP requests, response handling, retry logic, and API key management.
 *     Concrete API integrations derive from this base.
 *
 * ES: Manager base para integraciones con APIs externas. Proporciona patrones estandarizados
 *     para peticiones HTTP, manejo de respuestas, logica de reintentos, y gestion de API keys.
 *     Integraciones concretas de API derivan de esta base.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXAPIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface
};
