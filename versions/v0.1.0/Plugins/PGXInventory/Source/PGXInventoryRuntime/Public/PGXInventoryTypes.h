// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Observability/PGXValidationResult.h"
#include "PGXInventoryTypes.generated.h"

class UPGXItemDefinition;

/** EN: Typed result codes for inventory operations / ES: Codigos tipados para operaciones de inventario */
UENUM(BlueprintType)
enum class EPGXInventoryResultCode : uint8
{
	Success = 0 UMETA(DisplayName = "Success"),
	InvalidDefinition = 1 UMETA(DisplayName = "Invalid Definition"),
	InvalidQuantity = 2 UMETA(DisplayName = "Invalid Quantity"),
	SourceMissing = 3 UMETA(DisplayName = "Source Missing"),
	DestinationMissing = 4 UMETA(DisplayName = "Destination Missing"),
	InsufficientQuantity = 5 UMETA(DisplayName = "Insufficient Quantity"),
	SlotCapacityExceeded = 6 UMETA(DisplayName = "Slot Capacity Exceeded"),
	WeightCapacityExceeded = 7 UMETA(DisplayName = "Weight Capacity Exceeded"),
	InternalError = 8 UMETA(DisplayName = "Internal Error")
};

/** EN: Detailed result for inventory mutations / ES: Resultado detallado para mutaciones de inventario */
USTRUCT(BlueprintType)
struct PGXINVENTORYRUNTIME_API FPGXInventoryResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Inventory")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Inventory")
	EPGXInventoryResultCode Code = EPGXInventoryResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Inventory")
	TObjectPtr<const UPGXItemDefinition> Definition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Inventory")
	int32 RequestedQuantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Inventory")
	int32 AffectedQuantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Inventory")
	FString Message;

	static FPGXInventoryResult Success(const UPGXItemDefinition* InDefinition, int32 InRequestedQuantity, int32 InAffectedQuantity, FString InMessage = FString());
	static FPGXInventoryResult Failure(EPGXInventoryResultCode InCode, const UPGXItemDefinition* InDefinition, int32 InRequestedQuantity, FString InMessage);

	/**
	 * EN: Convert to the canonical FPGXValidationResult for cross-cutting
	 *     concerns. Maps bSuccess -> bValid, EPGXInventoryResultCode -> FName.
	 *     Domain fields (Definition, RequestedQuantity, AffectedQuantity) are
	 *     NOT carried — they stay in this local struct.
	 *
	 *      Bridge to FPGXValidationResult.
	 *
	 * ES: Convertir al FPGXValidationResult canonico. Mapea bSuccess -> bValid,
	 *     EPGXInventoryResultCode -> FName. Los campos de dominio (Definition,
	 *     RequestedQuantity, AffectedQuantity) NO se llevan — se quedan en la
	 *     struct local.
	 */
	FPGXValidationResult ToValidationResult() const
	{
		if (bSuccess)
		{
			return FPGXValidationResult::MakeValid();
		}
		const FName CodeName(*UEnum::GetValueAsString(Code));
		FPGXValidationResult R;
		R.AddError(CodeName, FString(), FText::FromString(Message));
		return R;
	}
};
