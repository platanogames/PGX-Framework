// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PGXDemoHUD.generated.h"
UCLASS()
class PGXDEMO_API APGXDemoHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
