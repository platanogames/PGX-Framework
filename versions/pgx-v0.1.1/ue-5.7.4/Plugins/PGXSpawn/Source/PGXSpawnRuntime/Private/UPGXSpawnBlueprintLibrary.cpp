// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// Blueprint facade — BlueprintLibrary implementation.

#include "UPGXSpawnBlueprintLibrary.h"

#include "PGXSpawnSubsystem.h"
#include "Engine/World.h"

UPGXSpawnSubsystem* UPGXSpawnBlueprintLibrary::GetSpawnSubsystem(const UObject* WorldContext)
{
	if (!IsValid(WorldContext))
	{
		return nullptr;
	}
	UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return nullptr;
	}
	return World->GetSubsystem<UPGXSpawnSubsystem>();
}

bool UPGXSpawnBlueprintLibrary::IsValidSpawnRequest(const FPGXSpawnRequest& Request)
{
	UClass* SpawnClass = Request.SpawnClass.Get();
	if (!SpawnClass)
	{
		return false;
	}
	if (!SpawnClass->IsChildOf(AActor::StaticClass()) || SpawnClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
	{
		return false;
	}
	if (Request.Transform.ContainsNaN() || Request.Transform.GetScale3D().IsNearlyZero())
	{
		return false;
	}
	return true;
}

FPGXSpawnRequest UPGXSpawnBlueprintLibrary::MakeSpawnRequest(TSubclassOf<AActor> Class, FTransform Transform, FGameplayTag SourceTag, int32 Priority)
{
	FPGXSpawnRequest Request;
	Request.SpawnClass = Class;
	Request.Transform = Transform;
	Request.SourceTag = SourceTag;
	Request.Priority = Priority;
	return Request;
}

FPGXSpawnClassEntry UPGXSpawnBlueprintLibrary::MakeSpawnClassEntry(TSubclassOf<AActor> Class, float Weight, int32 MinCount, int32 MaxCount)
{
	FPGXSpawnClassEntry Entry;
	Entry.Class = Class;
	Entry.Weight = FMath::Max(Weight, 0.0f);
	Entry.MinCount = FMath::Max(MinCount, 0);
	Entry.MaxCount = FMath::Max(MaxCount, 1);
	// Ensure MinCount <= MaxCount
	if (Entry.MinCount > Entry.MaxCount)
	{
		Entry.MinCount = Entry.MaxCount;
	}
	return Entry;
}

FString UPGXSpawnBlueprintLibrary::ResultCodeToString(EPGXSpawnResultCode Code)
{
	switch (Code)
	{
		case EPGXSpawnResultCode::Success:                return TEXT("Success");
		case EPGXSpawnResultCode::InvalidRequest:         return TEXT("Invalid Request");
		case EPGXSpawnResultCode::InvalidSpawnClass:     return TEXT("Invalid Spawn Class");
		case EPGXSpawnResultCode::InvalidTransform:       return TEXT("Invalid Transform");
		case EPGXSpawnResultCode::BudgetExceeded:         return TEXT("Budget Exceeded");
		case EPGXSpawnResultCode::RecordNotFound:         return TEXT("Record Not Found");
		case EPGXSpawnResultCode::AlreadyCompleted:       return TEXT("Already Completed");
		case EPGXSpawnResultCode::InternalError:          return TEXT("Internal Error");
		case EPGXSpawnResultCode::InvalidWorld:           return TEXT("Invalid World");
		case EPGXSpawnResultCode::SpawnActorFailed:      return TEXT("Spawn Actor Failed");
		default:                                          return TEXT("Unknown");
	}
}

FString UPGXSpawnBlueprintLibrary::RequestStatusToString(EPGXSpawnRequestStatus Status)
{
	switch (Status)
	{
		case EPGXSpawnRequestStatus::None:        return TEXT("None");
		case EPGXSpawnRequestStatus::Queued:      return TEXT("Queued");
		case EPGXSpawnRequestStatus::Running:     return TEXT("Running");
		case EPGXSpawnRequestStatus::Completed:   return TEXT("Completed");
		case EPGXSpawnRequestStatus::Failed:      return TEXT("Failed");
		case EPGXSpawnRequestStatus::Cancelled:   return TEXT("Cancelled");
		case EPGXSpawnRequestStatus::Expired:     return TEXT("Expired");
		default:                                  return TEXT("Unknown");
	}
}

TArray<FPGXSpawnRecord> UPGXSpawnBlueprintLibrary::FilterRecordsByStatus(const TArray<FPGXSpawnRecord>& Records, EPGXSpawnRequestStatus Status)
{
	TArray<FPGXSpawnRecord> Result;
	Result.Reserve(Records.Num());
	for (const FPGXSpawnRecord& Record : Records)
	{
		if (Record.Status == Status)
		{
			Result.Add(Record);
		}
	}
	return Result;
}
