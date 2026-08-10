// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Base/PGXGameModeBase.h"
#include "Base/PGXBaseLog.h"
#include "Base/PGXBaseEventTags.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Construction/PGXGameModeConstruction.h"
#include "Construction/PGXConstructionResolver.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/HUD.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/Tags/PGXBridgeTags.h"

// EN: Base game mode for PGX projects implementation
// ES: Implementacion del game mode base para proyectos PGX

APGXGameModeBase::APGXGameModeBase()
{
}

FString APGXGameModeBase::GetPGXDisplayName_Implementation() const
{
	return GetName();
}

void APGXGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogPGXBase, Verbose, TEXT("GameMode [%s]: InitGame | Map=%s"), *GetPGXDisplayName(), *MapName);

	// 1. Resolve DA: instance override → global
	const UPGXGameModeConstruction* Construction = nullptr;
	if (!ConstructionOverride.IsNull())
	{
		Construction = ConstructionOverride.LoadSynchronous();
	}
	if (!Construction)
	{
		Construction = FPGXConstructionResolver::Resolve<UPGXGameModeConstruction>(GetWorld());
	}

	// 2. Structural from DA (class assignments, components, tags)
	if (Construction)
	{
		ApplyConstruction(Construction);
		ApplyPlatformProfile();

		// EN: Publish initial GameFlow state if configured in DA
		// ES: Publicar estado inicial de GameFlow si configurado en DA
		if (Construction->InitialGameFlowState.IsValid())
		{
			PublishGameFlowState(Construction->InitialGameFlowState);
		}
	}

	// 3. Bridges from INSTANCE PROPERTIES (not from DA)
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

void APGXGameModeBase::ApplyConstruction_Implementation(const UPGXGameModeConstruction* Construction)
{
	if (!Construction)
	{
		return;
	}

	// EN: Apply class assignments / ES: Aplicar asignaciones de clases
	if (!Construction->DefaultPawnClass.IsNull())
	{
		UClass* PawnClass = Construction->DefaultPawnClass.LoadSynchronous();
		if (PawnClass)
		{
			DefaultPawnClass = PawnClass;
		}
	}

	if (!Construction->PlayerControllerClass.IsNull())
	{
		UClass* PCClass = Construction->PlayerControllerClass.LoadSynchronous();
		if (PCClass)
		{
			PlayerControllerClass = PCClass;
		}
	}

	if (!Construction->PlayerStateClass.IsNull())
	{
		UClass* PSClass = Construction->PlayerStateClass.LoadSynchronous();
		if (PSClass)
		{
			PlayerStateClass = PSClass;
		}
	}

	if (!Construction->GameStateClass.IsNull())
	{
		UClass* GSClass = Construction->GameStateClass.LoadSynchronous();
		if (GSClass)
		{
			GameStateClass = GSClass;
		}
	}

	if (!Construction->HUDClass.IsNull())
	{
		UClass* HClass = Construction->HUDClass.LoadSynchronous();
		if (HClass)
		{
			HUDClass = HClass;
		}
	}

	if (!Construction->SpectatorClass.IsNull())
	{
		UClass* SpecClass = Construction->SpectatorClass.LoadSynchronous();
		if (SpecClass)
		{
			SpectatorClass = SpecClass;
		}
	}

	// EN: Inject components / ES: Inyectar componentes
	FPGXConstructionResolver::InjectComponents(this, Construction);

	UE_LOG(LogPGXConstruction, Log, TEXT("GameMode [%s]: Construction applied from [%s]"),
		*GetName(), *Construction->GetName());
}

void APGXGameModeBase::ApplyPlatformProfile_Implementation()
{
	// EN: Override in derived classes to apply platform-specific settings
	// ES: Sobreescribir en clases derivadas para aplicar configuracion de plataforma
}

void APGXGameModeBase::FirePGXEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid()) { return; }
	UPGXEventHandlerSubsystem* EventHandler = UPGXEventHandlerSubsystem::Get(this);
	if (!IsValid(EventHandler)) { return; }
	EventHandler->ResolveAndExecute(EventTag, this, FInstancedStruct());
}

UPGXDataAsset* APGXGameModeBase::QueryPGXRegistry(FGameplayTag DatabaseTag, FGameplayTag EntryTag) const
{
	if (!DatabaseTag.IsValid() || !EntryTag.IsValid()) { return nullptr; }
	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!IsValid(Registry)) { return nullptr; }
	return Registry->ResolveAsset(DatabaseTag, EntryTag);
}

void APGXGameModeBase::FireMatchStartedEvent()
{
	FirePGXEvent(TAG_PGX_Event_Match_Started);
}

void APGXGameModeBase::FireMatchEndedEvent()
{
	FirePGXEvent(TAG_PGX_Event_Match_Ended);
}

void APGXGameModeBase::FirePlayerJoinedEvent(APlayerController* NewPlayer)
{
	if (!IsValid(NewPlayer)) { return; }
	UPGXEventHandlerSubsystem* EventHandler = UPGXEventHandlerSubsystem::Get(this);
	if (!IsValid(EventHandler)) { return; }
	FPGXEventContext Context;
	Context.Instigator = this;
	Context.Target = NewPlayer;
	EventHandler->ResolveAndExecuteWithContext(TAG_PGX_Event_Player_Joined, Context, FInstancedStruct());
}

void APGXGameModeBase::FirePlayerLeftEvent(AController* Exiting)
{
	if (!IsValid(Exiting)) { return; }
	UPGXEventHandlerSubsystem* EventHandler = UPGXEventHandlerSubsystem::Get(this);
	if (!IsValid(EventHandler)) { return; }
	FPGXEventContext Context;
	Context.Instigator = this;
	Context.Target = Exiting;
	EventHandler->ResolveAndExecuteWithContext(TAG_PGX_Event_Player_Left, Context, FInstancedStruct());
}

// ─── L2 Bridge Implementations ───

void APGXGameModeBase::PublishGameFlowState(FGameplayTag NewState)
{
	FPGXBridgeGameFlowChanged Msg;
	Msg.OldState = CachedGameFlowState;
	Msg.NewState = NewState;
	Msg.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	CachedGameFlowState = NewState;
	BroadcastPGXMessage(TAG_PGX_Bridge_GameFlow_StateChanged, Msg);
}

void APGXGameModeBase::SetupSaveBridge(FGameplayTag InSaveDomain)
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

void APGXGameModeBase::TeardownSaveBridge()
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

void APGXGameModeBase::OnPGXPreSave_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXGameModeBase::OnPGXPostLoad_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXGameModeBase::OnGameFlowStateChanged_Implementation(FGameplayTag OldState, FGameplayTag NewState) {}
void APGXGameModeBase::OnLevelTransitionStarted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXGameModeBase::OnLevelTransitionCompleted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXGameModeBase::OnLoadingScreenStateChanged_Implementation(const FPGXBridgeLoadingState& State) {}

void APGXGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogPGXBase, Verbose, TEXT("GameMode [%s]: EndPlay (%s)"),
		*GetPGXDisplayName(), *UEnum::GetValueAsString(EndPlayReason));
	TeardownSaveBridge();
	PGXBaseMessaging::UnregisterAll(MessageListenerHandles);
	Super::EndPlay(EndPlayReason);
}
