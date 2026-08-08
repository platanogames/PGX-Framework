// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "PGXAudioTypes.h"
#include "PGXMusicManager.generated.h"

// Forward declarations
class UPGXAudioSubsystem;
class UAudioComponent;
class UPGXMusicPlaylist;

/**
 * EN: Music management system — handles playlists, crossfade, stingers, state-driven music.
 *     Owned by UPGXAudioSubsystem. Uses UAudioComponent for playback.
 *     State machine: Idle → Playing → CrossFading → Paused → Stopping → Idle.
 *
 * ES: Sistema de gestion musical — maneja playlists, crossfade, stingers, musica basada en estado.
 *     Owned por UPGXAudioSubsystem. Usa UAudioComponent para reproduccion.
 *     Maquina de estados: Idle → Playing → CrossFading → Paused → Stopping → Idle.
 */
UCLASS()
class PGXAUDIORUNTIME_API UPGXMusicManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * EN: Initialize with owner subsystem reference.
	 * ES: Inicializar con referencia al subsistema propietario.
	 */
	void Initialize(UPGXAudioSubsystem* InOwner);

	/** EN: Shutdown and stop all music / ES: Apagar y detener toda la musica */
	void Deinitialize();

	// ── Playback ──

	/** EN: Play a music track / ES: Reproducir una pista musical */
	void PlayMusic(USoundBase* Music, float FadeInDuration = 0.0f);

	/** EN: Stop current music / ES: Detener musica actual */
	void StopMusic(float FadeOutDuration = 2.0f);

	/** EN: Pause current music / ES: Pausar musica actual */
	void PauseMusic();

	/** EN: Resume paused music / ES: Reanudar musica pausada */
	void ResumeMusic();

	/** EN: Crossfade to a new track / ES: Crossfade a una nueva pista */
	void CrossfadeTo(USoundBase* NewMusic, float CrossfadeDuration = 2.0f);

	// ── Playlist ──

	/** EN: Start playing a playlist / ES: Empezar a reproducir un playlist */
	void PlayPlaylist(const UPGXMusicPlaylist* Playlist);

	// ── State ──

	/** EN: Set music state tag (for state-driven music) / ES: Establecer tag de estado musical (para musica basada en estado) */
	void SetMusicState(FGameplayTag StateTag);

	/** EN: Get current music state tag / ES: Obtener tag de estado musical actual */
	FGameplayTag GetMusicState() const { return ActiveMusicStateTag; }

	/** EN: Get current playback state / ES: Obtener estado de reproduccion actual */
	EPGXMusicState GetState() const { return CurrentState; }

	/** EN: Get current track name / ES: Obtener nombre de pista actual */
	FString GetCurrentTrackName() const;

private:
	/** EN: Internal state transition / ES: Transicion de estado interno */
	void SetState(EPGXMusicState NewState);

	/** EN: Callback when track finishes / ES: Callback cuando termina una pista */
	UFUNCTION()
	void OnTrackFinished();

	/** EN: Callback when crossfade source finishes fading out / ES: Callback cuando la fuente de crossfade termina el fade out */
	UFUNCTION()
	void OnCrossfadeSourceFinished();

	/** EN: Advance to next playlist track / ES: Avanzar a la siguiente pista del playlist */
	void AdvancePlaylist();

	/** EN: Create an audio component for music playback / ES: Crear un componente de audio para reproduccion musical */
	UAudioComponent* CreateMusicComponent(USoundBase* Sound);

	/** EN: Owner subsystem reference / ES: Referencia al subsistema propietario */
	UPROPERTY()
	TObjectPtr<UPGXAudioSubsystem> OwnerSubsystem = nullptr;

	/** EN: Primary music audio component / ES: Componente de audio musical principal */
	UPROPERTY()
	TObjectPtr<UAudioComponent> CurrentComponent = nullptr;

	/** EN: Secondary component for crossfade / ES: Componente secundario para crossfade */
	UPROPERTY()
	TObjectPtr<UAudioComponent> CrossfadeComponent = nullptr;

	/** EN: Current playback state / ES: Estado de reproduccion actual */
	EPGXMusicState CurrentState = EPGXMusicState::Idle;

	/** EN: Active music state tag / ES: Tag de estado musical activo */
	FGameplayTag ActiveMusicStateTag;

	/** EN: Active playlist reference / ES: Referencia a playlist activo */
	TWeakObjectPtr<const UPGXMusicPlaylist> ActivePlaylist;

	/** EN: Current index in playlist / ES: Indice actual en playlist */
	int32 CurrentPlaylistIndex = -1;

	/** EN: Shuffled indices for shuffle mode / ES: Indices mezclados para modo shuffle */
	TArray<int32> ShuffledIndices;
};
