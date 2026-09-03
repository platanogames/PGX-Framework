// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXEffectFacade.h"
#include "PGXAbilityRuntime.h"
#include "PGXAbilityComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Subsystems/PGXLogSubsystem.h"

UPGXEffectFacade::UPGXEffectFacade()
{
}

UAbilitySystemComponent* UPGXEffectFacade::ResolveASC() const
{
	if (const UPGXAbilityComponent* OwningComponent = GetTypedOuter<UPGXAbilityComponent>())
	{
		return OwningComponent->GetAbilitySystemComponentInternal();
	}
	return nullptr;
}

FPGXAbilityResult UPGXEffectFacade::ApplyEffect(TSubclassOf<UGameplayEffect> EffectClass, float Level, FPGXEffectHandle& OutHandle)
{
	OutHandle = FPGXEffectHandle();

	if (!EffectClass)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::InvalidInput, TEXT("ApplyEffect: EffectClass is null."));
	}

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ComponentUnavailable, TEXT("ApplyEffect: AbilitySystemComponent unavailable."));
	}

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, Level, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::Failure, TEXT("ApplyEffect: MakeOutgoingSpec failed."));
	}

	const FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	if (!ActiveHandle.IsValid())
	{
		// EN: An instant (non-duration) effect applies and returns an invalid handle by GAS
		//     design — that is success, not failure. Only report failure if the effect was
		//     rejected outright (e.g. application immunity); GAS does not expose a separate
		//     rejection signal here beyond the invalid handle, so we treat invalid-but-no-error
		//     as success for instant effects.
		// ES: Un efecto instantaneo (sin duracion) se aplica y retorna un handle invalido por
		//     diseno de GAS — eso es exito, no fallo.
		const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
		const bool bIsInstant = EffectCDO && EffectCDO->DurationPolicy == EGameplayEffectDurationType::Instant;
		if (bIsInstant)
		{
			return FPGXAbilityResult::MakeSuccess(TEXT("Instant effect applied (no active handle by design)."));
		}
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::Failure, TEXT("ApplyEffect: ApplyGameplayEffectSpecToSelf returned an invalid handle."));
	}

	const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
	OutHandle.EffectTag = (EffectCDO && EffectCDO->GetAssetTags().Num() > 0) ? *EffectCDO->GetAssetTags().CreateConstIterator() : FGameplayTag();
	OutHandle.SpecHandle = ActiveHandle;

	PGX_LOG_INFO(LogPGXAbility, TEXT("UPGXEffectFacade::ApplyEffect — %s"), *GetNameSafe(EffectClass));

	return FPGXAbilityResult::MakeSuccess();
}

FPGXAbilityResult UPGXEffectFacade::RemoveEffect(const FPGXEffectHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::InvalidInput, TEXT("RemoveEffect: Handle is invalid."));
	}

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ComponentUnavailable, TEXT("RemoveEffect: AbilitySystemComponent unavailable."));
	}

	const bool bRemoved = ASC->RemoveActiveGameplayEffect(Handle.SpecHandle);
	if (!bRemoved)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::NotFound, TEXT("RemoveEffect: effect handle not found/already removed."));
	}

	PGX_LOG_INFO(LogPGXAbility, TEXT("UPGXEffectFacade::RemoveEffect — %s"), *Handle.EffectTag.ToString());

	return FPGXAbilityResult::MakeSuccess();
}

int32 UPGXEffectFacade::RemoveEffectsByTag(FGameplayTag EffectTag)
{
	if (!EffectTag.IsValid())
	{
		return 0;
	}

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return 0;
	}

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(FGameplayTagContainer(EffectTag));
	return ASC->RemoveActiveEffects(Query);
}

bool UPGXEffectFacade::HasEffect(FGameplayTag EffectTag) const
{
	if (!EffectTag.IsValid())
	{
		return false;
	}

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return false;
	}

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(FGameplayTagContainer(EffectTag));
	return ASC->GetActiveEffects(Query).Num() > 0;
}

TArray<FPGXEffectSnapshot> UPGXEffectFacade::GetActiveEffects() const
{
	TArray<FPGXEffectSnapshot> Snapshots;

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return Snapshots;
	}

	const FGameplayEffectQuery MatchAllQuery; // EN: Default-constructed query matches all active effects. / ES: Query por defecto matchea todos los efectos activos.
	for (const FActiveGameplayEffectHandle& Handle : ASC->GetActiveEffects(MatchAllQuery))
	{
		FPGXEffectSnapshot Snapshot;
		Snapshot.Handle.SpecHandle = Handle;

		if (const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(Handle))
		{
			Snapshot.Handle.EffectTag = ActiveEffect->Spec.Def && ActiveEffect->Spec.Def->GetAssetTags().Num() > 0
				? *ActiveEffect->Spec.Def->GetAssetTags().CreateConstIterator()
				: FGameplayTag();
			Snapshot.StackCount = ActiveEffect->Spec.GetStackCount();

			// EN: UAbilitySystemComponent::GetGameplayEffectTimeRemaining does not exist in UE5.7
			//     (build-fail caught by governor). Read remaining time from the FActiveGameplayEffect
			//     struct directly instead.
			// ES: GetGameplayEffectTimeRemaining no existe en UE5.7. Leer el tiempo restante
			//     directamente del struct FActiveGameplayEffect.
			if (const UWorld* World = ASC->GetWorld())
			{
				Snapshot.RemainingDurationSeconds = ActiveEffect->GetTimeRemaining(World->GetTimeSeconds());
			}
		}

		Snapshots.Add(Snapshot);
	}

	return Snapshots;
}

float UPGXEffectFacade::GetEffectRemainingDuration(const FPGXEffectHandle& Handle) const
{
	if (!Handle.IsValid())
	{
		return 0.0f;
	}

	const UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return 0.0f;
	}

	const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(Handle.SpecHandle);
	const UWorld* World = ASC->GetWorld();
	if (!ActiveEffect || !World)
	{
		return 0.0f;
	}

	return ActiveEffect->GetTimeRemaining(World->GetTimeSeconds());
}
