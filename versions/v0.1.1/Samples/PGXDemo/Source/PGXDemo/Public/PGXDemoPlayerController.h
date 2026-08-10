// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PGXDemoPlayerController.generated.h"
UCLASS()
class PGXDEMO_API APGXDemoPlayerController : public APlayerController
{
    GENERATED_BODY()
protected:
    virtual void SetupInputComponent() override;
private:
    void HandleInteract();
#if WITH_DEV_AUTOMATION_TESTS
public:
    void TriggerInteractionForAutomation() { HandleInteract(); }
#endif
};
