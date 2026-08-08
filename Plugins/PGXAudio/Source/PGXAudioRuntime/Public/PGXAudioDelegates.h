// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXAudioTypes.h"
#include "PGXAudioDelegates.generated.h"

// ============================================================================
// EN: PGX Audio delegate declarations.
//     12 delegates: each has a Dynamic (Blueprint) + Native (C++) variant.
//     Dynamic delegates use DECLARE_DYNAMIC_MULTICAST_DELEGATE_*.
//     Native delegates use DECLARE_MULTICAST_DELEGATE_*.
//
// ES: Declaraciones de delegados de PGX Audio.
//     12 delegados: cada uno tiene variante Dynamic (Blueprint) + Native (C++).
//     Delegados dynamic usan DECLARE_DYNAMIC_MULTICAST_DELEGATE_*.
//     Delegados native usan DECLARE_MULTICAST_DELEGATE_*.
// ============================================================================

// ── 1. OnAudioSystemReady ──
// EN: Fired when the audio subsystem completes initialization / ES: Se dispara cuando el subsistema de audio completa la inicializacion
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPGXAudioSystemReady);
DECLARE_MULTICAST_DELEGATE(FOnPGXAudioSystemReadyNative);

// ── 2. OnSoundPlayed ──
// EN: Fired when a sound starts playing / ES: Se dispara cuando un sonido empieza a reproducirse
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPGXSoundPlayed,
	FPGXSoundHandle, Handle,
	FGameplayTag, ProfileTag
);
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnPGXSoundPlayedNative,
	FPGXSoundHandle /*Handle*/,
	FGameplayTag /*ProfileTag*/
);

// ── 3. OnSoundStopped ──
// EN: Fired when a sound stops (any reason) / ES: Se dispara cuando un sonido se detiene (cualquier razon)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPGXSoundStopped,
	FPGXSoundHandle, Handle,
	EPGXSoundStopReason, Reason
);
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnPGXSoundStoppedNative,
	FPGXSoundHandle /*Handle*/,
	EPGXSoundStopReason /*Reason*/
);

// ── 4. OnChannelVolumeChanged ──
// EN: Fired when a channel volume changes / ES: Se dispara cuando cambia el volumen de un canal
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXChannelVolumeChanged,
	FGameplayTag, ChannelTag,
	float, OldVolume,
	float, NewVolume
);
DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXChannelVolumeChangedNative,
	FGameplayTag /*ChannelTag*/,
	float /*OldVolume*/,
	float /*NewVolume*/
);

// ── 5. OnMuteChanged ──
// EN: Fired when a channel mute state changes / ES: Se dispara cuando cambia el estado de mute de un canal
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPGXMuteChanged,
	FGameplayTag, ChannelTag,
	bool, bMuted
);
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnPGXMuteChangedNative,
	FGameplayTag /*ChannelTag*/,
	bool /*bMuted*/
);

// ── 6. OnMusicStateChanged ──
// EN: Fired when the music manager changes state / ES: Se dispara cuando el music manager cambia de estado
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPGXMusicStateChanged,
	EPGXMusicState, OldState,
	EPGXMusicState, NewState
);
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnPGXMusicStateChangedNative,
	EPGXMusicState /*OldState*/,
	EPGXMusicState /*NewState*/
);

// ── 7. OnBackendSwitched ──
// EN: Fired when the audio backend is hot-switched / ES: Se dispara cuando se hace hot-switch del backend de audio
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPGXBackendSwitched,
	EPGXAudioBackendType, OldType,
	EPGXAudioBackendType, NewType
);
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnPGXBackendSwitchedNative,
	EPGXAudioBackendType /*OldType*/,
	EPGXAudioBackendType /*NewType*/
);

// ── 8. OnPoolExhausted ──
// EN: Fired when sound pool runs out of instances / ES: Se dispara cuando el pool de sonidos se queda sin instancias
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPGXPoolExhausted,
	int32, RequestedCount,
	int32, AvailableCount
);
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnPGXPoolExhaustedNative,
	int32 /*RequestedCount*/,
	int32 /*AvailableCount*/
);

// ── 9. OnMixLayerChanged ──
// EN: Fired when a mix layer is activated or deactivated / ES: Se dispara cuando una capa de mezcla se activa o desactiva
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPGXMixLayerChanged,
	EPGXMixLayer, Layer,
	bool, bActive
);
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnPGXMixLayerChangedNative,
	EPGXMixLayer /*Layer*/,
	bool /*bActive*/
);

// ── 10. OnDuckingChanged ──
// EN: Fired when a ducking rule activates or deactivates / ES: Se dispara cuando una regla de ducking se activa o desactiva
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXDuckingChanged,
	FGameplayTag, TriggerChannelTag,
	FGameplayTag, TargetChannelTag,
	float, DuckVolume
);
DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXDuckingChangedNative,
	FGameplayTag /*TriggerChannelTag*/,
	FGameplayTag /*TargetChannelTag*/,
	float /*DuckVolume*/
);

// ── 11. OnDialogueSubtitle ──
// EN: Fired when dialogue plays, carrying subtitle data for UI / ES: Se dispara cuando se reproduce dialogo, llevando datos de subtitulo para UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXDialogueSubtitle,
	FText, SubtitleText,
	float, Duration,
	FGameplayTag, SpeakerTag
);
DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXDialogueSubtitleNative,
	FText /*SubtitleText*/,
	float /*Duration*/,
	FGameplayTag /*SpeakerTag*/
);

// ── 12. OnMixSubsystemReady ──
// EN: Fired when the WorldSubsystem (mix) completes initialization / ES: Se dispara cuando el WorldSubsystem (mix) completa la inicializacion
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPGXMixSubsystemReady);
DECLARE_MULTICAST_DELEGATE(FOnPGXMixSubsystemReadyNative);

// ============================================================================
// EN: Dummy USTRUCT to satisfy UHT — this file declares delegates via macros
//     that need a GENERATED_BODY() somewhere in the translation unit.
// ES: USTRUCT dummy para satisfacer UHT — este archivo declara delegados via macros
//     que necesitan un GENERATED_BODY() en alguna parte de la unidad de traduccion.
// ============================================================================
USTRUCT()
struct FPGXAudioDelegatesRegistrar
{
	GENERATED_BODY()
};
