// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "PGXAudioTypes.h"
#include "PGXAudioBlueprintLibrary.generated.h"

class UPGXAudioSubsystem;
class UPGXSoundDefinition;
class UPGXMusicPlaylist;
class UPGXAudioMixSubsystem;

/**
 * EN: Blueprint Function Library for PGX Audio system.
 *     Provides static access to all audio functionality with WorldContext.
 *     Wraps AudioSubsystem + MixSubsystem for clean Blueprint UX.
 *     ~50 methods across 9 categories.
 *
 * ES: Libreria de Funciones Blueprint para el sistema PGX Audio.
 *     Provee acceso estatico a toda la funcionalidad de audio con WorldContext.
 *     Envuelve AudioSubsystem + MixSubsystem para UX Blueprint limpio.
 *     ~50 metodos en 9 categorias.
 */
UCLASS()
class PGXAUDIORUNTIME_API UPGXAudioBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ══════════════════════════════════════════════
	// Volume (7)
	// ══════════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Volume", meta = (WorldContext = "WorldContextObject"))
	static void SetChannelVolume(const UObject* WorldContextObject, FGameplayTag ChannelTag, float Volume);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Volume", meta = (WorldContext = "WorldContextObject"))
	static float GetChannelVolume(const UObject* WorldContextObject, FGameplayTag ChannelTag);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Volume", meta = (WorldContext = "WorldContextObject"))
	static void SetChannelMuted(const UObject* WorldContextObject, FGameplayTag ChannelTag, bool bMuted);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Volume", meta = (WorldContext = "WorldContextObject"))
	static bool IsChannelMuted(const UObject* WorldContextObject, FGameplayTag ChannelTag);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Volume", meta = (WorldContext = "WorldContextObject"))
	static void SetMuteAll(const UObject* WorldContextObject, bool bMuted);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Volume", meta = (WorldContext = "WorldContextObject"))
	static bool IsMuteAll(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Volume", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXAudioChannelSnapshot> GetAllChannelStates(const UObject* WorldContextObject);

	// ══════════════════════════════════════════════
	// Playback (5)
	// ══════════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback", meta = (WorldContext = "WorldContextObject"))
	static FPGXSoundHandle PlaySound2D(const UObject* WorldContextObject, USoundBase* Sound, FPGXAudioPlayParams Params);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback", meta = (WorldContext = "WorldContextObject"))
	static FPGXSoundHandle PlaySoundAtLocation(const UObject* WorldContextObject, USoundBase* Sound, FVector Location, FRotator Rotation, FPGXAudioPlayParams Params);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback", meta = (WorldContext = "WorldContextObject"))
	static FPGXSoundHandle PlaySoundAttached(const UObject* WorldContextObject, USoundBase* Sound, USceneComponent* AttachComponent, FName AttachPointName, FPGXAudioPlayParams Params);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback", meta = (WorldContext = "WorldContextObject"))
	static void StopSound(const UObject* WorldContextObject, FPGXSoundHandle Handle, float FadeOutDuration = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Playback", meta = (WorldContext = "WorldContextObject"))
	static void StopAllSounds(const UObject* WorldContextObject, float FadeOutDuration = 0.0f);

	// ══════════════════════════════════════════════
	// Resolution (2)
	// ══════════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Resolution", meta = (WorldContext = "WorldContextObject"))
	static USoundBase* ResolveSound(const UObject* WorldContextObject, const UPGXSoundDefinition* Definition, const FGameplayTagContainer& ContextTags);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Resolution", meta = (WorldContext = "WorldContextObject"))
	static FPGXSoundHandle PlayResolved(const UObject* WorldContextObject, const UPGXSoundDefinition* Definition, FPGXAudioPlayParams Params);

	// ══════════════════════════════════════════════
	// Music (9)
	// ══════════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music", meta = (WorldContext = "WorldContextObject"))
	static void PlayMusic(const UObject* WorldContextObject, USoundBase* Music, float FadeInDuration = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music", meta = (WorldContext = "WorldContextObject"))
	static void StopMusic(const UObject* WorldContextObject, float FadeOutDuration = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music", meta = (WorldContext = "WorldContextObject"))
	static void PauseMusic(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music", meta = (WorldContext = "WorldContextObject"))
	static void ResumeMusic(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music", meta = (WorldContext = "WorldContextObject"))
	static void CrossfadeTo(const UObject* WorldContextObject, USoundBase* NewMusic, float CrossfadeDuration = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music", meta = (WorldContext = "WorldContextObject"))
	static void PlayPlaylist(const UObject* WorldContextObject, const UPGXMusicPlaylist* Playlist);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Music", meta = (WorldContext = "WorldContextObject"))
	static void SetMusicState(const UObject* WorldContextObject, FGameplayTag StateTag);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Music", meta = (WorldContext = "WorldContextObject"))
	static FGameplayTag GetMusicState(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Music", meta = (WorldContext = "WorldContextObject"))
	static FString GetCurrentTrackName(const UObject* WorldContextObject);

	// ══════════════════════════════════════════════
	// Dialogue (5)
	// ══════════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Dialogue", meta = (WorldContext = "WorldContextObject"))
	static bool QueueDialogue(const UObject* WorldContextObject, USoundBase* Sound, FText SubtitleText, float Duration,
		FGameplayTag SpeakerTag, FGameplayTag PriorityTag, EPGXDialogueInterruptPolicy Policy = EPGXDialogueInterruptPolicy::QueueBehind);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Dialogue", meta = (WorldContext = "WorldContextObject"))
	static void StopDialogue(const UObject* WorldContextObject, float FadeOutDuration = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Dialogue", meta = (WorldContext = "WorldContextObject"))
	static void ClearDialogueQueue(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Dialogue", meta = (WorldContext = "WorldContextObject"))
	static int32 GetDialogueQueueCount(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Dialogue", meta = (WorldContext = "WorldContextObject"))
	static bool IsDialoguePlaying(const UObject* WorldContextObject);

	// ══════════════════════════════════════════════
	// Backend (3)
	// ══════════════════════════════════════════════

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Backend", meta = (WorldContext = "WorldContextObject"))
	static EPGXAudioBackendType GetActiveBackendType(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Backend", meta = (WorldContext = "WorldContextObject"))
	static bool SwitchBackend(const UObject* WorldContextObject, EPGXAudioBackendType NewType);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Backend", meta = (WorldContext = "WorldContextObject"))
	static FString GetBackendStatusText(const UObject* WorldContextObject);

	// ══════════════════════════════════════════════
	// Mix/Ducking (10) — delegates to MixSubsystem (WorldSubsystem)
	// ══════════════════════════════════════════════

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Mix", meta = (WorldContext = "WorldContextObject"))
	static FPGXMixLayerState GetMixLayerState(const UObject* WorldContextObject, EPGXMixLayer Layer);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Mix", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXMixLayerState> GetAllMixLayerStates(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Ducking", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXDuckingRule> GetActiveDuckingRules(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Ducking", meta = (WorldContext = "WorldContextObject"))
	static void SetDuckingEnabled(const UObject* WorldContextObject, bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Ducking", meta = (WorldContext = "WorldContextObject"))
	static bool IsDuckingEnabled(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|HDR", meta = (WorldContext = "WorldContextObject"))
	static void SetHDRAudioEnabled(const UObject* WorldContextObject, bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|HDR", meta = (WorldContext = "WorldContextObject"))
	static bool IsHDRAudioEnabled(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|HRTF", meta = (WorldContext = "WorldContextObject"))
	static void SetHRTFEnabled(const UObject* WorldContextObject, bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|HRTF", meta = (WorldContext = "WorldContextObject"))
	static bool IsHRTFEnabled(const UObject* WorldContextObject);

	// ══════════════════════════════════════════════
	// Device (3)
	// ══════════════════════════════════════════════

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Device", meta = (WorldContext = "WorldContextObject"))
	static TArray<FString> GetAvailableAudioDevices(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Device", meta = (WorldContext = "WorldContextObject"))
	static FString GetCurrentAudioDevice(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Device", meta = (WorldContext = "WorldContextObject"))
	static bool SetAudioDevice(const UObject* WorldContextObject, const FString& DeviceName);

	// ══════════════════════════════════════════════
	// Query (7)
	// ══════════════════════════════════════════════

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Query", meta = (WorldContext = "WorldContextObject"))
	static bool IsAudioSystemReady(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Query", meta = (WorldContext = "WorldContextObject"))
	static int32 GetActiveSoundCount(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXActiveSoundInfo> GetActiveSounds(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Query", meta = (WorldContext = "WorldContextObject"))
	static FPGXAudioSystemSnapshot GetAudioSnapshot(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Query", meta = (WorldContext = "WorldContextObject"))
	static FPGXSoundPoolStats GetPoolStatistics(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Query", meta = (WorldContext = "WorldContextObject"))
	static FPGXAudioMemoryInfo GetMemoryEstimate(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "PGX|Audio|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXAudioEventRecord> GetEventHistory(const UObject* WorldContextObject, int32 MaxCount = 50);

private:
	/** EN: Helper to get AudioSubsystem from WorldContext / ES: Helper para obtener AudioSubsystem desde WorldContext */
	static UPGXAudioSubsystem* GetAudioSubsystem(const UObject* WorldContextObject);

	/** EN: Helper to get MixSubsystem from WorldContext / ES: Helper para obtener MixSubsystem desde WorldContext */
	static UPGXAudioMixSubsystem* GetMixSubsystem(const UObject* WorldContextObject);
};
