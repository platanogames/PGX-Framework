// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionComponent.h"
#include "PGXPerceptionComponent.generated.h"

/**
 * EN: PGX wrapper over UAIPerceptionComponent. Provides simplified perception setup,
 *     event filtering by PGX categories, and integration with PGXAISubsystem.
 *     Following "Epic First": wraps AIPerception, doesn't replace it.
 *
 * ES: Wrapper PGX sobre UAIPerceptionComponent. Proporciona setup simplificado de percepcion,
 *     filtrado de eventos por categorias PGX, e integracion con PGXAISubsystem.
 *     Siguiendo "Epic First": wrappea AIPerception, no lo reemplaza.
 */
UCLASS(ClassGroup=(PGX), meta=(BlueprintSpawnableComponent))
class PGXAIRUNTIME_API UPGXPerceptionComponent : public UAIPerceptionComponent
{
	GENERATED_BODY()

public:
	UPGXPerceptionComponent();
};
