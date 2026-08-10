// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Manager/PGXMusicManager.h"
#include "PGXAudioSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXAudioLog.h"
#include "PGXAudioConfig.h"
#include "Data/PGXMusicPlaylist.h"
#include "Tags/PGXAudioTags.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// EN: Music management system implementation
// ES: Implementacion del sistema de gestion musical

void UPGXMusicManager::Initialize(UPGXAudioSubsystem* InOwner)
{
	OwnerSubsystem = InOwner;
	CurrentState = EPGXMusicState::Idle;
	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXMusicManager::Initialize — Music manager ready"));
}

void UPGXMusicManager::Deinitialize()
{
	StopMusic(0.0f);

	if (CurrentComponent)
	{
		CurrentComponent->DestroyComponent();
		CurrentComponent = nullptr;
	}

	if (CrossfadeComponent)
	{
		CrossfadeComponent->DestroyComponent();
		CrossfadeComponent = nullptr;
	}

	ActivePlaylist.Reset();
	OwnerSubsystem = nullptr;
	CurrentState = EPGXMusicState::Idle;

	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXMusicManager::Deinitialize — Music manager shutdown"));
}

void UPGXMusicManager::PlayMusic(USoundBase* Music, float FadeInDuration)
{
	if (!Music)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlayMusic — Null music asset"));
		return;
	}

	// EN: Destroy previous component to avoid orphaned audio / ES: Destruir componente previo para evitar audio huerfano
	if (CurrentComponent)
	{
		CurrentComponent->OnAudioFinished.RemoveDynamic(this, &UPGXMusicManager::OnTrackFinished);
		CurrentComponent->Stop();
		CurrentComponent->DestroyComponent();
		CurrentComponent = nullptr;
	}

	CurrentComponent = CreateMusicComponent(Music);
	if (!CurrentComponent)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlayMusic — Failed to create audio component"));
		return;
	}

	if (FadeInDuration > 0.0f)
	{
		CurrentComponent->FadeIn(FadeInDuration);
	}
	else
	{
		CurrentComponent->Play();
	}

	// EN: Bind completion / ES: Vincular completado
	CurrentComponent->OnAudioFinished.AddDynamic(this, &UPGXMusicManager::OnTrackFinished);

	SetState(EPGXMusicState::Playing);

	PGX_LOG_INFO(LogPGXAudio, TEXT("PlayMusic — Playing: %s (fade: %.1fs)"), *Music->GetName(), FadeInDuration);
}

void UPGXMusicManager::StopMusic(float FadeOutDuration)
{
	if (!CurrentComponent || !CurrentComponent->IsPlaying())
	{
		SetState(EPGXMusicState::Idle);
		return;
	}

	if (FadeOutDuration > 0.0f)
	{
		SetState(EPGXMusicState::Stopping);
		CurrentComponent->FadeOut(FadeOutDuration, 0.0f);
		PGX_LOG_INFO(LogPGXAudio, TEXT("StopMusic — Fading out (%.1fs)"), FadeOutDuration);
	}
	else
	{
		CurrentComponent->Stop();
		SetState(EPGXMusicState::Idle);
		PGX_LOG_INFO(LogPGXAudio, TEXT("StopMusic — Stopped immediately"));
	}

	ActivePlaylist.Reset();
	CurrentPlaylistIndex = -1;
}

void UPGXMusicManager::PauseMusic()
{
	if (CurrentComponent && CurrentComponent->IsPlaying())
	{
		CurrentComponent->SetPaused(true);
		SetState(EPGXMusicState::Paused);
		PGX_LOG_INFO(LogPGXAudio, TEXT("PauseMusic — Paused"));
	}
}

void UPGXMusicManager::ResumeMusic()
{
	if (CurrentComponent && CurrentState == EPGXMusicState::Paused)
	{
		CurrentComponent->SetPaused(false);
		SetState(EPGXMusicState::Playing);
		PGX_LOG_INFO(LogPGXAudio, TEXT("ResumeMusic — Resumed"));
	}
}

