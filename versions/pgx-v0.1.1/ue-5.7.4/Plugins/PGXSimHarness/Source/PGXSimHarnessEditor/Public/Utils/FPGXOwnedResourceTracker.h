// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"

//
/**
 * [ES] Rastreador liviano de recursos creados por harness/tests. Existe para que
 *      el cleanup nunca recorra ni destruya recursos ajenos al harness.
 * [EN] Lightweight tracker for harness/test-owned resources. Exists so cleanup
 *      never scans or destroys resources owned by gameplay/runtime systems.
 *
 * Depende de / Depends on: CoreMinimal, GameplayTagContainer, AActor weak refs.
 * Usado por / Used by: PGXSimHarness editor utilities and batch-oriented harness utilities.
 */
struct FPGXOwnedResourceTracker
{
	struct FOwnedActor
	{
		TWeakObjectPtr<AActor> Actor;
		FGameplayTag OwnerTag;
	};

	struct FOwnedHandle
	{
		FGuid Handle;
		FGameplayTag OwnerTag;
		FName SystemName;
	};

	void TrackActor(AActor* Actor, FGameplayTag OwnerTag)
	{
		if (IsValid(Actor))
		{
			OwnedActors.Add({ Actor, OwnerTag });
		}
	}

	void TrackHandle(FGuid Handle, FGameplayTag OwnerTag, FName SystemName)
	{
		if (Handle.IsValid())
		{
			OwnedHandles.Add({ Handle, OwnerTag, SystemName });
		}
	}

	TArray<FGuid> GetHandlesForTag(FGameplayTag OwnerTag, FName SystemName = NAME_None) const
	{
		TArray<FGuid> Result;
		for (const FOwnedHandle& Entry : OwnedHandles)
		{
			if (Entry.OwnerTag == OwnerTag && (SystemName.IsNone() || Entry.SystemName == SystemName))
			{
				Result.Add(Entry.Handle);
			}
		}
		return Result;
	}

	int32 DestroyActorsForTag(FGameplayTag OwnerTag)
	{
		int32 Destroyed = 0;
		for (FOwnedActor& Entry : OwnedActors)
		{
			if (Entry.OwnerTag == OwnerTag)
			{
				if (AActor* Actor = Entry.Actor.Get())
				{
					Actor->Destroy();
					++Destroyed;
				}
				Entry.Actor.Reset();
			}
		}
		OwnedActors.RemoveAll([](const FOwnedActor& Entry) { return !Entry.Actor.IsValid(); });
		return Destroyed;
	}

	void ForgetTag(FGameplayTag OwnerTag)
	{
		OwnedActors.RemoveAll([OwnerTag](const FOwnedActor& Entry) { return Entry.OwnerTag == OwnerTag; });
		OwnedHandles.RemoveAll([OwnerTag](const FOwnedHandle& Entry) { return Entry.OwnerTag == OwnerTag; });
	}

	void Reset()
	{
		OwnedActors.Reset();
		OwnedHandles.Reset();
	}

	int32 NumActors() const { return OwnedActors.Num(); }
	int32 NumHandles() const { return OwnedHandles.Num(); }

private:
	TArray<FOwnedActor> OwnedActors;
	TArray<FOwnedHandle> OwnedHandles;
};
