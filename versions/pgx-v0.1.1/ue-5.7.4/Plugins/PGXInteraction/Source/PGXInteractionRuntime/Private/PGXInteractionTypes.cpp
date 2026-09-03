// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXInteractionTypes.h"

FPGXInteractionHandle FPGXInteractionHandle::NewHandle()
{
	FPGXInteractionHandle Handle;
	Handle.Id = FGuid::NewGuid();
	return Handle;
}

FPGXInteractionResult FPGXInteractionResult::Success(FPGXInteractionHandle InActionHandle, FPGXInteractionHandle InTargetHandle, EPGXInteractionActionState InState, FString InMessage)
{
	FPGXInteractionResult Result;
	Result.bSuccess = true;
	Result.Code = EPGXInteractionResultCode::Success;
	Result.State = InState;
	Result.ActionHandle = InActionHandle;
	Result.TargetHandle = InTargetHandle;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXInteractionResult FPGXInteractionResult::Failure(EPGXInteractionResultCode InCode, EPGXInteractionActionState InState, FString InMessage, FPGXInteractionHandle InActionHandle, FPGXInteractionHandle InTargetHandle)
{
	FPGXInteractionResult Result;
	Result.bSuccess = false;
	Result.Code = InCode;
	Result.State = InState;
	Result.ActionHandle = InActionHandle;
	Result.TargetHandle = InTargetHandle;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXInteractionQueryResult FPGXInteractionQueryResult::Success(FPGXInteractionPromptSnapshot InPromptSnapshot, FString InMessage)
{
	FPGXInteractionQueryResult Result;
	Result.bSuccess = true;
	Result.Code = EPGXInteractionResultCode::Success;
	Result.PromptSnapshot = MoveTemp(InPromptSnapshot);
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXInteractionQueryResult FPGXInteractionQueryResult::Failure(EPGXInteractionResultCode InCode, FString InMessage, FPGXInteractionHandle InTargetHandle)
{
	FPGXInteractionQueryResult Result;
	Result.bSuccess = false;
	Result.Code = InCode;
	Result.PromptSnapshot.TargetHandle = InTargetHandle;
	Result.Message = MoveTemp(InMessage);
	return Result;
}
