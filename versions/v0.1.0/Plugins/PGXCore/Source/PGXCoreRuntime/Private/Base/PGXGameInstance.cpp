// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Base/PGXGameInstance.h"
#include "Base/PGXBaseLog.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/Tags/PGXBridgeTags.h"

UPGXGameInstance::UPGXGameInstance()
{
}

FString UPGXGameInstance::GetPGXDisplayName_Implementation() const
{
	return TEXT("GameInstance");
}

void UPGXGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogPGXBase, Log, TEXT("GameInstance [%s]: Init"), *GetPGXDisplayName());
	ApplyPlatformProfile();
	OnRegistryReady();

	// EN: Subscribe to GameFlow state changes (conditional on instance property, default true)
	// ES: Suscribirse a cambios de estado de GameFlow (condicional en propiedad de instancia, default true)
	if (bListenToGameFlowChanges)
	{
		ListenForPGXMessage<FPGXBridgeGameFlowChanged>(
			TAG_PGX_Bridge_GameFlow_StateChanged,
			[this](FGameplayTag, const FPGXBridgeGameFlowChanged& Msg)
			{ OnGameFlowStateChanged(Msg.OldState, Msg.NewState); });
	}
}

void UPGXGameInstance::ApplyPlatformProfile_Implementation()
{
	// EN: Override in derived classes to apply platform-specific settings
	// ES: Sobreescribir en clases derivadas para aplicar configuracion de plataforma
}

void UPGXGameInstance::OnRegistryReady_Implementation()
{
	// EN: Override in derived classes for post-registry initialization
	// ES: Sobreescribir en clases derivadas para inicializacion post-registry
	UE_LOG(LogPGXBase, Verbose, TEXT("GameInstance [%s]: DataRegistry ready"), *GetPGXDisplayName());
}

void UPGXGameInstance::FirePGXEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid()) { return; }
	UPGXEventHandlerSubsystem* EventHandler = UPGXEventHandlerSubsystem::Get(this);
	if (!IsValid(EventHandler)) { return; }
	EventHandler->ResolveAndExecute(EventTag, this, FInstancedStruct());
}

UPGXDataAsset* UPGXGameInstance::QueryPGXRegistry(FGameplayTag DatabaseTag, FGameplayTag EntryTag) const
{
	if (!DatabaseTag.IsValid() || !EntryTag.IsValid()) { return nullptr; }
	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!IsValid(Registry)) { return nullptr; }
	return Registry->ResolveAsset(DatabaseTag, EntryTag);
}

void UPGXGameInstance::OnGameFlowStateChanged_Implementation(FGameplayTag OldState, FGameplayTag NewState)
{
	// EN: Override in derived classes to react to GameFlow state transitions
	// ES: Sobreescribir en clases derivadas para reaccionar a transiciones de GameFlow
}

void UPGXGameInstance::Shutdown()
{
	// EN: Log BEFORE Super to ensure subsystems are still accessible (Gotcha #4)
	// ES: Log ANTES de Super para asegurar que los subsistemas aun estan accesibles
	UE_LOG(LogPGXBase, Log, TEXT("GameInstance [%s]: Shutdown"), *GetPGXDisplayName());
	PGXBaseMessaging::UnregisterAll(MessageListenerHandles);
	Super::Shutdown();
}
