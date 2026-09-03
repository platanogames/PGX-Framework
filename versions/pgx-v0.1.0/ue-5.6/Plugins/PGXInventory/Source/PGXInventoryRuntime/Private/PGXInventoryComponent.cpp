// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXInventoryComponent.h"

#include "PGXItemDefinition.h"
#include "PGXItemInstance.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"

UPGXInventoryComponent::UPGXInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FPGXInventoryResult UPGXInventoryComponent::AddItem(const UPGXItemDefinition* Definition, int32 Quantity)
{
	if (!Definition)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInventory: AddItem rejected null definition"));
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::InvalidDefinition, Definition, Quantity, TEXT("Invalid item definition."));
	}
	if (Quantity <= 0)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInventory: AddItem rejected invalid quantity %d"), Quantity);
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::InvalidQuantity, Definition, Quantity, TEXT("Invalid item quantity."));
	}

	EPGXInventoryResultCode FailureCode = EPGXInventoryResultCode::InternalError;
	FString FailureMessage;
	if (!CanAcceptQuantity(Definition, Quantity, FailureCode, FailureMessage))
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInventory: AddItem rejected %d units: %s"), Quantity, *FailureMessage);
		return FPGXInventoryResult::Failure(FailureCode, Definition, Quantity, FailureMessage);
	}

	int32 RemainingQuantity = Quantity;
	for (UPGXItemInstance* Item : Items)
	{
		if (!Item || !Item->CanStackWith(Definition))
		{
			continue;
		}

		const int32 QuantityToMerge = FMath::Min(RemainingQuantity, Item->GetAvailableStackSpace());
		Item->StackCount += QuantityToMerge;
		RemainingQuantity -= QuantityToMerge;
		if (RemainingQuantity <= 0)
		{
			break;
		}
	}

	const int32 MaxStackSize = GetDefinitionMaxStackSize(Definition);
	while (RemainingQuantity > 0)
	{
		const int32 StackQuantity = FMath::Min(RemainingQuantity, MaxStackSize);
		Items.Add(CreateStack(Definition, StackQuantity));
		RemainingQuantity -= StackQuantity;
	}

	return FPGXInventoryResult::Success(Definition, Quantity, Quantity, TEXT("Item quantity added."));
}

FPGXInventoryResult UPGXInventoryComponent::RemoveItem(const UPGXItemDefinition* Definition, int32 Quantity)
{
	if (!Definition)
	{
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::InvalidDefinition, Definition, Quantity, TEXT("Invalid item definition."));
	}
	if (Quantity <= 0)
	{
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::InvalidQuantity, Definition, Quantity, TEXT("Invalid item quantity."));
	}
	if (GetItemQuantity(Definition) < Quantity)
	{
		PGX_LOG_WARNING(LogPGX, TEXT("PGXInventory: RemoveItem insufficient quantity requested=%d available=%d"), Quantity, GetItemQuantity(Definition));
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::InsufficientQuantity, Definition, Quantity, TEXT("Insufficient item quantity."));
	}

	int32 RemainingQuantity = Quantity;
	for (int32 Index = Items.Num() - 1; Index >= 0 && RemainingQuantity > 0; --Index)
	{
		UPGXItemInstance* Item = Items[Index];
		if (!Item || Item->Definition != Definition)
		{
			continue;
		}

		const int32 QuantityToRemove = FMath::Min(RemainingQuantity, Item->StackCount);
		Item->StackCount -= QuantityToRemove;
		RemainingQuantity -= QuantityToRemove;
		if (Item->StackCount <= 0)
		{
			Items.RemoveAt(Index, 1, EAllowShrinking::No);
		}
	}

	return FPGXInventoryResult::Success(Definition, Quantity, Quantity, TEXT("Item quantity removed."));
}

FPGXInventoryResult UPGXInventoryComponent::TransferItemTo(UPGXInventoryComponent* Destination, const UPGXItemDefinition* Definition, int32 Quantity)
{
	if (!Destination)
	{
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::DestinationMissing, Definition, Quantity, TEXT("Destination inventory missing."));
	}
	if (!Definition)
	{
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::InvalidDefinition, Definition, Quantity, TEXT("Invalid item definition."));
	}
	if (Quantity <= 0)
	{
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::InvalidQuantity, Definition, Quantity, TEXT("Invalid item quantity."));
	}
	if (!HasItemQuantity(Definition, Quantity))
	{
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::InsufficientQuantity, Definition, Quantity, TEXT("Insufficient source item quantity."));
	}

	EPGXInventoryResultCode FailureCode = EPGXInventoryResultCode::InternalError;
	FString FailureMessage;
	if (!Destination->CanAcceptQuantity(Definition, Quantity, FailureCode, FailureMessage))
	{
		return FPGXInventoryResult::Failure(FailureCode, Definition, Quantity, FailureMessage);
	}

	const FPGXInventoryResult RemoveResult = RemoveItem(Definition, Quantity);
	if (!RemoveResult.bSuccess)
	{
		return RemoveResult;
	}

	const FPGXInventoryResult AddResult = Destination->AddItem(Definition, Quantity);
	if (!AddResult.bSuccess)
	{
		AddItem(Definition, Quantity);
		return FPGXInventoryResult::Failure(EPGXInventoryResultCode::InternalError, Definition, Quantity, TEXT("Transfer rollback executed after destination add failure."));
	}

	return FPGXInventoryResult::Success(Definition, Quantity, Quantity, TEXT("Item quantity transferred."));
}

