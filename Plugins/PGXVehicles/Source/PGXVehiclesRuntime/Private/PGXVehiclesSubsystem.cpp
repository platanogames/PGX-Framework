// Copyright PGX Framework. All Rights Reserved.

#include "PGXVehiclesSubsystem.h"

#include "Tags/PGXVehiclesTags.h"
#include "HAL/PlatformTime.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXVehiclesSubsystem, Log, All);

void UPGXVehiclesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Vehicles.Reset();
	OperationHistory.Reset();
}

void UPGXVehiclesSubsystem::Deinitialize()
{
	Vehicles.Reset();
	OperationHistory.Reset();
	Super::Deinitialize();
}

FPGXVehicleResult UPGXVehiclesSubsystem::ValidateVehicleDefinition(const FPGXVehicleDefinition& Definition) const
{
	if (!Definition.DefinitionTag.IsValid())
	{
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidDefinition, EPGXVehicleOperationState::Rejected, TEXT("Vehicle definition tag is invalid."));
	}
	if (!Definition.VehicleTypeTag.IsValid())
	{
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidDefinition, EPGXVehicleOperationState::Rejected, TEXT("Vehicle type tag is invalid."), FPGXVehicleHandle(), Definition.DefinitionTag);
	}
	if (Definition.MaxFuel < 0.0f || Definition.MaxCondition <= 0.0f || Definition.PassengerCapacity < 0)
	{
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidDefinition, EPGXVehicleOperationState::Rejected, TEXT("Vehicle numeric limits are invalid."), FPGXVehicleHandle(), Definition.DefinitionTag);
	}
	return FPGXVehicleResult::Success(FPGXVehicleHandle(), Definition.DefinitionTag, EPGXVehicleOperationState::Validated, TEXT("Vehicle definition valid."));
}

FPGXVehicleResult UPGXVehiclesSubsystem::RegisterVehicle(const FPGXVehicleRegistration& Registration)
{
	const FPGXVehicleResult ValidationResult = ValidateVehicleDefinition(Registration.Definition);
	if (!ValidationResult.bSuccess)
	{
		PGX_LOG_WARNING(LogPGXVehiclesSubsystem, TEXT("PGXVehicles: invalid vehicle registration rejected"));
		return ValidationResult;
	}

	if (Vehicles.Num() >= MaxVehicles)
	{
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::CapacityExceeded, EPGXVehicleOperationState::Rejected, TEXT("Vehicle registry capacity exhausted."), FPGXVehicleHandle(), Registration.Definition.DefinitionTag);
	}


	FPGXVehicleState State;
	State.Handle = FPGXVehicleHandle::NewHandle();
	State.Definition = Registration.Definition;
	State.Availability = EPGXVehicleAvailability::Available;
	State.Fuel = Registration.Definition.MaxFuel;
	State.Condition = Registration.Definition.MaxCondition;
	State.RegisteredTimeSeconds = FPlatformTime::Seconds();
	State.UpdatedTimeSeconds = State.RegisteredTimeSeconds;
	Vehicles.Add(State);

	FPGXVehicleOperationRequest Request;
	Request.Handle = State.Handle;
	Request.OperationTag = TAG_PGX_Vehicles_Operation_Register.GetTag();
	Request.SourceTag = Registration.SourceTag;
	RecordOperation(Request, EPGXVehicleOperationState::Applied, EPGXVehicleResultCode::Success, TEXT("Vehicle registered."));

	return FPGXVehicleResult::Success(State.Handle, State.Definition.DefinitionTag, EPGXVehicleOperationState::Applied, TEXT("Vehicle registered."));
}

FPGXVehicleResult UPGXVehiclesSubsystem::RegisterVehicleAsset(const UPGXVehicleDefinitionAsset* VehicleAsset, FGameplayTag SourceTag)
{
	if (!IsValid(VehicleAsset))
	{
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidDefinition, EPGXVehicleOperationState::Rejected, TEXT("Vehicle asset is invalid."));
	}
	FPGXVehicleRegistration Registration;
	Registration.Definition = VehicleAsset->Definition;
	Registration.SourceTag = SourceTag;
	return RegisterVehicle(Registration);
}

FPGXVehicleResult UPGXVehiclesSubsystem::UnregisterVehicle(FPGXVehicleHandle Handle, FString Message)
{
	if (!Handle.IsValid())
	{
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidRequest, EPGXVehicleOperationState::Rejected, TEXT("Vehicle handle is invalid."), Handle);
	}

	const int32 RemovedCount = Vehicles.RemoveAll([Handle](const FPGXVehicleState& State)
	{
		return State.Handle.Id == Handle.Id;
	});
	if (RemovedCount <= 0)
	{
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::VehicleNotFound, EPGXVehicleOperationState::Rejected, TEXT("Vehicle was not found."), Handle);
	}

	return FPGXVehicleResult::Success(Handle, FGameplayTag(), EPGXVehicleOperationState::Applied, Message.IsEmpty() ? TEXT("Vehicle unregistered.") : MoveTemp(Message));
}

