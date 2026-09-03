// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

//
/**
 * [ES] Helpers para crear Actor+Component en harness sin duplicar BeginPlay ni
 *      olvidar EndPlay/Destroy. Encapsula el patrón frágil visto en RuntimeCore.
 * [EN] Helpers for creating Actor+Component pairs in harnesses without double
 *      BeginPlay or missing EndPlay/Destroy. Encapsulates the fragile RuntimeCore pattern.
 *
 * Depende de / Depends on: AActor, UActorComponent.
 * Usado por / Used by: PGXSimHarness and reusable component smoke tests.
 */
struct FPGXComponentLifecycleHarness
{
	template<typename TComponent>
	static TComponent* CreateRuntimeComponent(UWorld* World, AActor* Owner, FName ComponentName = NAME_None)
	{
		if (!IsValid(World) || !IsValid(Owner))
		{
			return nullptr;
		}

		TComponent* Component = ComponentName.IsNone()
			? NewObject<TComponent>(Owner)
			: NewObject<TComponent>(Owner, TComponent::StaticClass(), ComponentName);
		if (!IsValid(Component))
		{
			return nullptr;
		}

		Owner->AddInstanceComponent(Component);
		if (!Component->IsRegistered())
		{
			Component->RegisterComponentWithWorld(World);
		}
		Component->RegisterAllComponentTickFunctions(true);
		EnsureBegunPlayOnce(Component);
		return Component;
	}

	static void EnsureBegunPlayOnce(UActorComponent* Component)
	{
		if (IsValid(Component) && !Component->HasBegunPlay())
		{
			Component->BeginPlay();
		}
	}

	static void EndPlayOnce(UActorComponent* Component, EEndPlayReason::Type Reason = EEndPlayReason::Destroyed)
	{
		if (IsValid(Component) && Component->HasBegunPlay())
		{
			Component->EndPlay(Reason);
		}
	}

	static void EndPlayAndDestroyActor(AActor* Actor, EEndPlayReason::Type Reason = EEndPlayReason::Destroyed)
	{
		if (!IsValid(Actor))
		{
			return;
		}

		// EN/ES: Let the engine dispatch component EndPlay once during Actor::Destroy.
		Actor->Destroy();
	}
};
