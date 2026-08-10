// SPDX-License-Identifier: Apache-2.0
#include "PGXDemoPlayerController.h"
#include "PGXDemoMessages.h"
#include "PGXDemoTags.h"
#include "PGXInputSubsystem.h"
#include "PGXInputBuffer.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
void APGXDemoPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    check(InputComponent);
    InputComponent->BindKey(EKeys::E, IE_Pressed, this, &APGXDemoPlayerController::HandleInteract);
}
void APGXDemoPlayerController::HandleInteract()
{
    UGameInstance* GI = GetGameInstance();
    UPGXInputSubsystem* Input = GI ? GI->GetSubsystem<UPGXInputSubsystem>() : nullptr;
    UPGXMessageSubsystem* Message = UPGXMessageSubsystem::Get(this);
    UPGXInputBuffer* Buffer = Input ? Input->GetInputBuffer() : nullptr;
    if (!Buffer || !Message) { UE_LOG(LogTemp, Error, TEXT("PGXDemo: InputBuffer or Message unavailable")); return; }
    Buffer->RecordInput(PGXDemoTags::InputInteract(), FVector(1.0, 0.0, 0.0));
    if (!Buffer->ConsumeRecentInput(PGXDemoTags::InputInteract())) { UE_LOG(LogTemp, Error, TEXT("PGXDemo: buffered input was not consumable")); return; }
    FPGXDemoInteractionMessage Payload;
    Payload.Source = this;
    Message->BroadcastMessage(PGXDemoTags::EventInteracted(), Payload);
}
