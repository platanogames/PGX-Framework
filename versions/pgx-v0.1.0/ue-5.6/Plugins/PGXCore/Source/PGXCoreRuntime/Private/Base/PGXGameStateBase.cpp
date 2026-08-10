// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Base/PGXGameStateBase.h"
#include "Base/PGXBaseLog.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Construction/PGXGameStateConstruction.h"
#include "Construction/PGXConstructionResolver.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/Tags/PGXBridgeTags.h"

// EN: Base game state for PGX projects implementation
// ES: Implementacion del game state base para proyectos PGX

APGXGameStateBase::APGXGameStateBase()
{
}

FString APGXGameStateBase::GetPGXDisplayName_Implementation() const
{
	return GetName();
}

void APGXGameStateBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogPGXBase, Verbose, TEXT("GameState [%s]: BeginPlay"), *GetPGXDisplayName());

	// 1. Resolve DA: instance override → global
	const UPGXGameStateConstruction* Construction = nullptr;
	if (!ConstructionOverride.IsNull())
	{
		Construction = ConstructionOverride.LoadSynchronous();
	}
	if (!Construction)
	{
		Construction = FPGXConstructionResolver::Resolve<UPGXGameStateConstruction>(GetWorld());
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
}

void APGXGameStateBase::ApplyConstruction_Implementation(const UPGXGameStateConstruction* Construction)
{
	if (!Construction)
	{
		return;
	}

	// EN: Inject components / ES: Inyectar componentes
	FPGXConstructionResolver::InjectComponents(this, Construction);

	UE_LOG(LogPGXConstruction, Verbose, TEXT("GameState [%s]: Construction applied from [%s]"),
		*GetName(), *Construction->GetName());
}

void APGXGameStateBase::ApplyPlatformProfile_Implementation()
{
	// EN: Override in derived classes to apply platform-specific settings
	// ES: Sobreescribir en clases derivadas para aplicar configuracion de plataforma
}

void APGXGameStateBase::FirePGXEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid()) { return; }
	UPGXEventHandlerSubsystem* EventHandler = UPGXEventHandlerSubsystem::Get(this);
	if (!IsValid(EventHandler)) { return; }
	EventHandler->ResolveAndExecute(EventTag, this, FInstancedStruct());
}

UPGXDataAsset* APGXGameStateBase::QueryPGXRegistry(FGameplayTag DatabaseTag, FGameplayTag EntryTag) const
{
	if (!DatabaseTag.IsValid() || !EntryTag.IsValid()) { return nullptr; }
	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!IsValid(Registry)) { return nullptr; }
	return Registry->ResolveAsset(DatabaseTag, EntryTag);
}

void APGXGameStateBase::SetupSaveBridge(FGameplayTag InSaveDomain)
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

void APGXGameStateBase::TeardownSaveBridge()
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

void APGXGameStateBase::OnPGXPreSave_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXGameStateBase::OnPGXPostLoad_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXGameStateBase::OnGameFlowStateChanged_Implementation(FGameplayTag OldState, FGameplayTag NewState) {}
void APGXGameStateBase::OnLevelTransitionStarted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXGameStateBase::OnLevelTransitionCompleted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXGameStateBase::OnLoadingScreenStateChanged_Implementation(const FPGXBridgeLoadingState& State) {}

void APGXGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogPGXBase, Verbose, TEXT("GameState [%s]: EndPlay (%s)"),
		*GetPGXDisplayName(), *UEnum::GetValueAsString(EndPlayReason));
	TeardownSaveBridge();
	PGXBaseMessaging::UnregisterAll(MessageListenerHandles);
	Super::EndPlay(EndPlayReason);
}
