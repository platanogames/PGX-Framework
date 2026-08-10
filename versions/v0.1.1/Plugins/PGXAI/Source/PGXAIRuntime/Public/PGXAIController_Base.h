// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PGXAIController_Base.generated.h"

/**
 * EN: Base AI controller with integrated perception, blackboard, and debug support.
 * ES: Controller AI base con percepcion integrada, blackboard y soporte de depuracion.
 */
UCLASS(Abstract, Blueprintable)
class PGXAIRUNTIME_API APGXAIController_Base : public AAIController
{
	GENERATED_BODY()

public:
	APGXAIController_Base();
};
