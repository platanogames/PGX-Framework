// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXVehiclesTypes.h"
#include "PGXVehiclesObservability.h"

FPGXVehicleHandle FPGXVehicleHandle::NewHandle()
{
	FPGXVehicleHandle Handle;
	Handle.Id = FGuid::NewGuid();
	return Handle;
}

bool FPGXVehicleDefinition::IsValid() const
{
	return DefinitionTag.IsValid()
		&& VehicleTypeTag.IsValid()
		&& MaxFuel >= 0.0f
		&& MaxCondition > 0.0f
		&& PassengerCapacity >= 0;
}

FPGXVehicleResult FPGXVehicleResult::Success(FPGXVehicleHandle InHandle, FGameplayTag InDefinitionTag, EPGXVehicleOperationState InOperationState, FString InMessage)
{
	FPGXVehicleResult Result;
	Result.bSuccess = true;
	Result.Code = EPGXVehicleResultCode::Success;
	Result.OperationState = InOperationState;
	Result.Handle = InHandle;
	Result.DefinitionTag = InDefinitionTag;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXVehicleResult FPGXVehicleResult::Failure(EPGXVehicleResultCode InCode, EPGXVehicleOperationState InOperationState, FString InMessage, FPGXVehicleHandle InHandle, FGameplayTag InDefinitionTag)
{
	FPGXVehicleResult Result;
	Result.bSuccess = false;
	Result.Code = InCode;
	Result.OperationState = InOperationState;
	Result.Handle = InHandle;
	Result.DefinitionTag = InDefinitionTag;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

const FName UPGXVehicleDefinitionAsset::SchemaVersion(TEXT("1.0"));

FPGXJsonValue UPGXVehicleDefinitionAsset::ToJson() const
{
	FPGXJsonValue Out;
	Out.JsonString = FString::Printf(
		TEXT("{\"schema\":{\"type\":\"%s\",\"version\":\"%s\",\"plugin\":\"%s\"},\"data\":{\"DefinitionTag\":\"%s\",\"VehicleTypeTag\":\"%s\",\"DisplayName\":\"%s\",\"MaxFuel\":%.6f,\"MaxCondition\":%.6f,\"PassengerCapacity\":%d}}"),
		*GetClass()->GetName(),
		*GetSchemaVersion().ToString(),
		*PGXVehiclesObservability::GetOwningPluginName(this, TEXT("PGXVehiclesRuntime")).ToString(),
		*PGXVehiclesObservability::EscapeJsonString(Definition.DefinitionTag.ToString()),
		*PGXVehiclesObservability::EscapeJsonString(Definition.VehicleTypeTag.ToString()),
		*PGXVehiclesObservability::EscapeJsonString(Definition.DisplayName.ToString()),
		Definition.MaxFuel,
		Definition.MaxCondition,
		Definition.PassengerCapacity);
	return Out;
}

FPGXValidationResult UPGXVehicleDefinitionAsset::FromJson(const FPGXJsonValue& Json)
{
	return PGXVehiclesObservability::ValidateJsonEnvelope(
		Json,
		NSLOCTEXT("PGXVehicles", "ObservableEmptyPayload", "UPGXVehicleDefinitionAsset FromJson received an empty payload."));
}

FName UPGXVehicleDefinitionAsset::GetSchemaVersion() const
{
	return SchemaVersion;
}

FPGXSchemaDescriptor UPGXVehicleDefinitionAsset::GetSchemaDescriptor() const
{
	return PGXVehiclesObservability::MakeSchemaDescriptor(this, GetSchemaVersion(), TEXT("PGXVehiclesRuntime"));
}
