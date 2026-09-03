// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAbilityFacade.h"
#include "PGXAbilityRuntime.h"
#include "PGXAbilityComponent.h"
#include "PGXAbilitySubsystem.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UPGXAbilityFacade::UPGXAbilityFacade()
{
}

UAbilitySystemComponent* UPGXAbilityFacade::ResolveASC() const
{
	if (const UPGXAbilityComponent* OwningComponent = GetTypedOuter<UPGXAbilityComponent>())
	{
		return OwningComponent->GetAbilitySystemComponentInternal();
	}
	return nullptr;
}

FPGXAbilityResult UPGXAbilityFacade::GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level, FPGXAbilityHandle& OutHandle)
{
	OutHandle = FPGXAbilityHandle();

	if (!AbilityClass)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::InvalidInput, TEXT("GrantAbility: AbilityClass is null."));
	}

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ComponentUnavailable, TEXT("GrantAbility: AbilitySystemComponent unavailable."));
	}

	const UGameplayAbility* AbilityCDO = AbilityClass->GetDefaultObject<UGameplayAbility>();
	const FGameplayTag IdentityTag = AbilityCDO && AbilityCDO->AbilityTags.Num() > 0
		? AbilityCDO->AbilityTags.First()
		: FGameplayTag();

	// EN: Idempotency — if an ability with this identity tag is already granted, return its handle.
	// ES: Idempotencia — si ya hay una ability con este tag concedida, retornar su handle.
	if (IdentityTag.IsValid())
	{
		for (const FGameplayAbilitySpec& ExistingSpec : ASC->GetActivatableAbilities())
		{
			if (ExistingSpec.Ability && ExistingSpec.Ability->AbilityTags.HasTag(IdentityTag))
			{
				OutHandle.AbilityTag = IdentityTag;
				OutHandle.SpecHandle = ExistingSpec.Handle;
				return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::AlreadyGranted,
					FString::Printf(TEXT("GrantAbility: '%s' already granted."), *IdentityTag.ToString()));
			}
		}
	}

	const FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, Level, INDEX_NONE, this));
	if (!SpecHandle.IsValid())
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::Failure, TEXT("GrantAbility: GiveAbility returned an invalid handle."));
	}

	OutHandle.AbilityTag = IdentityTag;
	OutHandle.SpecHandle = SpecHandle;

	PGX_LOG_INFO(LogPGXAbility, TEXT("UPGXAbilityFacade::GrantAbility — %s"), *GetNameSafe(AbilityClass));

	return FPGXAbilityResult::MakeSuccess();
}

FPGXAbilityResult UPGXAbilityFacade::RevokeAbility(const FPGXAbilityHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::InvalidInput, TEXT("RevokeAbility: Handle is invalid."));
	}

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ComponentUnavailable, TEXT("RevokeAbility: AbilitySystemComponent unavailable."));
	}

	if (!ASC->FindAbilitySpecFromHandle(Handle.SpecHandle))
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::NotFound, TEXT("RevokeAbility: ability handle not found."));
	}

	ASC->ClearAbility(Handle.SpecHandle);

	PGX_LOG_INFO(LogPGXAbility, TEXT("UPGXAbilityFacade::RevokeAbility — %s"), *Handle.AbilityTag.ToString());

	return FPGXAbilityResult::MakeSuccess();
}

FPGXAbilityResult UPGXAbilityFacade::ActivateAbilityByTag(FGameplayTag AbilityTag)
{
	if (!AbilityTag.IsValid())
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::InvalidInput, TEXT("ActivateAbilityByTag: AbilityTag is invalid."));
	}

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ComponentUnavailable, TEXT("ActivateAbilityByTag: AbilitySystemComponent unavailable."));
	}

	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
	if (!bActivated)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ActivationFailed,
			FString::Printf(TEXT("ActivateAbilityByTag: '%s' failed to activate (not granted, on cooldown, blocked, or cost unmet)."), *AbilityTag.ToString()));
	}

	PGX_LOG_INFO(LogPGXAbility, TEXT("UPGXAbilityFacade::ActivateAbilityByTag — %s"), *AbilityTag.ToString());

	// EN: Fan-in to the subsystem's native delegate (Inspector reactive-refresh hook,
	//     architecture design section 6.1). Was missing — broadcast never fired (required for correct runtime behavior).
	// ES: Fan-in al delegate nativo del subsistema. Faltaba — nunca se disparaba.
	if (UPGXAbilityComponent* OwningComponent = GetTypedOuter<UPGXAbilityComponent>())
	{
		if (const UWorld* World = OwningComponent->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (UPGXAbilitySubsystem* Subsystem = GameInstance->GetSubsystem<UPGXAbilitySubsystem>())
				{
					Subsystem->OnAbilityActivatedNative.Broadcast(OwningComponent, AbilityTag);
				}
			}
		}
	}

	return FPGXAbilityResult::MakeSuccess();
}

FPGXAbilityResult UPGXAbilityFacade::CancelAbility(const FPGXAbilityHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::InvalidInput, TEXT("CancelAbility: Handle is invalid."));
	}

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ComponentUnavailable, TEXT("CancelAbility: AbilitySystemComponent unavailable."));
	}

	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle.SpecHandle);
	if (!Spec)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::NotFound, TEXT("CancelAbility: ability handle not found."));
	}

	ASC->CancelAbilityHandle(Handle.SpecHandle);

	PGX_LOG_INFO(LogPGXAbility, TEXT("UPGXAbilityFacade::CancelAbility — %s"), *Handle.AbilityTag.ToString());

	return FPGXAbilityResult::MakeSuccess();
}

bool UPGXAbilityFacade::IsAbilityGranted(FGameplayTag AbilityTag) const
{
	if (!AbilityTag.IsValid())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag))
		{
			return true;
		}
	}
	return false;
}

bool UPGXAbilityFacade::IsAbilityActive(const FPGXAbilityHandle& Handle) const
{
	if (!Handle.IsValid())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return false;
	}

	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle.SpecHandle);
	return Spec && Spec->IsActive();
}

TArray<FPGXAbilitySnapshot> UPGXAbilityFacade::GetGrantedAbilities() const
{
	TArray<FPGXAbilitySnapshot> Snapshots;

	const UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return Snapshots;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		FPGXAbilitySnapshot Snapshot;
		Snapshot.Handle.SpecHandle = Spec.Handle;
		Snapshot.Handle.AbilityTag = (Spec.Ability && Spec.Ability->AbilityTags.Num() > 0)
			? Spec.Ability->AbilityTags.First()
			: FGameplayTag();
		Snapshot.bIsActive = Spec.IsActive();
		Snapshot.Level = Spec.Level;
		Snapshot.CooldownRemainingSeconds = GetCooldownRemaining(Snapshot.Handle.AbilityTag);
		Snapshots.Add(Snapshot);
	}

	return Snapshots;
}

float UPGXAbilityFacade::GetCooldownRemaining(FGameplayTag AbilityTag) const
{
	if (!AbilityTag.IsValid())
	{
		return 0.0f;
	}

	const UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return 0.0f;
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->AbilityTags.HasTag(AbilityTag))
		{
			float TimeRemaining = 0.0f;
			float CooldownDuration = 0.0f;
			Spec.Ability->GetCooldownTimeRemainingAndDuration(Spec.Handle, ASC->AbilityActorInfo.Get(), TimeRemaining, CooldownDuration);
			return TimeRemaining;
		}
	}

	return 0.0f;
}
