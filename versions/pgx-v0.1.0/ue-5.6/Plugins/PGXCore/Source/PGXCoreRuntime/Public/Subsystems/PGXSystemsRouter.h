// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PGXSystemsRouter.generated.h"

/**
 * EN: Service locator and access pattern for PGX systems. Provides a clean API
 *     for cross-system communication without direct coupling between plugins.
 * ES: Localizador de servicios y patron de acceso para sistemas PGX. Proporciona una API limpia
 *     para comunicacion entre sistemas sin acoplamiento directo entre plugins.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXSystemsRouter : public UObject
{
	GENERATED_BODY()
};
