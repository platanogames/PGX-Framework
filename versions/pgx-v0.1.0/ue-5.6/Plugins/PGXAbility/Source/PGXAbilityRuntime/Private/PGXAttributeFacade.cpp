// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAttributeFacade.h"
#include "PGXAbilityRuntime.h"
#include "PGXAbilityComponent.h"
#include "PGXAttributeBridge.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Subsystems/PGXLogSubsystem.h"

UPGXAttributeFacade::UPGXAttributeFacade()
{
}

UAbilitySystemComponent* UPGXAttributeFacade::ResolveASC() const
{
	if (const UPGXAbilityComponent* OwningComponent = GetTypedOuter<UPGXAbilityComponent>())
	{
		return OwningComponent->GetAbilitySystemComponentInternal();
	}
	return nullptr;
}

float UPGXAttributeFacade::GetAttributeValue(FGameplayTag AttributeTag) const
{
	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return 0.0f;
	}

	const FGameplayAttribute Attribute = UPGXAttributeBridge::ResolveAttributeByTag(ASC, AttributeTag);
	if (!Attribute.IsValid())
	{
		return 0.0f;
	}

	return ASC->GetNumericAttribute(Attribute);
}

float UPGXAttributeFacade::GetAttributeBaseValue(FGameplayTag AttributeTag) const
{
	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return 0.0f;
	}

	const FGameplayAttribute Attribute = UPGXAttributeBridge::ResolveAttributeByTag(ASC, AttributeTag);
	if (!Attribute.IsValid())
	{
		return 0.0f;
	}

	const UAttributeSet* OwningSet = UPGXAttributeBridge::FindOwningAttributeSet(ASC, Attribute);
	const FGameplayAttributeData* Data = Attribute.GetGameplayAttributeData(OwningSet);
	return Data ? Data->GetBaseValue() : 0.0f;
}

FPGXAbilityResult UPGXAttributeFacade::SetAttributeBaseValue(FGameplayTag AttributeTag, float NewValue)
{
	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ComponentUnavailable, TEXT("SetAttributeBaseValue: AbilitySystemComponent unavailable."));
	}

	const FGameplayAttribute Attribute = UPGXAttributeBridge::ResolveAttributeByTag(ASC, AttributeTag);
	if (!Attribute.IsValid())
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::AttributeNotFound,
			FString::Printf(TEXT("SetAttributeBaseValue: no attribute resolved for tag '%s'."), *AttributeTag.ToString()));
	}

	ASC->SetNumericAttributeBase(Attribute, NewValue);

	PGX_LOG_VERBOSE(LogPGXAbility, TEXT("UPGXAttributeFacade::SetAttributeBaseValue — %s = %f"), *AttributeTag.ToString(), NewValue);

	return FPGXAbilityResult::MakeSuccess();
}

// EN: Known limitation: this clamps the BASE value only. CurrentValue = BaseValue + active
//     GameplayEffect modifiers, so an attribute with active additive effects can still read
//     outside [Min, Max] on CurrentValue even after this call. Enforcing CurrentValue bounds
//     requires the project's UAttributeSet to override PreAttributeChange/PostGameplayEffectExecute
//     (project responsibility — the AttributeSet class itself is project-authored content, out of
//     this generic facade's scope). Documented here rather than silently assumed correct.
// ES: Limitacion conocida: esto clampa solo el valor BASE. CurrentValue = BaseValue + modificadores
//     de GameplayEffect activos, asi que un atributo con efectos aditivos activos puede seguir
//     leyendo fuera de [Min, Max] en CurrentValue incluso despues de esta llamada. Forzar los
//     bounds de CurrentValue requiere que el UAttributeSet del proyecto override
//     PreAttributeChange/PostGameplayEffectExecute (responsabilidad de proyecto).
FPGXAbilityResult UPGXAttributeFacade::ClampAttributeValue(FGameplayTag AttributeTag, float Min, float Max)
{
	if (Min > Max)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::InvalidInput, TEXT("ClampAttributeValue: Min must be <= Max."));
	}

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::ComponentUnavailable, TEXT("ClampAttributeValue: AbilitySystemComponent unavailable."));
	}

	const FGameplayAttribute Attribute = UPGXAttributeBridge::ResolveAttributeByTag(ASC, AttributeTag);
	if (!Attribute.IsValid())
	{
		return FPGXAbilityResult::MakeFailure(EPGXAbilityResultCode::AttributeNotFound,
			FString::Printf(TEXT("ClampAttributeValue: no attribute resolved for tag '%s'."), *AttributeTag.ToString()));
	}

	const UAttributeSet* OwningSet = UPGXAttributeBridge::FindOwningAttributeSet(ASC, Attribute);
	const FGameplayAttributeData* Data = Attribute.GetGameplayAttributeData(OwningSet);
	const float CurrentBase = Data ? Data->GetBaseValue() : 0.0f;
	const float Clamped = FMath::Clamp(CurrentBase, Min, Max);

	ASC->SetNumericAttributeBase(Attribute, Clamped);

	return FPGXAbilityResult::MakeSuccess(FString::Printf(TEXT("Clamped to [%f, %f] -> %f"), Min, Max, Clamped));
}

bool UPGXAttributeFacade::HasAttribute(FGameplayTag AttributeTag) const
{
	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return false;
	}

	return UPGXAttributeBridge::ResolveAttributeByTag(ASC, AttributeTag).IsValid();
}

TArray<FPGXAttributeSnapshot> UPGXAttributeFacade::GetAttributeSnapshot() const
{
	TArray<FPGXAttributeSnapshot> Snapshots;

	UAbilitySystemComponent* ASC = ResolveASC();
	if (!ASC)
	{
		return Snapshots;
	}

	for (const TPair<FGameplayTag, FGameplayAttribute>& Pair : UPGXAttributeBridge::GetAllAttributesOnASC(ASC))
	{
		const FGameplayAttribute& Attribute = Pair.Value;
		const UAttributeSet* OwningSet = UPGXAttributeBridge::FindOwningAttributeSet(ASC, Attribute);
		const FGameplayAttributeData* Data = Attribute.GetGameplayAttributeData(OwningSet);

		FPGXAttributeSnapshot Snapshot;
		Snapshot.AttributeTag = Pair.Key;
		Snapshot.CurrentValue = Data ? Data->GetCurrentValue() : 0.0f;
		Snapshot.BaseValue = Data ? Data->GetBaseValue() : 0.0f;
		// EN: Clamp bounds are not GAS-native per-attribute metadata; default-config bounds from
		//     UPGXAbilityConfig apply unless the project clamps explicitly via ClampAttributeValue.
		// ES: Los bounds de clamp no son metadata nativa de GAS por atributo; se aplican los
		//     bounds default de UPGXAbilityConfig salvo clamp explicito del proyecto.
		Snapshot.ClampMin = 0.0f;
		Snapshot.ClampMax = 0.0f;
		Snapshots.Add(Snapshot);
	}

	return Snapshots;
}
