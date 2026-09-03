// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSpawnTypes.h"

FPGXSpawnRequestHandle FPGXSpawnRequestHandle::NewHandle()
{
	FPGXSpawnRequestHandle Handle;
	Handle.Id = FGuid::NewGuid();
	return Handle;
}

FPGXSpawnResult FPGXSpawnResult::Success(FPGXSpawnRequestHandle InHandle, EPGXSpawnRequestStatus InStatus, AActor* InSpawnedActor, FString InMessage)
{
	FPGXSpawnResult Result;
	Result.bSuccess = true;
	Result.Code = EPGXSpawnResultCode::Success;
	Result.Status = InStatus;
	Result.Handle = InHandle;
	Result.SpawnedActor = InSpawnedActor;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXSpawnResult FPGXSpawnResult::Failure(EPGXSpawnResultCode InCode, EPGXSpawnRequestStatus InStatus, FString InMessage, FPGXSpawnRequestHandle InHandle)
{
	FPGXSpawnResult Result;
	Result.bSuccess = false;
	Result.Code = InCode;
	Result.Status = InStatus;
	Result.Handle = InHandle;
	Result.Message = MoveTemp(InMessage);
	return Result;
}