void UPGXMusicManager::CrossfadeTo(USoundBase* NewMusic, float CrossfadeDuration)
{
	if (!NewMusic)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("CrossfadeTo — Null music asset"));
		return;
	}

	// EN: Current becomes crossfade source (fading out) / ES: Actual se vuelve fuente de crossfade (fading out)
	if (CurrentComponent && CurrentComponent->IsPlaying())
	{
		CrossfadeComponent = CurrentComponent;
		CrossfadeComponent->OnAudioFinished.RemoveDynamic(this, &UPGXMusicManager::OnTrackFinished);
		CrossfadeComponent->OnAudioFinished.AddDynamic(this, &UPGXMusicManager::OnCrossfadeSourceFinished);
		CrossfadeComponent->FadeOut(CrossfadeDuration, 0.0f);
	}

	// EN: New music fades in / ES: Nueva musica con fade in
	CurrentComponent = CreateMusicComponent(NewMusic);
	if (CurrentComponent)
	{
		CurrentComponent->FadeIn(CrossfadeDuration);
		CurrentComponent->OnAudioFinished.AddDynamic(this, &UPGXMusicManager::OnTrackFinished);
		SetState(EPGXMusicState::CrossFading);

		PGX_LOG_INFO(LogPGXAudio, TEXT("CrossfadeTo — %s (%.1fs)"), *NewMusic->GetName(), CrossfadeDuration);
	}
}

void UPGXMusicManager::PlayPlaylist(const UPGXMusicPlaylist* Playlist)
{
	if (!Playlist || Playlist->Tracks.Num() == 0)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlayPlaylist — Null or empty playlist"));
		return;
	}

	ActivePlaylist = Playlist;
	CurrentPlaylistIndex = -1;

	// EN: Build shuffled indices if needed / ES: Construir indices mezclados si es necesario
	if (Playlist->bShuffle)
	{
		ShuffledIndices.SetNum(Playlist->Tracks.Num());
		for (int32 i = 0; i < ShuffledIndices.Num(); ++i)
		{
			ShuffledIndices[i] = i;
		}
		// EN: Fisher-Yates shuffle / ES: Mezcla Fisher-Yates
		for (int32 i = ShuffledIndices.Num() - 1; i > 0; --i)
		{
			const int32 j = FMath::RandRange(0, i);
			ShuffledIndices.Swap(i, j);
		}
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("PlayPlaylist — %s (%d tracks, loop=%s, shuffle=%s)"),
		*Playlist->GetName(), Playlist->Tracks.Num(),
		Playlist->bLoop ? TEXT("ON") : TEXT("OFF"),
		Playlist->bShuffle ? TEXT("ON") : TEXT("OFF"));

	AdvancePlaylist();
}

void UPGXMusicManager::SetMusicState(FGameplayTag StateTag)
{
	ActiveMusicStateTag = StateTag;
	PGX_LOG_INFO(LogPGXAudio, TEXT("SetMusicState — %s"), *StateTag.ToString());
}

FString UPGXMusicManager::GetCurrentTrackName() const
{
	if (CurrentComponent && CurrentComponent->Sound)
	{
		return CurrentComponent->Sound->GetName();
	}
	return TEXT("None");
}

void UPGXMusicManager::SetState(EPGXMusicState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const EPGXMusicState OldState = CurrentState;
	CurrentState = NewState;

	if (OwnerSubsystem)
	{
		OwnerSubsystem->OnMusicStateChanged.Broadcast(OldState, NewState);
		OwnerSubsystem->OnMusicStateChangedNative.Broadcast(OldState, NewState);
	}
}

void UPGXMusicManager::OnTrackFinished()
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("OnTrackFinished — Track completed"));

	// EN: If playlist is active, advance to next track / ES: Si hay playlist activo, avanzar a siguiente pista
	if (ActivePlaylist.IsValid())
	{
		AdvancePlaylist();
	}
	else
	{
		SetState(EPGXMusicState::Idle);
	}
}

