// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAbilityComponent.h"
#include "PGXAbilityRuntime.h"
#include "PGXAbilityFacade.h"
#include "PGXAttributeFacade.h"
#include "PGXEffectFacade.h"
#include "PGXAbilitySubsystem.h"
#include "AbilitySystemComponent.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"

/**
 * EN: Constructor. Sets default values for this component.
 * ES: Constructor. Establece valores por defecto para este componente.
 */
UPGXAbilityComponent::UPGXAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UPGXAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	ResolveAbilitySystemComponent();

	AbilityFacade = NewObject<UPGXAbilityFacade>(this);
	AttributeFacade = NewObject<UPGXAttributeFacade>(this);
	EffectFacade = NewObject<UPGXEffectFacade>(this);

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UPGXAbilitySubsystem* Subsystem = GameInstance->GetSubsystem<UPGXAbilitySubsystem>())
			{
				Subsystem->RegisterComponent(this);
			}
		}
	}
}

void UPGXAbilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UPGXAbilitySubsystem* Subsystem = GameInstance->GetSubsystem<UPGXAbilitySubsystem>())
			{
				Subsystem->UnregisterComponent(this);
			}
		}
	}

	AbilityFacade = nullptr;
	AttributeFacade = nullptr;
	EffectFacade = nullptr;

	// EN: Do not destroy a pre-existing owner-authored ASC; only release our own reference. If we
	//     created it (see ResolveAbilitySystemComponent), it is an instance component on the
	//     owner and will be torn down with the owner actor.
	// ES: No destruir un ASC pre-existente authoring del owner; solo liberar nuestra referencia.
	//     Si lo creamos, es un instance component del owner y se destruye con el actor owner.
	AbilitySystemComponent = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UPGXAbilityComponent::ResolveAbilitySystemComponent()
{
	if (IsValid(AbilitySystemComponent))
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		PGX_LOG_WARNING(LogPGXAbility, TEXT("UPGXAbilityComponent::ResolveAbilitySystemComponent — no valid owner."));
		return;
	}

	// EN: Reuse an existing ASC if the owner (or a parent class) already added one — common in
	//     multiplayer projects that put the ASC on PlayerState. Never create a duplicate.
	// ES: Reusar un ASC existente si el owner ya tiene uno — comun en proyectos multiplayer que
	//     ponen el ASC en PlayerState. Nunca crear un duplicado.
	if (UAbilitySystemComponent* ExistingASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
	{
		AbilitySystemComponent = ExistingASC;

		// EN: Only init ActorInfo if not already set up (e.g. a PlayerState-owned ASC may already
		//     be initialized with Owner=PlayerState/Avatar=Pawn by project code — do not stomp that).
		//     Without AbilityActorInfo, TryActivateAbilitiesByTag/cooldowns silently fail at runtime
		//     (required for correct runtime behavior).
		// ES: Solo inicializar ActorInfo si aun no esta configurado (un ASC en PlayerState puede ya
		//     estar inicializado con Owner=PlayerState/Avatar=Pawn por codigo del proyecto — no pisar
		//     eso). Sin AbilityActorInfo, TryActivateAbilitiesByTag/cooldowns fallan silenciosamente.
		if (!ExistingASC->AbilityActorInfo.IsValid() || !ExistingASC->AbilityActorInfo->OwnerActor.IsValid())
		{
			ExistingASC->InitAbilityActorInfo(Owner, Owner);
		}

		PGX_LOG_VERBOSE(LogPGXAbility, TEXT("UPGXAbilityComponent::ResolveAbilitySystemComponent — reused existing ASC on %s"), *Owner->GetName());
		return;
	}

	// EN: Lazily create one. Runtime-created components must be registered explicitly (they did
	//     not go through CreateDefaultSubobject in the owner's constructor).
	// ES: Crear uno perezosamente. Componentes creados en runtime deben registrarse explicitamente.
	UAbilitySystemComponent* NewASC = NewObject<UAbilitySystemComponent>(Owner, TEXT("PGXAbilitySystemComponent"));
	if (!IsValid(NewASC))
	{
		PGX_LOG_ERROR(LogPGXAbility, TEXT("UPGXAbilityComponent::ResolveAbilitySystemComponent — failed to create ASC on %s"), *Owner->GetName());
		return;
	}

	NewASC->RegisterComponent();
	Owner->AddInstanceComponent(NewASC);
	AbilitySystemComponent = NewASC;

	// EN: Replication mode is intentionally left at the GAS default (Full) here. Full is correct
	//     for single-player/testing; Mixed (PlayerState-owned ASC, locally-controlled Pawn) is
	//     correct for replicated player characters; Minimal is correct for AI-controlled agents.
	//     The right mode depends on project-specific actor ownership (Player vs AI), which this
	//     generic framework component cannot infer — the multiplayer-agnostic boundary of this component;
	//     authority decision in Architecture design section 14. Project code should call
	//     `GetAbilitySystemComponentInternal()->SetReplicationMode(...)` after BeginPlay if it
	//     needs Mixed/Minimal.
	// ES: El modo de replicacion se deja intencionalmente en el default de GAS (Full). Full es
	//     correcto para single-player/testing; Mixed es correcto para personajes jugador
	//     replicados; Minimal para agentes controlados por AI. El modo correcto depende de la
	//     propiedad del actor (Player vs AI), que este componente generico no puede inferir.
	PGX_LOG_INFO(LogPGXAbility, TEXT("UPGXAbilityComponent::ResolveAbilitySystemComponent — created ASC on %s (replication mode: Full default, project should override for Player/AI)"), *Owner->GetName());
}

UPGXAbilityFacade* UPGXAbilityComponent::GetAbilityFacade()
{
	if (!IsValid(AbilityFacade))
	{
		AbilityFacade = NewObject<UPGXAbilityFacade>(this);
	}
	return AbilityFacade;
}

UPGXAttributeFacade* UPGXAbilityComponent::GetAttributeFacade()
{
	if (!IsValid(AttributeFacade))
	{
		AttributeFacade = NewObject<UPGXAttributeFacade>(this);
	}
	return AttributeFacade;
}

UPGXEffectFacade* UPGXAbilityComponent::GetEffectFacade()
{
	if (!IsValid(EffectFacade))
	{
		EffectFacade = NewObject<UPGXEffectFacade>(this);
	}
	return EffectFacade;
}

bool UPGXAbilityComponent::IsAbilitySystemReady() const
{
	return IsValid(AbilitySystemComponent);
}

int32 UPGXAbilityComponent::GetActiveAbilityCount() const
{
	if (!IsValid(AbilitySystemComponent))
	{
		return 0;
	}

	int32 Count = 0;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.IsActive())
		{
			++Count;
		}
	}
	return Count;
}
