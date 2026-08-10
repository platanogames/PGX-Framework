// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PGXVehiclesTypes.h"
#include "PGXVehiclesSubsystem.generated.h"

/**
 * EN: Baseline vehicles subsystem. Owns local vehicle registry and generic state transitions.
 * ES: Subsistema base de vehiculos. Posee registro local de vehiculos y transiciones genericas de estado.
 */
UCLASS()
class PGXVEHICLESRUNTIME_API UPGXVehiclesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	FPGXVehicleResult ValidateVehicleDefinition(const FPGXVehicleDefinition& Definition) const;
	FPGXVehicleResult RegisterVehicle(const FPGXVehicleRegistration& Registration);
	FPGXVehicleResult RegisterVehicleAsset(const UPGXVehicleDefinitionAsset* VehicleAsset, FGameplayTag SourceTag);
	FPGXVehicleResult UnregisterVehicle(FPGXVehicleHandle Handle, FString Message = FString());

	bool HasVehicle(FPGXVehicleHandle Handle) const;
	bool GetVehicleState(FPGXVehicleHandle Handle, FPGXVehicleState& OutState) const;
	int32 GetVehicleCount() const;
	TArray<FPGXVehicleState> GetVehicleStatesSnapshot() const;

	FPGXVehicleResult ClaimVehicle(FPGXVehicleHandle Handle, FString OwnerId, FGameplayTag SourceTag);
	FPGXVehicleResult ParkVehicle(FPGXVehicleHandle Handle, FGameplayTag SourceTag);
	FPGXVehicleResult RefuelVehicle(FPGXVehicleHandle Handle, float Amount, FGameplayTag SourceTag);
	FPGXVehicleResult RepairVehicle(FPGXVehicleHandle Handle, float Amount, FGameplayTag SourceTag);
	FPGXVehicleResult ApplyVehicleOperation(const FPGXVehicleOperationRequest& Request);

	int32 GetOperationRecordCount() const;
	TArray<FPGXVehicleOperationRecord> GetOperationRecordsSnapshot() const;
	void ClearOperationHistory();

#if WITH_DEV_AUTOMATION_TESTS
	void ClearVehiclesStateForTesting();
	void SetMaxVehiclesForTesting(int32 InMaxVehicles);
#endif

private:
	FPGXVehicleState* FindVehicleMutable(FPGXVehicleHandle Handle);
	const FPGXVehicleState* FindVehicle(FPGXVehicleHandle Handle) const;
	void RecordOperation(const FPGXVehicleOperationRequest& Request, EPGXVehicleOperationState State, EPGXVehicleResultCode ResultCode, FString Message);

	UPROPERTY(Transient)
	TArray<FPGXVehicleState> Vehicles;

	UPROPERTY(Transient)
	TArray<FPGXVehicleOperationRecord> OperationHistory;

	UPROPERTY(Transient)
	int32 MaxVehicles = 64;
};