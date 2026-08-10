// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Base/PGXBaseComponent.h"
#include "Base/PGXBaseLog.h"
#include "EventHandler/PGXEventHandlerSubsystem.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "Registry/PGXDataRegistrySubsystem.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/Tags/PGXBridgeTags.h"

// EN: Base component for all PGX components implementation
// ES: Implementacion del componente base para todos los componentes PGX

UPGXBaseComponent::UPGXBaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FString UPGXBaseComponent::GetPGXDisplayName_Implementation() const
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		return FString::Printf(TEXT("%s.%s"), *Owner->GetName(), *GetName());
	}
	return GetName();
}

void UPGXBaseComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogPGXBase, Verbose, TEXT("Component [%s]: BeginPlay"), *GetPGXDisplayName());

	// EN: Wire save bridge from instance properties
	// ES: Conectar bridge save desde propiedades de instancia
	if (bParticipateInSave && SaveDomain.IsValid())
	{
		SetupSaveBridge(SaveDomain);
	}
}

void UPGXBaseComponent::FirePGXEvent(FGameplayTag EventTag)
{
	if (!EventTag.IsValid()) { return; }
	UPGXEventHandlerSubsystem* EventHandler = UPGXEventHandlerSubsystem::Get(this);
	if (!IsValid(EventHandler)) { return; }
	EventHandler->ResolveAndExecute(EventTag, GetOwner(), FInstancedStruct());
}

UPGXDataAsset* UPGXBaseComponent::QueryPGXRegistry(FGameplayTag DatabaseTag, FGameplayTag EntryTag) const
{
	if (!DatabaseTag.IsValid() || !EntryTag.IsValid()) { return nullptr; }
	UPGXDataRegistrySubsystem* Registry = UPGXDataRegistrySubsystem::GetCached();
	if (!IsValid(Registry)) { return nullptr; }
	return Registry->ResolveAsset(DatabaseTag, EntryTag);
}

// ─── L2 Bridge Implementations (Save) ───

void UPGXBaseComponent::SetupSaveBridge(FGameplayTag InSaveDomain)
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

void UPGXBaseComponent::TeardownSaveBridge()
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

void UPGXBaseComponent::OnPGXPreSave_Implementation(const FPGXBridgeSaveNotification& Notification) {}
void UPGXBaseComponent::OnPGXPostLoad_Implementation(const FPGXBridgeSaveNotification& Notification) {}

void UPGXBaseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogPGXBase, Verbose, TEXT("Component [%s]: EndPlay (%s)"),
		*GetPGXDisplayName(), *UEnum::GetValueAsString(EndPlayReason));
	TeardownSaveBridge();
	PGXBaseMessaging::UnregisterAll(MessageListenerHandles);
	Super::EndPlay(EndPlayReason);
}
