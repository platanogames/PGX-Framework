// SPDX-License-Identifier: Apache-2.0
#include "PGXDemoGameMode.h"
#include "PGXDemoHUD.h"
#include "PGXDemoPlayerController.h"
#include "PGXDemoTags.h"
#include "Messages/PGXMessageSubsystem.h"
#include "PGXGameFlowSubsystem.h"
#include "PGXSaveSubsystem.h"
#include "PGXSaveGame.h"
#include "GameFramework/DefaultPawn.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
APGXDemoGameMode::APGXDemoGameMode()
{
    PlayerControllerClass = APGXDemoPlayerController::StaticClass();
    HUDClass = APGXDemoHUD::StaticClass();
    DefaultPawnClass = ADefaultPawn::StaticClass();
}
void APGXDemoGameMode::BeginPlay()
{
    Super::BeginPlay();
    UGameInstance* GI = GetGameInstance();
    MessageSubsystem = UPGXMessageSubsystem::Get(this);
    FlowSubsystem = GI ? GI->GetSubsystem<UPGXGameFlowSubsystem>() : nullptr;
    SaveSubsystem = GI ? GI->GetSubsystem<UPGXSaveSubsystem>() : nullptr;
    if (!MessageSubsystem || !FlowSubsystem || !SaveSubsystem)
    {
        LastOperationText = TEXT("FAIL: required PGX subsystem unavailable");
        UE_LOG(LogTemp, Error, TEXT("PGXDemo: %s"), *LastOperationText);
        return;
    }
    ListenerHandle = MessageSubsystem->RegisterListener<FPGXDemoInteractionMessage>(
        PGXDemoTags::EventInteracted(), this, &APGXDemoGameMode::HandleInteractionMessage);
    if (SaveSubsystem->DoesSlotExist(PGXDemoTags::SaveContext(), TEXT("DemoSlot")))
    {
        const EPGXSaveResult LoadResult = SaveSubsystem->LoadContext(PGXDemoTags::SaveContext(), TEXT("DemoSlot"));
        if (LoadResult == EPGXSaveResult::Success)
        {
            if (const UPGXSaveGame* Save = SaveSubsystem->GetSaveGame(PGXDemoTags::SaveDomainProgress()))
            {
                InteractionCount = Save->ReadInt(TEXT("InteractionCount"), 0);
            }
        }
    }
    LastOperationText = TEXT("Ready: press E to exercise PGX");
}
void APGXDemoGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ListenerHandle.Unregister();
    Super::EndPlay(EndPlayReason);
}
void APGXDemoGameMode::HandleInteractionMessage(FGameplayTag, const FPGXDemoInteractionMessage&)
{
    if (!FlowSubsystem || !SaveSubsystem) { LastOperationText = TEXT("FAIL: subsystem lost"); return; }
    ++InteractionCount;
    const FGameplayTag Current = FlowSubsystem->GetCurrentFlowTag(EPGXFlowChannel::Global);
    const FGameplayTag Destination = Current == PGXDemoTags::FlowReady()
        ? PGXDemoTags::FlowInteracted() : PGXDemoTags::FlowReady();
    const FPGXFlowResult FlowResult = FlowSubsystem->SetStateByTag(EPGXFlowChannel::Global, Destination, this);
    if (!FlowResult.bSuccess) { LastOperationText = TEXT("FAIL: GameFlow transition rejected"); return; }
    UPGXSaveGame* Save = SaveSubsystem->GetSaveGame(PGXDemoTags::SaveDomainProgress());
    if (!Save) { LastOperationText = TEXT("FAIL: Save domain unavailable"); return; }
    Save->WriteInt(TEXT("InteractionCount"), InteractionCount);
    const EPGXSaveResult SaveResult = SaveSubsystem->SaveContext(PGXDemoTags::SaveContext(), TEXT("DemoSlot"));
    LastOperationText = SaveResult == EPGXSaveResult::Success
        ? FString::Printf(TEXT("PASS: interaction %d persisted"), InteractionCount)
        : TEXT("FAIL: SaveContext rejected");
    if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, SaveResult == EPGXSaveResult::Success ? FColor::Green : FColor::Red, LastOperationText); }
}
FGameplayTag APGXDemoGameMode::GetCurrentFlowState() const
{
    return FlowSubsystem ? FlowSubsystem->GetCurrentFlowTag(EPGXFlowChannel::Global) : FGameplayTag();
}