int32 UPGXInventoryComponent::GetItemQuantity(const UPGXItemDefinition* Definition) const
{
	if (!Definition)
	{
		return 0;
	}

	int32 Quantity = 0;
	for (const UPGXItemInstance* Item : Items)
	{
		if (Item && Item->Definition == Definition)
		{
			Quantity += Item->StackCount;
		}
	}
	return Quantity;
}

bool UPGXInventoryComponent::HasItemQuantity(const UPGXItemDefinition* Definition, int32 Quantity) const
{
	return Quantity > 0 && GetItemQuantity(Definition) >= Quantity;
}

int32 UPGXInventoryComponent::GetUsedSlotCount() const
{
	return Items.Num();
}

float UPGXInventoryComponent::GetCurrentWeight() const
{
	float TotalWeight = 0.0f;
	for (const UPGXItemInstance* Item : Items)
	{
		TotalWeight += Item ? Item->GetStackWeight() : 0.0f;
	}
	return TotalWeight;
}

TArray<UPGXItemInstance*> UPGXInventoryComponent::GetItemsSnapshot() const
{
	TArray<UPGXItemInstance*> Snapshot;
	Snapshot.Reserve(Items.Num());
	for (UPGXItemInstance* Item : Items)
	{
		if (Item)
		{
			Snapshot.Add(Item);
		}
	}
	return Snapshot;
}

void UPGXInventoryComponent::ClearInventory()
{
	Items.Reset();
}

bool UPGXInventoryComponent::CanAcceptQuantity(const UPGXItemDefinition* Definition, int32 Quantity, EPGXInventoryResultCode& OutFailureCode, FString& OutFailureMessage) const
{
	if (!Definition)
	{
		OutFailureCode = EPGXInventoryResultCode::InvalidDefinition;
		OutFailureMessage = TEXT("Invalid item definition.");
		return false;
	}
	if (Quantity <= 0)
	{
		OutFailureCode = EPGXInventoryResultCode::InvalidQuantity;
		OutFailureMessage = TEXT("Invalid item quantity.");
		return false;
	}

	const float IncomingWeight = GetDefinitionUnitWeight(Definition) * static_cast<float>(Quantity);
	if (GetCurrentWeight() + IncomingWeight > MaxWeight)
	{
		OutFailureCode = EPGXInventoryResultCode::WeightCapacityExceeded;
		OutFailureMessage = TEXT("Inventory weight capacity exceeded.");
		return false;
	}

	const int32 ExistingStackSpace = GetAvailableStackSpace(Definition);
	const int32 RemainingAfterStackMerge = FMath::Max(0, Quantity - ExistingStackSpace);
	const int32 MaxStackSize = GetDefinitionMaxStackSize(Definition);
	const int32 RequiredNewSlots = FMath::DivideAndRoundUp(RemainingAfterStackMerge, MaxStackSize);
	if (RequiredNewSlots > GetFreeSlotCount())
	{
		OutFailureCode = EPGXInventoryResultCode::SlotCapacityExceeded;
		OutFailureMessage = TEXT("Inventory slot capacity exceeded.");
		return false;
	}

	OutFailureCode = EPGXInventoryResultCode::Success;
	OutFailureMessage.Reset();
	return true;
}

UPGXItemInstance* UPGXInventoryComponent::CreateStack(const UPGXItemDefinition* Definition, int32 Quantity)
{
	UPGXItemInstance* Item = NewObject<UPGXItemInstance>(this, UPGXItemInstance::StaticClass(), NAME_None, RF_Transient);
	Item->InitializeItem(Definition, Quantity);
	return Item;
}

int32 UPGXInventoryComponent::GetAvailableStackSpace(const UPGXItemDefinition* Definition) const
{
	int32 AvailableStackSpace = 0;
	for (const UPGXItemInstance* Item : Items)
	{
		if (Item && Item->CanStackWith(Definition))
		{
			AvailableStackSpace += Item->GetAvailableStackSpace();
		}
	}
	return AvailableStackSpace;
}

int32 UPGXInventoryComponent::GetFreeSlotCount() const
{
	return FMath::Max(0, MaxSlots - Items.Num());
}

int32 UPGXInventoryComponent::GetDefinitionMaxStackSize(const UPGXItemDefinition* Definition)
{
	return Definition ? FMath::Max(1, Definition->MaxStackSize) : 1;
}

float UPGXInventoryComponent::GetDefinitionUnitWeight(const UPGXItemDefinition* Definition)
{
	return Definition ? FMath::Max(0.0f, Definition->Weight) : 0.0f;
}