bool UPGXVehiclesSubsystem::HasVehicle(FPGXVehicleHandle Handle) const
{
	return FindVehicle(Handle) != nullptr;
}

bool UPGXVehiclesSubsystem::GetVehicleState(FPGXVehicleHandle Handle, FPGXVehicleState& OutState) const
{
	const FPGXVehicleState* State = FindVehicle(Handle);
	if (!State)
	{
		return false;
	}
	OutState = *State;
	return true;
}

int32 UPGXVehiclesSubsystem::GetVehicleCount() const
{
	return Vehicles.Num();
}

TArray<FPGXVehicleState> UPGXVehiclesSubsystem::GetVehicleStatesSnapshot() const
{
	TArray<FPGXVehicleState> Snapshot = Vehicles;
	Snapshot.Sort([](const FPGXVehicleState& Left, const FPGXVehicleState& Right)
	{
		return Left.RegisteredTimeSeconds < Right.RegisteredTimeSeconds;
	});
	return Snapshot;
}

FPGXVehicleResult UPGXVehiclesSubsystem::ClaimVehicle(FPGXVehicleHandle Handle, FString OwnerId, FGameplayTag SourceTag)
{
	if (OwnerId.IsEmpty())
	{
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidRequest, EPGXVehicleOperationState::Rejected, TEXT("Vehicle owner id is empty."), Handle);
	}
	FPGXVehicleOperationRequest Request;
	Request.Handle = Handle;
	Request.OperationTag = TAG_PGX_Vehicles_Operation_Claim.GetTag();
	Request.SourceTag = SourceTag;
	Request.ActorId = MoveTemp(OwnerId);
	return ApplyVehicleOperation(Request);
}

FPGXVehicleResult UPGXVehiclesSubsystem::ParkVehicle(FPGXVehicleHandle Handle, FGameplayTag SourceTag)
{
	FPGXVehicleOperationRequest Request;
	Request.Handle = Handle;
	Request.OperationTag = TAG_PGX_Vehicles_Operation_Park.GetTag();
	Request.SourceTag = SourceTag;
	return ApplyVehicleOperation(Request);
}

FPGXVehicleResult UPGXVehiclesSubsystem::RefuelVehicle(FPGXVehicleHandle Handle, float Amount, FGameplayTag SourceTag)
{
	FPGXVehicleOperationRequest Request;
	Request.Handle = Handle;
	Request.OperationTag = TAG_PGX_Vehicles_Operation_Refuel.GetTag();
	Request.SourceTag = SourceTag;
	Request.Amount = Amount;
	return ApplyVehicleOperation(Request);
}

FPGXVehicleResult UPGXVehiclesSubsystem::RepairVehicle(FPGXVehicleHandle Handle, float Amount, FGameplayTag SourceTag)
{
	FPGXVehicleOperationRequest Request;
	Request.Handle = Handle;
	Request.OperationTag = TAG_PGX_Vehicles_Operation_Repair.GetTag();
	Request.SourceTag = SourceTag;
	Request.Amount = Amount;
	return ApplyVehicleOperation(Request);
}

