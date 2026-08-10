// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "CoreMinimal.h"
#include "Base/PGXGameModeBase.h"
#include "Messages/PGXMessage.h"
#include "PGXSaveTypes.h"
#include "PGXDemoMessages.h"
#include "PGXDemoGameMode.generated.h"
class UPGXMessageSubsystem;
class UPGXGameFlowSubsystem;
class UPGXSaveSubsystem;
UCLASS()
class PGXDEMO_API APGXDemoGameMode : public APGXGameModeBase
{
    GENERATED_BODY()
public:
    APGXDemoGameMode();
    int32 GetInteractionCount() const { return InteractionCount; }
    FGameplayTag GetCurrentFlowState() const;
    FString GetLastOperationText() const { return LastOperationText; }
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
    void HandleInteractionMessage(FGameplayTag Channel, const FPGXDemoInteractionMessage& Message);
    UPROPERTY() TObjectPtr<UPGXMessageSubsystem> MessageSubsystem;
    UPROPERTY() TObjectPtr<UPGXGameFlowSubsystem> FlowSubsystem;
    UPROPERTY() TObjectPtr<UPGXSaveSubsystem> SaveSubsystem;
    FPGXMessageListenerHandle ListenerHandle;
    int32 InteractionCount = 0;
    FString LastOperationText = TEXT("Waiting for input");
};
