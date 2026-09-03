// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXInventoryTypes.h"

FPGXInventoryResult FPGXInventoryResult::Success(const UPGXItemDefinition* InDefinition, int32 InRequestedQuantity, int32 InAffectedQuantity, FString InMessage)
{
	FPGXInventoryResult Result;
	Result.bSuccess = true;
	Result.Code = EPGXInventoryResultCode::Success;
	Result.Definition = InDefinition;
	Result.RequestedQuantity = InRequestedQuantity;
	Result.AffectedQuantity = InAffectedQuantity;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXInventoryResult FPGXInventoryResult::Failure(EPGXInventoryResultCode InCode, const UPGXItemDefinition* InDefinition, int32 InRequestedQuantity, FString InMessage)
{
	FPGXInventoryResult Result;
	Result.bSuccess = false;
	Result.Code = InCode;
	Result.Definition = InDefinition;
	Result.RequestedQuantity = InRequestedQuantity;
	Result.AffectedQuantity = 0;
	Result.Message = MoveTemp(InMessage);
	return Result;
}
