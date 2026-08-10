// Copyright PGX Framework. All Rights Reserved.

#include "PGXInputTypes.h"

FPGXInputContextResult FPGXInputContextResult::Success(FGameplayTag InContextTag, FString InMessage)
{
	FPGXInputContextResult Result;
	Result.bSuccess = true;
	Result.Code = EPGXInputContextResultCode::Success;
	Result.ContextTag = InContextTag;
	Result.Message = MoveTemp(InMessage);
	return Result;
}

FPGXInputContextResult FPGXInputContextResult::Failure(EPGXInputContextResultCode InCode, FGameplayTag InContextTag, FString InMessage)
{
	FPGXInputContextResult Result;
	Result.bSuccess = false;
	Result.Code = InCode;
	Result.ContextTag = InContextTag;
	Result.Message = MoveTemp(InMessage);
	return Result;
}
