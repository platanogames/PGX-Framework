// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAudioBlueprintLibrary.h"
#include "PGXAudioSubsystem.h"
#include "Mix/PGXAudioMixSubsystem.h"
#include "Manager/PGXMusicManager.h"
#include "Manager/PGXDialogueManager.h"
#include "Manager/PGXSoundPool.h"
#include "Data/PGXSoundDefinition.h"
#include "Data/PGXMusicPlaylist.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

// EN: Blueprint Function Library implementation — wraps subsystem methods with WorldContext
// ES: Implementacion de Libreria de Funciones Blueprint — envuelve metodos del subsistema con WorldContext

// ── Helpers ──

UPGXAudioSubsystem* UPGXAudioBlueprintLibrary::GetAudioSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXAudioSubsystem>() : nullptr;
}

UPGXAudioMixSubsystem* UPGXAudioBlueprintLibrary::GetMixSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	return World ? World->GetSubsystem<UPGXAudioMixSubsystem>() : nullptr;
}

// ══════════════════════════════════════════════
// Volume
// ══════════════════════════════════════════════

void UPGXAudioBlueprintLibrary::SetChannelVolume(const UObject* WorldContextObject, FGameplayTag ChannelTag, float Volume)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->SetChannelVolume(ChannelTag, Volume);
	}
}

float UPGXAudioBlueprintLibrary::GetChannelVolume(const UObject* WorldContextObject, FGameplayTag ChannelTag)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetChannelVolume(ChannelTag);
	}
	return 1.0f;
}

void UPGXAudioBlueprintLibrary::SetChannelMuted(const UObject* WorldContextObject, FGameplayTag ChannelTag, bool bMuted)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->SetChannelMuted(ChannelTag, bMuted);
	}
}

bool UPGXAudioBlueprintLibrary::IsChannelMuted(const UObject* WorldContextObject, FGameplayTag ChannelTag)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->IsChannelMuted(ChannelTag);
	}
	return false;
}

void UPGXAudioBlueprintLibrary::SetMuteAll(const UObject* WorldContextObject, bool bMuted)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->SetMuteAll(bMuted);
	}
}

bool UPGXAudioBlueprintLibrary::IsMuteAll(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->IsMuteAll();
	}
	return false;
}

TArray<FPGXAudioChannelSnapshot> UPGXAudioBlueprintLibrary::GetAllChannelStates(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetAllChannelStates();
	}
	return {};
}

// ══════════════════════════════════════════════
// Playback
// ══════════════════════════════════════════════

FPGXSoundHandle UPGXAudioBlueprintLibrary::PlaySound2D(const UObject* WorldContextObject, USoundBase* Sound, FPGXAudioPlayParams Params)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->PlaySound2D(Sound, Params);
	}
	return FPGXSoundHandle::Invalid();
}

FPGXSoundHandle UPGXAudioBlueprintLibrary::PlaySoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound,
	FVector Location, FRotator Rotation, FPGXAudioPlayParams Params)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->PlaySoundAtLocation(Sound, Location, Rotation, Params);
	}
	return FPGXSoundHandle::Invalid();
}

FPGXSoundHandle UPGXAudioBlueprintLibrary::PlaySoundAttached(const UObject* WorldContextObject, USoundBase* Sound,
	USceneComponent* AttachComponent, FName AttachPointName, FPGXAudioPlayParams Params)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->PlaySoundAttached(Sound, AttachComponent, AttachPointName, Params);
	}
	return FPGXSoundHandle::Invalid();
}

void UPGXAudioBlueprintLibrary::StopSound(const UObject* WorldContextObject, FPGXSoundHandle Handle, float FadeOutDuration)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->StopSound(Handle, FadeOutDuration);
	}
}

void UPGXAudioBlueprintLibrary::StopAllSounds(const UObject* WorldContextObject, float FadeOutDuration)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->StopAllSounds(FadeOutDuration);
	}
}

// ══════════════════════════════════════════════
// Resolution
// ══════════════════════════════════════════════

