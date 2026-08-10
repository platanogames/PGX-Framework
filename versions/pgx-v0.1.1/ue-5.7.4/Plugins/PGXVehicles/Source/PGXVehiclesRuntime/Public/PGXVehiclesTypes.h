// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Observability/PGXObservable.h"
#include "Observability/PGXValidationResult.h"
#include "PGXVehiclesTypes.generated.h"

/** EN: Typed outcome codes for vehicle operations / ES: Codigos tipados para operaciones de vehiculos */
UENUM(BlueprintType)
enum class EPGXVehicleResultCode : uint8
{
	Success = 0 UMETA(DisplayName = "Success"),
	InvalidDefinition = 1 UMETA(DisplayName = "Invalid Definition"),
	DuplicateVehicle = 2 UMETA(DisplayName = "Duplicate Vehicle"),
	VehicleNotFound = 3 UMETA(DisplayName = "Vehicle Not Found"),
	InvalidRequest = 4 UMETA(DisplayName = "Invalid Request"),
	InvalidState = 5 UMETA(DisplayName = "Invalid State"),
	CapacityExceeded = 6 UMETA(DisplayName = "Capacity Exceeded"),
	AlreadyResolved = 7 UMETA(DisplayName = "Already Resolved"),
	Unsupported = 8 UMETA(DisplayName = "Unsupported"),
	InternalError = 9 UMETA(DisplayName = "Internal Error")
};

/** EN: Baseline vehicle availability / ES: Disponibilidad base de vehiculo */
UENUM(BlueprintType)
enum class EPGXVehicleAvailability : uint8
{
	Unknown = 0 UMETA(DisplayName = "Unknown"),
	Available = 1 UMETA(DisplayName = "Available"),
	Claimed = 2 UMETA(DisplayName = "Claimed"),
	Parked = 3 UMETA(DisplayName = "Parked"),
	InRepair = 4 UMETA(DisplayName = "In Repair"),
	Unavailable = 5 UMETA(DisplayName = "Unavailable")
};

/** EN: Baseline vehicle operation lifecycle / ES: Ciclo de vida base de operacion de vehiculo */
UENUM(BlueprintType)
enum class EPGXVehicleOperationState : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Validated = 1 UMETA(DisplayName = "Validated"),
	Applied = 2 UMETA(DisplayName = "Applied"),
	Rejected = 3 UMETA(DisplayName = "Rejected")
};

/** EN: Stable opaque vehicle handle / ES: Handle opaco estable de vehiculo */
USTRUCT(BlueprintType)
struct PGXVEHICLESRUNTIME_API FPGXVehicleHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	FGuid Id;

	bool IsValid() const { return Id.IsValid(); }
	static FPGXVehicleHandle NewHandle();
};

/** EN: Runtime-safe vehicle definition snapshot / ES: Snapshot de definicion de vehiculo seguro para runtime */
USTRUCT(BlueprintType)
struct PGXVEHICLESRUNTIME_API FPGXVehicleDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles", meta = (Categories = "PGX.Vehicles.Definition"))
	FGameplayTag DefinitionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles", meta = (Categories = "PGX.Vehicles.Type"))
	FGameplayTag VehicleTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles", meta = (ClampMin = "0.0"))
	float MaxFuel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles", meta = (ClampMin = "0.0"))
	float MaxCondition = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles", meta = (ClampMin = "0"))
	int32 PassengerCapacity = 1;

	bool IsValid() const;
};

/** EN: DataAsset wrapper for authored vehicle definitions / ES: DataAsset contenedor para definiciones authoradas */
UCLASS(BlueprintType)
class PGXVEHICLESRUNTIME_API UPGXVehicleDefinitionAsset : public UDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	static const FName SchemaVersion;

	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Vehicles")
	FPGXVehicleDefinition Definition;
};

/** EN: Vehicle registration request / ES: Peticion de registro de vehiculo */
USTRUCT(BlueprintType)
struct PGXVEHICLESRUNTIME_API FPGXVehicleRegistration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles")
	FPGXVehicleDefinition Definition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles", meta = (Categories = "PGX.Vehicles.Source"))
	FGameplayTag SourceTag;

	bool IsValid() const { return Definition.IsValid(); }
};

