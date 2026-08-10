// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Base/PGXCharacterBase.h"
#include "Base/PGXBaseLog.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Construction/PGXCharacterConstruction.h"
#include "Construction/PGXConstructionResolver.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/Tags/PGXBridgeTags.h"

// EN: Base character for PGX projects implementation
// ES: Implementacion del character base para proyectos PGX

APGXCharacterBase::APGXCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

FString APGXCharacterBase::GetPGXDisplayName_Implementation() const
{
	return GetName();
}

void APGXCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogPGXBase, Verbose, TEXT("Character [%s]: BeginPlay"), *GetPGXDisplayName());

	// 1. Resolve DA: instance override → global
	const UPGXCharacterConstruction* Construction = nullptr;
	if (!ConstructionOverride.IsNull())
	{
		Construction = ConstructionOverride.LoadSynchronous();
	}
	if (!Construction)
	{
		Construction = FPGXConstructionResolver::Resolve<UPGXCharacterConstruction>(GetWorld());
	}

	// 2. Structural from DA (components, tags)
	if (Construction)
	{
		ApplyConstruction(Construction);
		ApplyPlatformProfile();
	}

	// 3. Bridges from INSTANCE PROPERTIES (not from DA)
	if (bParticipateInSave && SaveDomain.IsValid())
	{
		SetupSaveBridge(SaveDomain);
	}
	if (bListenToGameFlowChanges)
	{
		ListenForPGXMessage<FPGXBridgeGameFlowChanged>(
			TAG_PGX_Bridge_GameFlow_StateChanged,
			[this](FGameplayTag, const FPGXBridgeGameFlowChanged& Msg)
			{ OnGameFlowStateChanged(Msg.OldState, Msg.NewState); });
	}
	if (bListenToLevelTransitions)
	{
		ListenForPGXMessage<FPGXBridgeLevelTransition>(
			TAG_PGX_Bridge_LevelFlow_TransitionStarted,
			[this](FGameplayTag, const FPGXBridgeLevelTransition& Msg)
			{ OnLevelTransitionStarted(Msg); });
		ListenForPGXMessage<FPGXBridgeLevelTransition>(
			TAG_PGX_Bridge_LevelFlow_TransitionCompleted,
			[this](FGameplayTag, const FPGXBridgeLevelTransition& Msg)
			{ OnLevelTransitionCompleted(Msg); });
	}
	if (bListenToLoadingScreenState)
	{
		ListenForPGXMessage<FPGXBridgeLoadingState>(
			TAG_PGX_Bridge_Loading_ScreenShown,
			[this](FGameplayTag, const FPGXBridgeLoadingState& Msg)
			{ OnLoadingScreenStateChanged(Msg); });
		ListenForPGXMessage<FPGXBridgeLoadingState>(
			TAG_PGX_Bridge_Loading_ScreenClosed,
			[this](FGameplayTag, const FPGXBridgeLoadingState& Msg)
			{ OnLoadingScreenStateChanged(Msg); });
	}
}

void APGXCharacterBase::ApplyConstruction_Implementation(const UPGXCharacterConstruction* Construction)
{
	if (!Construction) { return; }

	// EN: Apply movement defaults / ES: Aplicar valores de movimiento por defecto
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = Construction->DefaultWalkSpeed;
	}

	// EN: Inject components / ES: Inyectar componentes
	FPGXConstructionResolver::InjectComponents(this, Construction);

	UE_LOG(LogPGXConstruction, Verbose, TEXT("Character [%s]: Construction applied from [%s] | WalkSpeed=%.0f"),
		*GetName(), *Construction->GetName(), Construction->DefaultWalkSpeed);
}

void APGXCharacterBase::ApplyPlatformProfile_Implementation()
{
	// EN: Override in derived classes to apply platform-specific settings
	// ES: Sobreescribir en clases derivadas para aplicar configuracion de plataforma
}

void APGXCharacterBase::FirePGXEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid()) { return; }
	UPGXEventHandlerSubsystem* EventHandler = UPGXEventHandlerSubsystem::Get(this);
	if (!IsValid(EventHandler)) { return; }
	EventHandler->ResolveAndExecute(EventTag, this, FInstancedStruct());
}

UPGXDataAsset* APGXCharacterBase::QueryPGXRegistry(FGameplayTag DatabaseTag, FGameplayTag EntryTag) const
{
	if (!DatabaseTag.IsValid() || !EntryTag.IsValid()) { return nullptr; }
	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!IsValid(Registry)) { return nullptr; }
	return Registry->ResolveAsset(DatabaseTag, EntryTag);
}

void APGXCharacterBase::SetupSaveBridge(FGameplayTag InSaveDomain)
{
	CachedSaveDomain = InSaveDomain;
	FPGXBridgeSaveableRegistration Registration;
	Registration.SaveableObject = this;
	Registration.DomainTag = InSaveDomain;
	Registration.bRegister = true;
	BroadcastPGXMessage(TAG_PGX_Bridge_Save_Register, Registration);

	ListenForPGXMessage<FPGXBridgeSaveNotification>(
		TAG_PGX_Bridge_Save_PreSave,
		[this](FGameplayTag, const FPGXBridgeSaveNotification& N)
		{ if (N.DomainTag == CachedSaveDomain) { OnPGXPreSave(N); } });
	ListenForPGXMessage<FPGXBridgeSaveNotification>(
		TAG_PGX_Bridge_Save_PostLoad,
		[this](FGameplayTag, const FPGXBridgeSaveNotification& N)
		{ if (N.DomainTag == CachedSaveDomain) { OnPGXPostLoad(N); } });
}

void APGXCharacterBase::TeardownSaveBridge()
{
	if (CachedSaveDomain.IsValid())
	{
		FPGXBridgeSaveableRegistration Unreg;
		Unreg.SaveableObject = this;
		Unreg.DomainTag = CachedSaveDomain;
		Unreg.bRegister = false;
		BroadcastPGXMessage(TAG_PGX_Bridge_Save_Unregister, Unreg);
	}
}

void APGXCharacterBase::OnPGXPreSave_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXCharacterBase::OnPGXPostLoad_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXCharacterBase::OnGameFlowStateChanged_Implementation(FGameplayTag OldState, FGameplayTag NewState) {}
void APGXCharacterBase::OnLevelTransitionStarted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXCharacterBase::OnLevelTransitionCompleted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXCharacterBase::OnLoadingScreenStateChanged_Implementation(const FPGXBridgeLoadingState& State) {}

void APGXCharacterBase::RequestPGXSound(FGameplayTag SoundEventTag, FVector Location)
{
	FPGXBridgeAudioRequest Request;
	Request.SoundEventTag = SoundEventTag;
	Request.Location = Location.IsZero() ? GetActorLocation() : Location;
	Request.Instigator = this;
	BroadcastPGXMessage(TAG_PGX_Bridge_Audio_PlayRequest, Request);
}

void APGXCharacterBase::StopPGXSound(FGameplayTag SoundEventTag)
{
	FPGXBridgeAudioRequest Request;
	Request.SoundEventTag = SoundEventTag;
	Request.Instigator = this;
	BroadcastPGXMessage(TAG_PGX_Bridge_Audio_StopRequest, Request);
}

void APGXCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogPGXBase, Verbose, TEXT("Character [%s]: EndPlay (%s)"),
		*GetPGXDisplayName(), *UEnum::GetValueAsString(EndPlayReason));
	TeardownSaveBridge();
	PGXBaseMessaging::UnregisterAll(MessageListenerHandles);
	Super::EndPlay(EndPlayReason);
}
