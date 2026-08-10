// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PGXInventoryTypes.h"
#include "PGXInventoryComponent.generated.h"

class UPGXItemDefinition;
class UPGXItemInstance;

/**
 * EN: Inventory component with slots, weight, and stack policy enforcement.
 *     Attach to any actor that needs generic item containment.
 *
 * ES: Componente de inventario con enforcement de slots, peso y politica de stack.
 *     Adjuntar a cualquier actor que necesite contencion generica de items.
 */
UCLASS(ClassGroup=(PGX), meta=(BlueprintSpawnableComponent))
class PGXINVENTORYRUNTIME_API UPGXInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPGXInventoryComponent();

	/** EN: Maximum number of inventory slots / ES: Numero maximo de slots de inventario */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Inventory", meta = (ClampMin = "0"))
	int32 MaxSlots = 20;

	/** EN: Maximum total weight the inventory can hold / ES: Peso total maximo que el inventario puede contener */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Inventory", meta = (ClampMin = "0.0"))
	float MaxWeight = 100.0f;

	UFUNCTION(BlueprintCallable, Category = "PGX|Inventory")
	FPGXInventoryResult AddItem(const UPGXItemDefinition* Definition, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "PGX|Inventory")
	FPGXInventoryResult RemoveItem(const UPGXItemDefinition* Definition, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "PGX|Inventory")
	FPGXInventoryResult TransferItemTo(UPGXInventoryComponent* Destination, const UPGXItemDefinition* Definition, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "PGX|Inventory")
	int32 GetItemQuantity(const UPGXItemDefinition* Definition) const;

	UFUNCTION(BlueprintPure, Category = "PGX|Inventory")
	bool HasItemQuantity(const UPGXItemDefinition* Definition, int32 Quantity) const;

	UFUNCTION(BlueprintPure, Category = "PGX|Inventory")
	int32 GetUsedSlotCount() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "PGX|Inventory")
	TArray<UPGXItemInstance*> GetItemsSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "PGX|Inventory")
	void ClearInventory();

private:
	bool CanAcceptQuantity(const UPGXItemDefinition* Definition, int32 Quantity, EPGXInventoryResultCode& OutFailureCode, FString& OutFailureMessage) const;
	UPGXItemInstance* CreateStack(const UPGXItemDefinition* Definition, int32 Quantity);
	int32 GetAvailableStackSpace(const UPGXItemDefinition* Definition) const;
	int32 GetFreeSlotCount() const;
	static int32 GetDefinitionMaxStackSize(const UPGXItemDefinition* Definition);
	static float GetDefinitionUnitWeight(const UPGXItemDefinition* Definition);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "PGX|Inventory", Transient, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UPGXItemInstance>> Items;
};