/** EN: Runtime vehicle state snapshot / ES: Snapshot de estado runtime de vehiculo */
USTRUCT(BlueprintType)
struct PGXVEHICLESRUNTIME_API FPGXVehicleState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	FPGXVehicleHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	FPGXVehicleDefinition Definition;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	EPGXVehicleAvailability Availability = EPGXVehicleAvailability::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	float Fuel = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	float Condition = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	FString OwnerId;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	double RegisteredTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	double UpdatedTimeSeconds = 0.0;
};

/** EN: Generic vehicle operation request / ES: Peticion generica de operacion de vehiculo */
USTRUCT(BlueprintType)
struct PGXVEHICLESRUNTIME_API FPGXVehicleOperationRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles")
	FPGXVehicleHandle Handle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles", meta = (Categories = "PGX.Vehicles.Operation"))
	FGameplayTag OperationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles", meta = (Categories = "PGX.Vehicles.Source"))
	FGameplayTag SourceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles")
	FString ActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Vehicles")
	float Amount = 0.0f;

	bool IsValid() const { return Handle.IsValid() && OperationTag.IsValid(); }
};

/** EN: Vehicle operation record snapshot / ES: Snapshot de registro de operacion de vehiculo */
USTRUCT(BlueprintType)
struct PGXVEHICLESRUNTIME_API FPGXVehicleOperationRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	FPGXVehicleOperationRequest Request;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	EPGXVehicleOperationState State = EPGXVehicleOperationState::None;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	EPGXVehicleResultCode ResultCode = EPGXVehicleResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	double TimestampSeconds = 0.0;
};

/** EN: Typed vehicle operation result / ES: Resultado tipado de operacion de vehiculo */
USTRUCT(BlueprintType)
struct PGXVEHICLESRUNTIME_API FPGXVehicleResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	EPGXVehicleResultCode Code = EPGXVehicleResultCode::InternalError;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	EPGXVehicleOperationState OperationState = EPGXVehicleOperationState::None;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	FPGXVehicleHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles", meta = (Categories = "PGX.Vehicles.Definition"))
	FGameplayTag DefinitionTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Vehicles")
	FString Message;

	static FPGXVehicleResult Success(FPGXVehicleHandle InHandle, FGameplayTag InDefinitionTag, EPGXVehicleOperationState InOperationState, FString InMessage = FString());
	static FPGXVehicleResult Failure(EPGXVehicleResultCode InCode, EPGXVehicleOperationState InOperationState, FString InMessage, FPGXVehicleHandle InHandle = FPGXVehicleHandle(), FGameplayTag InDefinitionTag = FGameplayTag());

	/**
	 * EN: Convert to the canonical FPGXValidationResult for cross-cutting
	 *     concerns. Maps bSuccess -> bValid, EPGXVehicleResultCode -> FName.
	 *     The DefinitionTag is carried as the Path field (useful for "which
	 *     vehicle archetype failed" diagnostics). Domain fields (Handle,
	 *     OperationState) are NOT carried — they stay in this local struct.
	 *
	 *      Bridge to FPGXValidationResult.
	 *
	 * ES: Convertir al FPGXValidationResult canonico. Mapea bSuccess -> bValid,
	 *     EPGXVehicleResultCode -> FName. El DefinitionTag se lleva como Path
	 *     (util para "que arquetipo de vehiculo fallo"). Los campos de dominio
	 *     (Handle, OperationState) NO se llevan — se quedan en la struct local.
	 */
	FPGXValidationResult ToValidationResult() const
	{
		if (bSuccess)
		{
			return FPGXValidationResult::MakeValid();
		}
		const FName CodeName(*UEnum::GetValueAsString(Code));
		FPGXValidationResult R;
		R.AddError(CodeName, DefinitionTag.ToString(), FText::FromString(Message));
		return R;
	}
};