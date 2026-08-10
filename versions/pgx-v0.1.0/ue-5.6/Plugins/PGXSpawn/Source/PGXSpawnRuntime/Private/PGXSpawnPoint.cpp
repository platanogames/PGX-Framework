// Copyright PGX Framework. All Rights Reserved.

#include "PGXSpawnPoint.h"

APGXSpawnPoint::APGXSpawnPoint()
{
}

FPGXSpawnRequest APGXSpawnPoint::BuildSpawnRequest() const
{
	FPGXSpawnRequest Request;
	Request.SpawnClass = SpawnClass;
	Request.Transform = GetActorTransform();
	Request.SourceTag = SpawnPointTag;
	return Request;
}

FPGXSpawnResult APGXSpawnPoint::ValidateSpawnPointPolicy() const
{
	if (!SpawnClass)
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidSpawnClass, EPGXSpawnRequestStatus::Failed, TEXT("Spawn point has no spawn class."));
	}
	if (RespawnCooldown < 0.0f || MaxActiveSpawns <= 0)
	{
		return FPGXSpawnResult::Failure(EPGXSpawnResultCode::InvalidRequest, EPGXSpawnRequestStatus::Failed, TEXT("Spawn point policy is invalid."));
	}
	return FPGXSpawnResult::Success(FPGXSpawnRequestHandle(), EPGXSpawnRequestStatus::Queued, nullptr, TEXT("Spawn point policy is valid."));
}
