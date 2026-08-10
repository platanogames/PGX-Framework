// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Component/PGXAudioComponent.h"
#include "PGXAudioSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "Data/PGXSoundDefinition.h"
#include "PGXAudioLog.h"
#include "Tags/PGXAudioTags.h"
#include "Engine/World.h"

// EN: PGX Audio Component implementation
// ES: Implementacion del componente de audio PGX

UPGXAudioComponent::UPGXAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPGXAudioComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPGXAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAll(0.0f);
	Super::EndPlay(EndPlayReason);
}

FPGXSoundHandle UPGXAudioComponent::PlaySound(FGameplayTag SoundTag, const FGameplayTagContainer& ContextTags)
{
	UPGXAudioSubsystem* AudioSub = GetAudioSubsystem();
	if (!AudioSub)
	{
		return FPGXSoundHandle::Invalid();
	}

	// EN: Resolve SoundDefinition by tag from cache / ES: Resolver SoundDefinition por tag desde cache
	const UPGXSoundDefinition* Definition = AudioSub->FindDefinitionByTag(SoundTag);
	if (!Definition)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlaySound — No SoundDefinition found for tag: %s"), *SoundTag.ToString());
		return FPGXSoundHandle::Invalid();
	}

	// EN: Build play params from component defaults / ES: Construir params de play desde defaults del componente
	FPGXAudioPlayParams Params;
	Params.ChannelTag = DefaultChannelTag.IsValid() ? DefaultChannelTag : Definition->DefaultChannelTag;
	Params.ProfileTag = DefaultProfileTag.IsValid() ? DefaultProfileTag : Definition->DefaultProfileTag;
	Params.ContextTags = ContextTags;

	// EN: Resolve and play via the subsystem definition pipeline
	// ES: Resolver y reproducir via el pipeline de definiciones del subsistema
	FPGXSoundHandle Handle = FPGXSoundHandle::Invalid();

	if (bPlayAtLocation && GetOwner())
	{
		// EN: Resolve to USoundBase, then play at location / ES: Resolver a USoundBase, luego reproducir en ubicacion
		USoundBase* ResolvedSound = AudioSub->ResolveSound(Definition, ContextTags);
		if (ResolvedSound)
		{
			const FVector Location = GetOwner()->GetActorLocation();
			const FRotator Rotation = GetOwner()->GetActorRotation();
			Handle = AudioSub->PlaySoundAtLocation(ResolvedSound, Location, Rotation, Params);
		}
	}
	else
	{
		Handle = AudioSub->PlayResolved(Definition, Params);
	}

	if (Handle.IsValid())
	{
		ActiveHandles.Add(Handle);
	}

	return Handle;
}

FPGXSoundHandle UPGXAudioComponent::PlaySoundDirect(USoundBase* Sound)
{
	if (!Sound)
	{
		return FPGXSoundHandle::Invalid();
	}

	UPGXAudioSubsystem* AudioSub = GetAudioSubsystem();
	if (!AudioSub)
	{
		return FPGXSoundHandle::Invalid();
	}

	FPGXAudioPlayParams Params;
	Params.ChannelTag = DefaultChannelTag;
	Params.ProfileTag = DefaultProfileTag;

	FPGXSoundHandle Handle = FPGXSoundHandle::Invalid();

	if (bPlayAtLocation && GetOwner())
	{
		const FVector Location = GetOwner()->GetActorLocation();
		const FRotator Rotation = GetOwner()->GetActorRotation();
		Handle = AudioSub->PlaySoundAtLocation(Sound, Location, Rotation, Params);
	}
	else
	{
		Handle = AudioSub->PlaySound2D(Sound, Params);
	}

	if (Handle.IsValid())
	{
		ActiveHandles.Add(Handle);
	}

	return Handle;
}

void UPGXAudioComponent::StopAll(float FadeOutDuration)
{
	UPGXAudioSubsystem* AudioSub = GetAudioSubsystem();
	if (!AudioSub)
	{
		return;
	}

	for (const FPGXSoundHandle& Handle : ActiveHandles)
	{
		AudioSub->StopSound(Handle, FadeOutDuration);
	}
	ActiveHandles.Empty();
}

void UPGXAudioComponent::SetProfile(FGameplayTag ProfileTag)
{
	DefaultProfileTag = ProfileTag;
}

UPGXAudioSubsystem* UPGXAudioComponent::GetAudioSubsystem() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UPGXAudioSubsystem>();
}