USoundBase* UPGXAudioBlueprintLibrary::ResolveSound(const UObject* WorldContextObject, const UPGXSoundDefinition* Definition,
	const FGameplayTagContainer& ContextTags)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->ResolveSound(Definition, ContextTags);
	}
	return nullptr;
}

FPGXSoundHandle UPGXAudioBlueprintLibrary::PlayResolved(const UObject* WorldContextObject, const UPGXSoundDefinition* Definition,
	FPGXAudioPlayParams Params)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->PlayResolved(Definition, Params);
	}
	return FPGXSoundHandle::Invalid();
}

// ══════════════════════════════════════════════
// Music
// ══════════════════════════════════════════════

void UPGXAudioBlueprintLibrary::PlayMusic(const UObject* WorldContextObject, USoundBase* Music, float FadeInDuration)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->PlayMusic(Music, FadeInDuration);
	}
}

void UPGXAudioBlueprintLibrary::StopMusic(const UObject* WorldContextObject, float FadeOutDuration)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->StopMusic(FadeOutDuration);
	}
}

void UPGXAudioBlueprintLibrary::PauseMusic(const UObject* WorldContextObject)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->PauseMusic();
	}
}

void UPGXAudioBlueprintLibrary::ResumeMusic(const UObject* WorldContextObject)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->ResumeMusic();
	}
}

void UPGXAudioBlueprintLibrary::CrossfadeTo(const UObject* WorldContextObject, USoundBase* NewMusic, float CrossfadeDuration)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->CrossfadeTo(NewMusic, CrossfadeDuration);
	}
}

void UPGXAudioBlueprintLibrary::PlayPlaylist(const UObject* WorldContextObject, const UPGXMusicPlaylist* Playlist)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->PlayPlaylist(Playlist);
	}
}

void UPGXAudioBlueprintLibrary::SetMusicState(const UObject* WorldContextObject, FGameplayTag StateTag)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->SetMusicState(StateTag);
	}
}

FGameplayTag UPGXAudioBlueprintLibrary::GetMusicState(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetMusicState();
	}
	return FGameplayTag();
}

FString UPGXAudioBlueprintLibrary::GetCurrentTrackName(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetCurrentTrackName();
	}
	return TEXT("");
}

// ══════════════════════════════════════════════
// Dialogue
// ══════════════════════════════════════════════

bool UPGXAudioBlueprintLibrary::QueueDialogue(const UObject* WorldContextObject, USoundBase* Sound, FText SubtitleText,
	float Duration, FGameplayTag SpeakerTag, FGameplayTag PriorityTag, EPGXDialogueInterruptPolicy Policy)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->QueueDialogue(Sound, SubtitleText, Duration, SpeakerTag, PriorityTag, Policy);
	}
	return false;
}

void UPGXAudioBlueprintLibrary::StopDialogue(const UObject* WorldContextObject, float FadeOutDuration)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->StopDialogue(FadeOutDuration);
	}
}

void UPGXAudioBlueprintLibrary::ClearDialogueQueue(const UObject* WorldContextObject)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		Sub->ClearDialogueQueue();
	}
}

int32 UPGXAudioBlueprintLibrary::GetDialogueQueueCount(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetDialogueQueueCount();
	}
	return 0;
}

bool UPGXAudioBlueprintLibrary::IsDialoguePlaying(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->IsDialoguePlaying();
	}
	return false;
}

// ══════════════════════════════════════════════
// Backend
// ══════════════════════════════════════════════

EPGXAudioBackendType UPGXAudioBlueprintLibrary::GetActiveBackendType(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetActiveBackendType();
	}
	return EPGXAudioBackendType::Auto;
}

bool UPGXAudioBlueprintLibrary::SwitchBackend(const UObject* WorldContextObject, EPGXAudioBackendType NewType)
{
	if (UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->SwitchBackend(NewType);
	}
	return false;
}

FString UPGXAudioBlueprintLibrary::GetBackendStatusText(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetBackendStatusText();
	}
	return TEXT("No subsystem");
}

// ══════════════════════════════════════════════
// Mix/Ducking — delegates to MixSubsystem
// ══════════════════════════════════════════════

