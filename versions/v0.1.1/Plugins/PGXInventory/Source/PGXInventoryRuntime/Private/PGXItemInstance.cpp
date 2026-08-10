// Copyright PGX Framework. All Rights Reserved.

#include "PGXItemInstance.h"

#include "PGXItemDefinition.h"

void UPGXItemInstance::InitializeItem(const UPGXItemDefinition* InDefinition, int32 InStackCount)
{
	Definition = InDefinition;
	const int32 MaxStackSize = InDefinition ? FMath::Max(1, InDefinition->MaxStackSize) : 1;
	StackCount = FMath::Clamp(InStackCount, 1, MaxStackSize);
}

bool UPGXItemInstance::CanStackWith(const UPGXItemDefinition* OtherDefinition) const
{
	return Definition == OtherDefinition && Definition != nullptr && StackCount < FMath::Max(1, Definition->MaxStackSize);
}

int32 UPGXItemInstance::GetAvailableStackSpace() const
{
	if (!Definition)
	{
		return 0;
	}
	return FMath::Max(0, FMath::Max(1, Definition->MaxStackSize) - StackCount);
}

float UPGXItemInstance::GetStackWeight() const
{
	return Definition ? FMath::Max(0.0f, Definition->Weight) * static_cast<float>(StackCount) : 0.0f;
}
