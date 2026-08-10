// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Base/PGXCoreHUD.h"
#include "Base/PGXBaseLog.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Construction/PGXHUDConstruction.h"
#include "Construction/PGXConstructionResolver.h"
#include "Blueprint/UserWidget.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/Tags/PGXBridgeTags.h"

APGXCoreHUD::APGXCoreHUD()
{
}

FString APGXCoreHUD::GetPGXDisplayName_Implementation() const
{
	return GetName();
}

void APGXCoreHUD::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogPGXBase, Verbose, TEXT("HUD [%s]: BeginPlay"), *GetPGXDisplayName());

	// 1. Resolve DA: instance override -> global
	const UPGXHUDConstruction* Construction = nullptr;
	if (!ConstructionOverride.IsNull())
	{
		Construction = ConstructionOverride.LoadSynchronous();
	}
	if (!Construction)
	{
		Construction = FPGXConstructionResolver::Resolve<UPGXHUDConstruction>(GetWorld());
	}

	// 2. Structural from DA (components, widgets, layers)
	if (Construction)
	{
		ApplyConstruction(Construction);
		ApplyPlatformProfile();
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

void APGXCoreHUD::ApplyConstruction_Implementation(const UPGXHUDConstruction* Construction)
{
	if (!Construction)
	{
		return;
	}

	// EN: Inject components / ES: Inyectar componentes
	FPGXConstructionResolver::InjectComponents(this, Construction);

	// EN: Create main HUD widget (if a player controller is available)
	// ES: Crear widget principal del HUD (si un player controller esta disponible)
	if (!Construction->MainHUDWidgetClass.IsNull())
	{
		UClass* WidgetClass = Construction->MainHUDWidgetClass.LoadSynchronous();
		if (WidgetClass && GetOwningPlayerController())
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(GetOwningPlayerController(), WidgetClass);
			if (Widget)
			{
				Widget->AddToViewport(0);
			}
		}
	}

	// EN: Create HUD layers / ES: Crear capas de HUD
	for (const FPGXHUDLayerEntry& Layer : Construction->HUDLayers)
	{
		if (Layer.WidgetClass.IsNull())
		{
			continue;
		}
		UClass* LayerClass = Layer.WidgetClass.LoadSynchronous();
		if (LayerClass && GetOwningPlayerController())
		{
			UUserWidget* LayerWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), LayerClass);
			if (LayerWidget)
			{
				LayerWidget->AddToViewport(Layer.ZOrder);
			}
		}
	}

	UE_LOG(LogPGXConstruction, Verbose, TEXT("HUD [%s]: Construction applied from [%s]"),
		*GetName(), *Construction->GetName());
}

void APGXCoreHUD::ApplyPlatformProfile_Implementation()
{
	// EN: Override in derived classes to apply platform-specific settings
	// ES: Sobreescribir en clases derivadas para aplicar configuracion de plataforma
}

void APGXCoreHUD::FirePGXEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid()) { return; }
	UPGXEventHandlerSubsystem* EventHandler = UPGXEventHandlerSubsystem::Get(this);
	if (!IsValid(EventHandler)) { return; }
	EventHandler->ResolveAndExecute(EventTag, this, FInstancedStruct());
}

UPGXDataAsset* APGXCoreHUD::QueryPGXRegistry(FGameplayTag DatabaseTag, FGameplayTag EntryTag) const
{
	if (!DatabaseTag.IsValid() || !EntryTag.IsValid()) { return nullptr; }
	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!IsValid(Registry)) { return nullptr; }
	return Registry->ResolveAsset(DatabaseTag, EntryTag);
}

// ─── L2 Bridge Implementations ───

void APGXCoreHUD::SetupSaveBridge(FGameplayTag InSaveDomain)
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

void APGXCoreHUD::TeardownSaveBridge()
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

void APGXCoreHUD::OnPGXPreSave_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXCoreHUD::OnPGXPostLoad_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void APGXCoreHUD::OnGameFlowStateChanged_Implementation(FGameplayTag OldState, FGameplayTag NewState) {}
void APGXCoreHUD::OnLevelTransitionStarted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXCoreHUD::OnLevelTransitionCompleted_Implementation(const FPGXBridgeLevelTransition& Transition) {}
void APGXCoreHUD::OnLoadingScreenStateChanged_Implementation(const FPGXBridgeLoadingState& State) {}

void APGXCoreHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogPGXBase, Verbose, TEXT("HUD [%s]: EndPlay (%s)"),
		*GetPGXDisplayName(), *UEnum::GetValueAsString(EndPlayReason));
	TeardownSaveBridge();
	PGXBaseMessaging::UnregisterAll(MessageListenerHandles);
	Super::EndPlay(EndPlayReason);
}
