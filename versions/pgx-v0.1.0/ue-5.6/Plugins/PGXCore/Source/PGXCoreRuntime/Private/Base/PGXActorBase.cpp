// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Base/PGXActorBase.h"
#include "Base/PGXBaseLog.h"
#include "Construction/PGXActorConstruction.h"
#include "Construction/PGXConstructionResolver.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/Tags/PGXBridgeTags.h"

// EN: Base actor for PGX projects implementation
// ES: Implementacion del actor base para proyectos PGX

APGXActorBase::APGXActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

FString APGXActorBase::GetPGXDisplayName_Implementation() const
{
	return GetName();
}

void APGXActorBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogPGXBase, Verbose, TEXT("ActorBase [%s]: BeginPlay"), *GetPGXDisplayName());

	// 1. Resolve DA: instance override → global
	const UPGXActorConstruction* Construction = nullptr;
	if (!ConstructionOverride.IsNull())
	{
		Construction = ConstructionOverride.LoadSynchronous();
	}
	if (!Construction)
	{
		Construction = FPGXConstructionResolver::Resolve<UPGXActorConstruction>(GetWorld());
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

void APGXActorBase::ApplyConstruction_Implementation(const UPGXActorConstruction* Construction)
{
	if (!Construction) { return; }

	// EN: Inject components from DA configuration
	// ES: Inyectar componentes desde configuracion del DA
	FPGXConstructionResolver::InjectComponents(this, Construction);

	UE_LOG(LogPGXBase, Verbose, TEXT("ActorBase [%s]: Construction applied from [%s]"),
		*GetName(), *Construction->GetName());
}

void APGXActorBase::ApplyPlatformProfile_Implementation()
{
	// EN: Override in derived classes to apply platform-specific settings
	// ES: Sobreescribir en clases derivadas para aplicar configuracion de plataforma
}

void APGXActorBase::FirePGXEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid()) { return; }
	UPGXEventHandlerSubsystem* EventHandler = UPGXEventHandlerSubsystem::Get(this);
	if (!IsValid(EventHandler)) { return; }
	EventHandler->ResolveAndExecute(EventTag, this, FInstancedStruct());
}

UPGXDataAsset* APGXActorBase::QueryPGXRegistry(FGameplayTag DatabaseTag, FGameplayTag EntryTag) const
{
	if (!DatabaseTag.IsValid() || !EntryTag.IsValid()) { return nullptr; }
	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!IsValid(Registry)) { return nullptr; }
	return Registry->ResolveAsset(DatabaseTag, EntryTag);
}

void APGXActorBase::SetupSaveBridge(FGameplayTag InSaveDomain)
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

void APGXActorBase::TeardownSaveBridge()
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

void APGXActorBase::OnPGXPreSave_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXActorBase::OnPGXPostLoad_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXActorBase::OnGameFlowStateChanged_Implementation(FGameplayTag OldState, FGameplayTag NewState) {}
void APGXActorBase::OnLevelTransitionStarted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXActorBase::OnLevelTransitionCompleted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXActorBase::OnLoadingScreenStateChanged_Implementation(const FPGXBridgeLoadingState& State) {}

void APGXActorBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogPGXBase, Verbose, TEXT("%s [%s]: EndPlay (%s)"),
		*StaticClass()->GetName(), *GetPGXDisplayName(),
		*UEnum::GetValueAsString(EndPlayReason));
	TeardownSaveBridge();
	PGXBaseMessaging::UnregisterAll(MessageListenerHandles);
	Super::EndPlay(EndPlayReason);
}
