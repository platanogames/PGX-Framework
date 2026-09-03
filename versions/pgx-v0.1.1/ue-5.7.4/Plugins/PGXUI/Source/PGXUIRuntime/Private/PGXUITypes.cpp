// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXUITypes.h"

FPGXUIHandle FPGXUIHandle::NewHandle()
{
	FPGXUIHandle Handle;
	Handle.Id = FGuid::NewGuid();
	return Handle;
}

FPGXUIResult FPGXUIResult::Success(FPGXUIHandle InHandle, FString InMessage)
{
	FPGXUIResult Result;
	Result.bSuccess = true;
	Result.Code = EPGXUIResultCode::Success;
	Result.Handle = InHandle;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXUIResult FPGXUIResult::Failure(EPGXUIResultCode InCode, FString InMessage, FPGXUIHandle InHandle)
{
	FPGXUIResult Result;
	Result.bSuccess = false;
	Result.Code = InCode;
	Result.Handle = InHandle;
	Result.Message = MoveTemp(InMessage);
	return Result;
}