FPGXMixLayerState UPGXAudioBlueprintLibrary::GetMixLayerState(const UObject* WorldContextObject, EPGXMixLayer Layer)
{
	if (const UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		return Mix->GetMixLayerState(Layer);
	}
	return FPGXMixLayerState();
}

TArray<FPGXMixLayerState> UPGXAudioBlueprintLibrary::GetAllMixLayerStates(const UObject* WorldContextObject)
{
	if (const UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		return Mix->GetAllMixLayerStates();
	}
	return {};
}

TArray<FPGXDuckingRule> UPGXAudioBlueprintLibrary::GetActiveDuckingRules(const UObject* WorldContextObject)
{
	if (const UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		return Mix->GetActiveDuckingRules();
	}
	return {};
}

void UPGXAudioBlueprintLibrary::SetDuckingEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	if (UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		Mix->SetDuckingEnabled(bEnabled);
	}
}

bool UPGXAudioBlueprintLibrary::IsDuckingEnabled(const UObject* WorldContextObject)
{
	if (const UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		return Mix->IsDuckingEnabled();
	}
	return false;
}

void UPGXAudioBlueprintLibrary::SetHDRAudioEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	if (UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		Mix->SetHDRAudioEnabled(bEnabled);
	}
}

bool UPGXAudioBlueprintLibrary::IsHDRAudioEnabled(const UObject* WorldContextObject)
{
	if (const UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		return Mix->IsHDRAudioEnabled();
	}
	return false;
}

void UPGXAudioBlueprintLibrary::SetHRTFEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	if (UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		Mix->SetHRTFEnabled(bEnabled);
	}
}

bool UPGXAudioBlueprintLibrary::IsHRTFEnabled(const UObject* WorldContextObject)
{
	if (const UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		return Mix->IsHRTFEnabled();
	}
	return false;
}

// ══════════════════════════════════════════════
// Device — delegates to MixSubsystem
// ══════════════════════════════════════════════

TArray<FString> UPGXAudioBlueprintLibrary::GetAvailableAudioDevices(const UObject* WorldContextObject)
{
	if (const UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		return Mix->GetAvailableAudioDevices();
	}
	return {};
}

FString UPGXAudioBlueprintLibrary::GetCurrentAudioDevice(const UObject* WorldContextObject)
{
	if (const UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		return Mix->GetCurrentAudioDevice();
	}
	return TEXT("Default");
}

bool UPGXAudioBlueprintLibrary::SetAudioDevice(const UObject* WorldContextObject, const FString& DeviceName)
{
	if (UPGXAudioMixSubsystem* Mix = GetMixSubsystem(WorldContextObject))
	{
		return Mix->SetAudioDevice(DeviceName);
	}
	return false;
}

// ══════════════════════════════════════════════
// Query
// ══════════════════════════════════════════════

bool UPGXAudioBlueprintLibrary::IsAudioSystemReady(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->IsInitialized();
	}
	return false;
}

int32 UPGXAudioBlueprintLibrary::GetActiveSoundCount(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetActiveSoundCount();
	}
	return 0;
}

TArray<FPGXActiveSoundInfo> UPGXAudioBlueprintLibrary::GetActiveSounds(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetActiveSounds();
	}
	return {};
}

FPGXAudioSystemSnapshot UPGXAudioBlueprintLibrary::GetAudioSnapshot(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetAudioSnapshot();
	}
	return FPGXAudioSystemSnapshot();
}

FPGXSoundPoolStats UPGXAudioBlueprintLibrary::GetPoolStatistics(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetPoolStatistics();
	}
	return FPGXSoundPoolStats();
}

FPGXAudioMemoryInfo UPGXAudioBlueprintLibrary::GetMemoryEstimate(const UObject* WorldContextObject)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetMemoryEstimate();
	}
	return FPGXAudioMemoryInfo();
}

TArray<FPGXAudioEventRecord> UPGXAudioBlueprintLibrary::GetEventHistory(const UObject* WorldContextObject, int32 MaxCount)
{
	if (const UPGXAudioSubsystem* Sub = GetAudioSubsystem(WorldContextObject))
	{
		return Sub->GetEventHistory(MaxCount);
	}
	return {};
}
