// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PGXItemInstance.generated.h"

class UPGXItemDefinition;

/**
 * EN: Runtime instance of an item.
 *     Holds per-instance state and stack count.
 *
 * ES: Instancia runtime de un item.
 *     Mantiene estado por instancia y cantidad en stack.
 */
UCLASS(BlueprintType)
class PGXINVENTORYRUNTIME_API UPGXItemInstance : public UObject
{
	GENERATED_BODY()

public:
	/** EN: Reference to the item definition this instance is based on / ES: Referencia a la definicion de item en la que se basa esta instancia */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Inventory")
	TObjectPtr<const UPGXItemDefinition> Definition;

	/** EN: Current stack count for this item instance / ES: Cantidad actual en stack para esta instancia de item */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Inventory")
	int32 StackCount = 1;

	UFUNCTION(BlueprintCallable, Category = "PGX|Inventory")
	void InitializeItem(const UPGXItemDefinition* InDefinition, int32 InStackCount);

	UFUNCTION(BlueprintPure, Category = "PGX|Inventory")
	bool CanStackWith(const UPGXItemDefinition* OtherDefinition) const;

	UFUNCTION(BlueprintPure, Category = "PGX|Inventory")
	int32 GetAvailableStackSpace() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Inventory")
	float GetStackWeight() const;
};
