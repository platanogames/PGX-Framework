// Copyright PGX Framework. All Rights Reserved.

#include "PGXAttributeBridge.h"
#include "PGXAbilityRuntime.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "UObject/UnrealType.h"

UPGXAttributeBridge::UPGXAttributeBridge()
{
}

FString UPGXAttributeBridge::GetLeafSegment(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return FString();
	}

	FString TagString = Tag.ToString();
	int32 LastDotIndex = INDEX_NONE;
	if (TagString.FindLastChar(TEXT('.'), LastDotIndex))
	{
		return TagString.Mid(LastDotIndex + 1);
	}
	return TagString;
}

FGameplayAttribute UPGXAttributeBridge::ResolveAttributeByTag(const UAbilitySystemComponent* ASC, FGameplayTag AttributeTag)
{
	if (!ASC || !AttributeTag.IsValid())
	{
		return FGameplayAttribute();
	}

	const FString LeafName = GetLeafSegment(AttributeTag);
	if (LeafName.IsEmpty())
	{
		return FGameplayAttribute();
	}

	for (const UAttributeSet* AttributeSet : ASC->GetSpawnedAttributes())
	{
		if (!AttributeSet)
		{
			continue;
		}

		for (TFieldIterator<FProperty> PropertyIt(AttributeSet->GetClass()); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			if (!Property || Property->GetName() != LeafName)
			{
				continue;
			}

			const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (StructProperty && StructProperty->Struct == FGameplayAttributeData::StaticStruct())
			{
				return FGameplayAttribute(Property);
			}
		}
	}

	PGX_LOG_WARNING(LogPGXAbility, TEXT("UPGXAttributeBridge::ResolveAttributeByTag — no FGameplayAttributeData property named '%s' found for tag '%s' on any spawned AttributeSet."),
		*LeafName, *AttributeTag.ToString());

	return FGameplayAttribute();
}

TArray<TPair<FGameplayTag, FGameplayAttribute>> UPGXAttributeBridge::GetAllAttributesOnASC(const UAbilitySystemComponent* ASC)
{
	TArray<TPair<FGameplayTag, FGameplayAttribute>> Results;

	if (!ASC)
	{
		return Results;
	}

	for (const UAttributeSet* AttributeSet : ASC->GetSpawnedAttributes())
	{
		if (!AttributeSet)
		{
			continue;
		}

		for (TFieldIterator<FProperty> PropertyIt(AttributeSet->GetClass()); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			const FStructProperty* StructProperty = Property ? CastField<FStructProperty>(Property) : nullptr;
			if (!StructProperty || StructProperty->Struct != FGameplayAttributeData::StaticStruct())
			{
				continue;
			}

			// EN: Best-effort reverse mapping by convention. Non-erroring lookup: an unregistered
			//     tag yields an invalid FGameplayTag, not an assert/crash.
			// ES: Mapeo inverso best-effort por convencion. Lookup sin error: un tag no
			//     registrado produce un FGameplayTag invalido, no un assert/crash.
			const FString ReconstructedTagName = FString::Printf(TEXT("PGX.Attribute.Identity.%s"), *Property->GetName());
			const FGameplayTag ReconstructedTag = FGameplayTag::RequestGameplayTag(FName(*ReconstructedTagName), /*ErrorIfNotFound=*/false);

			Results.Add(TPair<FGameplayTag, FGameplayAttribute>(ReconstructedTag, FGameplayAttribute(Property)));
		}
	}

	return Results;
}

const UAttributeSet* UPGXAttributeBridge::FindOwningAttributeSet(const UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute)
{
	if (!ASC || !Attribute.IsValid())
	{
		return nullptr;
	}

	const UClass* AttributeSetClass = Attribute.GetAttributeSetClass();
	for (const UAttributeSet* Set : ASC->GetSpawnedAttributes())
	{
		if (Set && Set->IsA(AttributeSetClass))
		{
			return Set;
		}
	}

	return nullptr;
}