void UPGXMusicManager::OnCrossfadeSourceFinished()
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("OnCrossfadeSourceFinished — Crossfade source faded out"));

	if (CrossfadeComponent)
	{
		CrossfadeComponent->DestroyComponent();
		CrossfadeComponent = nullptr;
	}

	// EN: Transition from CrossFading to Playing / ES: Transicion de CrossFading a Playing
	if (CurrentState == EPGXMusicState::CrossFading)
	{
		SetState(EPGXMusicState::Playing);
	}
}

void UPGXMusicManager::AdvancePlaylist()
{
	const UPGXMusicPlaylist* Playlist = ActivePlaylist.Get();
	if (!Playlist || Playlist->Tracks.Num() == 0)
	{
		SetState(EPGXMusicState::Idle);
		return;
	}

	CurrentPlaylistIndex++;

	// EN: Check if we've reached the end / ES: Comprobar si hemos llegado al final
	if (CurrentPlaylistIndex >= Playlist->Tracks.Num())
	{
		if (Playlist->bLoop)
		{
			CurrentPlaylistIndex = 0;

			// EN: Re-shuffle if shuffle mode / ES: Re-mezclar si modo shuffle
			if (Playlist->bShuffle)
			{
				for (int32 i = ShuffledIndices.Num() - 1; i > 0; --i)
				{
					const int32 j = FMath::RandRange(0, i);
					ShuffledIndices.Swap(i, j);
				}
			}
		}
		else
		{
			PGX_LOG_INFO(LogPGXAudio, TEXT("AdvancePlaylist — Playlist completed"));
			ActivePlaylist.Reset();
			SetState(EPGXMusicState::Idle);
			return;
		}
	}

	// EN: Get actual track index (accounting for shuffle) / ES: Obtener indice de pista real (contabilizando shuffle)
	const int32 TrackIndex = Playlist->bShuffle ? ShuffledIndices[CurrentPlaylistIndex] : CurrentPlaylistIndex;
	const FPGXMusicTrack& Track = Playlist->Tracks[TrackIndex];

	// EN: Load the music asset / ES: Cargar el asset de musica
	USoundBase* Music = Track.Music.LoadSynchronous();
	if (!Music)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("AdvancePlaylist — Failed to load track %d"), TrackIndex);
		AdvancePlaylist(); // EN: Skip to next / ES: Saltar al siguiente
		return;
	}

	// EN: Crossfade or direct play / ES: Crossfade o play directo
	const float CrossfadeDur = Track.CrossfadeDuration > 0.0f ? Track.CrossfadeDuration : Playlist->DefaultCrossfadeDuration;

	if (CurrentComponent && CurrentComponent->IsPlaying() && CrossfadeDur > 0.0f)
	{
		CrossfadeTo(Music, CrossfadeDur);
	}
	else
	{
		PlayMusic(Music, CrossfadeDur > 0.0f ? CrossfadeDur : 0.0f);
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("AdvancePlaylist — Track %d/%d: %s"),
		CurrentPlaylistIndex + 1, Playlist->Tracks.Num(), *Music->GetName());
}

UAudioComponent* UPGXMusicManager::CreateMusicComponent(USoundBase* Sound)
{
	UWorld* World = nullptr;
	if (OwnerSubsystem)
	{
		if (const UGameInstance* GI = OwnerSubsystem->GetGameInstance())
		{
			World = GI->GetWorld();
		}
	}

	if (!World)
	{
		return nullptr;
	}

	// EN: SpawnSound2D creates a non-auto-destroying component we can control
	// ES: SpawnSound2D crea un componente que no se auto-destruye que podemos controlar
	return UGameplayStatics::SpawnSound2D(World, Sound, 1.0f, 1.0f, 0.0f, nullptr, false, false);
}
