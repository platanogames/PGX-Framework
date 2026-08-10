// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "CoreMinimal.h"
#include "PGXDemoMessages.generated.h"
USTRUCT()
struct PGXDEMO_API FPGXDemoInteractionMessage
{
    GENERATED_BODY()
    UPROPERTY() TObjectPtr<UObject> Source = nullptr;
};