FPGXVehicleResult UPGXVehiclesSubsystem::ApplyVehicleOperation(const FPGXVehicleOperationRequest& Request)
{
	if (!Request.IsValid())
	{
		RecordOperation(Request, EPGXVehicleOperationState::Rejected, EPGXVehicleResultCode::InvalidRequest, TEXT("Vehicle operation request is invalid."));
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidRequest, EPGXVehicleOperationState::Rejected, TEXT("Vehicle operation request is invalid."), Request.Handle);
	}

	FPGXVehicleState* State = FindVehicleMutable(Request.Handle);
	if (!State)
	{
		RecordOperation(Request, EPGXVehicleOperationState::Rejected, EPGXVehicleResultCode::VehicleNotFound, TEXT("Vehicle was not found."));
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::VehicleNotFound, EPGXVehicleOperationState::Rejected, TEXT("Vehicle was not found."), Request.Handle);
	}

	FString Message;
	if (Request.OperationTag == TAG_PGX_Vehicles_Operation_Claim.GetTag())
	{
		if (Request.ActorId.IsEmpty())
		{
			RecordOperation(Request, EPGXVehicleOperationState::Rejected, EPGXVehicleResultCode::InvalidRequest, TEXT("Claim requires an owner id."));
			return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidRequest, EPGXVehicleOperationState::Rejected, TEXT("Claim requires an owner id."), Request.Handle, State->Definition.DefinitionTag);
		}
		State->Availability = EPGXVehicleAvailability::Claimed;
		State->OwnerId = Request.ActorId;
		Message = TEXT("Vehicle claimed.");
	}
	else if (Request.OperationTag == TAG_PGX_Vehicles_Operation_Park.GetTag())
	{
		State->Availability = EPGXVehicleAvailability::Parked;
		Message = TEXT("Vehicle parked.");
	}
	else if (Request.OperationTag == TAG_PGX_Vehicles_Operation_Refuel.GetTag())
	{
		if (Request.Amount <= 0.0f)
		{
			RecordOperation(Request, EPGXVehicleOperationState::Rejected, EPGXVehicleResultCode::InvalidRequest, TEXT("Refuel amount must be positive."));
			return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidRequest, EPGXVehicleOperationState::Rejected, TEXT("Refuel amount must be positive."), Request.Handle, State->Definition.DefinitionTag);
		}
		State->Fuel = FMath::Clamp(State->Fuel + Request.Amount, 0.0f, State->Definition.MaxFuel);
		Message = TEXT("Vehicle refueled.");
	}
	else if (Request.OperationTag == TAG_PGX_Vehicles_Operation_Repair.GetTag())
	{
		if (Request.Amount <= 0.0f)
		{
			RecordOperation(Request, EPGXVehicleOperationState::Rejected, EPGXVehicleResultCode::InvalidRequest, TEXT("Repair amount must be positive."));
			return FPGXVehicleResult::Failure(EPGXVehicleResultCode::InvalidRequest, EPGXVehicleOperationState::Rejected, TEXT("Repair amount must be positive."), Request.Handle, State->Definition.DefinitionTag);
		}
		State->Condition = FMath::Clamp(State->Condition + Request.Amount, 0.0f, State->Definition.MaxCondition);
		if (State->Availability == EPGXVehicleAvailability::Unavailable)
		{
			State->Availability = EPGXVehicleAvailability::InRepair;
		}
		Message = TEXT("Vehicle repaired.");
	}
	else
	{
		RecordOperation(Request, EPGXVehicleOperationState::Rejected, EPGXVehicleResultCode::Unsupported, TEXT("Vehicle operation is unsupported by baseline."));
		return FPGXVehicleResult::Failure(EPGXVehicleResultCode::Unsupported, EPGXVehicleOperationState::Rejected, TEXT("Vehicle operation is unsupported by baseline."), Request.Handle, State->Definition.DefinitionTag);
	}

	State->UpdatedTimeSeconds = FPlatformTime::Seconds();
	RecordOperation(Request, EPGXVehicleOperationState::Applied, EPGXVehicleResultCode::Success, Message);
	return FPGXVehicleResult::Success(Request.Handle, State->Definition.DefinitionTag, EPGXVehicleOperationState::Applied, Message);
}

int32 UPGXVehiclesSubsystem::GetOperationRecordCount() const
{
	return OperationHistory.Num();
}

TArray<FPGXVehicleOperationRecord> UPGXVehiclesSubsystem::GetOperationRecordsSnapshot() const
{
	return OperationHistory;
}

void UPGXVehiclesSubsystem::ClearOperationHistory()
{
	OperationHistory.Reset();
}

#if WITH_DEV_AUTOMATION_TESTS
void UPGXVehiclesSubsystem::ClearVehiclesStateForTesting()
{
	Vehicles.Reset();
	OperationHistory.Reset();
	MaxVehicles = 64;
}

void UPGXVehiclesSubsystem::SetMaxVehiclesForTesting(int32 InMaxVehicles)
{
	MaxVehicles = FMath::Max(0, InMaxVehicles);
}
#endif

FPGXVehicleState* UPGXVehiclesSubsystem::FindVehicleMutable(FPGXVehicleHandle Handle)
{
	return Vehicles.FindByPredicate([Handle](const FPGXVehicleState& State)
	{
		return Handle.IsValid() && State.Handle.Id == Handle.Id;
	});
}

const FPGXVehicleState* UPGXVehiclesSubsystem::FindVehicle(FPGXVehicleHandle Handle) const
{
	return Vehicles.FindByPredicate([Handle](const FPGXVehicleState& State)
	{
		return Handle.IsValid() && State.Handle.Id == Handle.Id;
	});
}

void UPGXVehiclesSubsystem::RecordOperation(const FPGXVehicleOperationRequest& Request, EPGXVehicleOperationState State, EPGXVehicleResultCode ResultCode, FString Message)
{
	FPGXVehicleOperationRecord Record;
	Record.Request = Request;
	Record.State = State;
	Record.ResultCode = ResultCode;
	Record.Message = MoveTemp(Message);
	Record.TimestampSeconds = FPlatformTime::Seconds();
	OperationHistory.Add(Record);
}
