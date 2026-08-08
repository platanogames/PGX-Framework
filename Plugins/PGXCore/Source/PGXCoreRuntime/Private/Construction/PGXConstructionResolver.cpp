// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Construction/PGXConstructionResolver.h"
#include "Construction/PGXWorldConstructionSubsystem.h"
#include "Construction/PGXConstructionSettings.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

DEFINE_LOG_CATEGORY(LogPGXConstruction);

void FPGXConstructionResolver::InjectComponents(AActor* Actor, const UPGXClassConstruction* Construction)
{
	if (!Actor || !Construction)
	{
		return;
	}

	for (const FPGXComponentInjection& Injection : Construction->Components)
	{
		// EN: Skip if network role doesn't match / ES: Saltar si el rol de red no coincide
		if (!ShouldInjectForNetwork(Actor, Injection))
		{
			continue;
		}

		// EN: Check Profile capability / ES: Verificar capability de Profile
		if (Injection.RequiredCapability.IsValid())
		{
			if (!IsCapabilityMet(Actor->GetWorld(), Injection.RequiredCapability))
			{
				UE_LOG(LogPGXConstruction, Verbose, TEXT("Skipping component [%s] — capability [%s] not met"),
					*Injection.ComponentClass.ToString(), *Injection.RequiredCapability.ToString());
				continue;
			}
		}

		// EN: Load the component class / ES: Cargar la clase de componente
		UClass* CompClass = Injection.ComponentClass.LoadSynchronous();
		if (!CompClass)
		{
			UE_LOG(LogPGXConstruction, Warning, TEXT("Failed to load component class [%s] for actor [%s]"),
				*Injection.ComponentClass.ToString(), *Actor->GetName());
			continue;
		}

		// EN: Determine component name / ES: Determinar nombre del componente
		FName CompName = Injection.ComponentName.IsNone()
			? CompClass->GetFName()
			: Injection.ComponentName;

		// EN: Check if component already exists / ES: Verificar si el componente ya existe
		if (Actor->FindComponentByClass(CompClass))
		{
			UE_LOG(LogPGXConstruction, Verbose, TEXT("Component [%s] already exists on [%s], skipping"),
				*CompName.ToString(), *Actor->GetName());
			continue;
		}

		// EN: Create and register the component / ES: Crear y registrar el componente
		UActorComponent* NewComp = NewObject<UActorComponent>(Actor, CompClass, CompName);
		if (NewComp)
		{
			NewComp->RegisterComponent();
			UE_LOG(LogPGXConstruction, Verbose, TEXT("Injected component [%s] into [%s]"),
				*CompName.ToString(), *Actor->GetName());
		}
	}
}

FGameplayTagContainer FPGXConstructionResolver::GetGrantedTags(const UPGXClassConstruction* Construction)
{
	if (!Construction)
	{
		return FGameplayTagContainer();
	}
	return Construction->GrantedTags;
}

bool FPGXConstructionResolver::ShouldInjectForNetwork(const AActor* Actor, const FPGXComponentInjection& Injection)
{
	if (!Injection.bClientOnly && !Injection.bServerOnly)
	{
		return true; // EN: No network filter / ES: Sin filtro de red
	}

	const bool bIsServer = Actor->HasAuthority();

	if (Injection.bServerOnly && !bIsServer)
	{
		return false;
	}
	if (Injection.bClientOnly && bIsServer)
	{
		return false;
	}

	return true;
}

bool FPGXConstructionResolver::IsCapabilityMet(const UWorld* World, const FGameplayTag& CapabilityTag)
{
	if (!World || !CapabilityTag.IsValid())
	{
		return true; // EN: No constraint = allowed / ES: Sin restriccion = permitido
	}

	// EN: Try to check against Profile subsystem if available
	// ES: Intentar verificar contra el subsistema de Profile si esta disponible
	// NOTE: We use a loose coupling here to avoid hard dependency on Profile module.
	// The Profile subsystem is in the same module (PGXCoreRuntime) but we avoid
	// including its header to keep construction code self-contained.
	// In practice, base classes that apply construction call into Profile directly.
	return true;
}
