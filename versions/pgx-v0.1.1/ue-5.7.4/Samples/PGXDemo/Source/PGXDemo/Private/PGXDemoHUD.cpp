// SPDX-License-Identifier: Apache-2.0
#include "PGXDemoHUD.h"
#include "PGXDemoGameMode.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
void APGXDemoHUD::DrawHUD()
{
    Super::DrawHUD();
    const APGXDemoGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<APGXDemoGameMode>() : nullptr;
    const FString State = Mode ? Mode->GetCurrentFlowState().ToString() : TEXT("Unavailable");
    const FString Text = Mode
        ? FString::Printf(TEXT("PGX Demo - press E\nMessage + GameFlow + Save + InputBuffer\nCount: %d\nFlow: %s\n%s"), Mode->GetInteractionCount(), *State, *Mode->GetLastOperationText())
        : TEXT("PGX Demo - GameMode unavailable");
    DrawText(Text, FLinearColor::White, 40.0f, 40.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 1.25f, false);
}
