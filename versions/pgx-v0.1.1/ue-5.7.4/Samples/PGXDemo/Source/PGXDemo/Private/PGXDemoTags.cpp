// SPDX-License-Identifier: Apache-2.0
#include "PGXDemoTags.h"
namespace
{
    FGameplayTag Resolve(const TCHAR* Name)
    {
        return FGameplayTag::RequestGameplayTag(FName(Name), false);
    }
}
FGameplayTag PGXDemoTags::EventInteracted() { static const FGameplayTag Tag=Resolve(TEXT("PGX.Demo.Event.Interacted")); return Tag; }
FGameplayTag PGXDemoTags::FlowConfig() { static const FGameplayTag Tag=Resolve(TEXT("PGX.Demo.Flow.Config")); return Tag; }
FGameplayTag PGXDemoTags::FlowReady() { static const FGameplayTag Tag=Resolve(TEXT("PGX.Demo.Flow.Ready")); return Tag; }
FGameplayTag PGXDemoTags::FlowInteracted() { static const FGameplayTag Tag=Resolve(TEXT("PGX.Demo.Flow.Interacted")); return Tag; }
FGameplayTag PGXDemoTags::InputInteract() { static const FGameplayTag Tag=Resolve(TEXT("PGX.Demo.Input.Interact")); return Tag; }
FGameplayTag PGXDemoTags::SaveContext() { static const FGameplayTag Tag=Resolve(TEXT("PGX.Demo.Save.Context")); return Tag; }
FGameplayTag PGXDemoTags::SaveDomainProgress() { static const FGameplayTag Tag=Resolve(TEXT("PGX.Demo.Save.Domain.Progress")); return Tag; }
