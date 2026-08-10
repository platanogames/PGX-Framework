// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Manager/PGXDialogueManager.h"
#include "PGXAudioSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXAudioLog.h"
#include "Tags/PGXAudioTags.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// EN: Dialogue management system implementation
// ES: Implementacion del sistema de gestion de dialogo

void UPGXDialogueManager::Initialize(UPGXAudioSubsystem* InOwner)
{
	OwnerSubsystem = InOwner;
	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXDialogueManager::Initialize — Dialogue manager ready"));
}

void UPGXDialogueManager::Deinitialize()
{
	StopDialogue(0.0f);
	ClearDialogueQueue();

	// EN: StopDialogue already destroys the component, but guard against edge cases
	// ES: StopDialogue ya destruye el componente, pero proteger contra casos borde
	if (DialogueAudioComponent)
	{
		DialogueAudioComponent->DestroyComponent();
		DialogueAudioComponent = nullptr;
	}

	OwnerSubsystem = nullptr;
	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXDialogueManager::Deinitialize — Dialogue manager shutdown"));
}

bool UPGXDialogueManager::QueueDialogue(USoundBase* Sound, const FText& SubtitleText, float Duration,
	FGameplayTag SpeakerTag, FGameplayTag PriorityTag, EPGXDialogueInterruptPolicy Policy)
{
	if (!Sound)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("QueueDialogue — Null sound asset"));
		return false;
	}

	FDialogueEntry Entry;
	Entry.Sound = Sound;
	Entry.SubtitleText = SubtitleText;
	Entry.Duration = Duration;
	Entry.SpeakerTag = SpeakerTag;
	Entry.PriorityTag = PriorityTag;
	Entry.Policy = Policy;

	// EN: Handle interrupt policies / ES: Manejar politicas de interrupcion
	if (bIsPlaying)
	{
		switch (Policy)
		{
		case EPGXDialogueInterruptPolicy::CanInterrupt:
			{
				PGX_LOG_INFO(LogPGXAudio, TEXT("QueueDialogue — Interrupting current dialogue for: %s"), *Sound->GetName());
				StopDialogue(0.1f);
				// EN: Insert at front of queue / ES: Insertar al frente de la cola
				DialogueQueue.Insert(Entry, 0);
				PlayNextInQueue();
				return true;
			}

		case EPGXDialogueInterruptPolicy::QueueBehind:
			{
				// EN: Insert in priority order / ES: Insertar en orden de prioridad
				const int32 NewPriority = GetPriorityValue(PriorityTag);
				int32 InsertIdx = DialogueQueue.Num();
				for (int32 i = 0; i < DialogueQueue.Num(); ++i)
				{
					if (GetPriorityValue(DialogueQueue[i].PriorityTag) < NewPriority)
					{
						InsertIdx = i;
						break;
					}
				}
				DialogueQueue.Insert(Entry, InsertIdx);
				PGX_LOG_INFO(LogPGXAudio, TEXT("QueueDialogue — Queued: %s (position %d)"), *Sound->GetName(), InsertIdx);
				return true;
			}

		case EPGXDialogueInterruptPolicy::DropIfPlaying:
			{
				PGX_LOG_INFO(LogPGXAudio, TEXT("QueueDialogue — Dropped (already playing): %s"), *Sound->GetName());
				return false;
			}
		}
	}
	else
	{
		// EN: Nothing playing, start immediately / ES: Nada reproduciendose, iniciar inmediatamente
		DialogueQueue.Insert(Entry, 0);
		PlayNextInQueue();
		return true;
	}

	return false;
}

void UPGXDialogueManager::StopDialogue(float FadeOutDuration)
{
	if (DialogueAudioComponent)
	{
		DialogueAudioComponent->OnAudioFinished.RemoveDynamic(this, &UPGXDialogueManager::OnDialogueFinished);

		if (DialogueAudioComponent->IsPlaying())
		{
			if (FadeOutDuration > 0.0f)
			{
				// EN: Fade out, then destroy via OnAudioFinished won't fire — destroy now
				// ES: Fade out, pero OnAudioFinished no se dispara — destruir ahora
				DialogueAudioComponent->FadeOut(FadeOutDuration, 0.0f);
			}
			else
			{
				DialogueAudioComponent->Stop();
			}
		}

		DialogueAudioComponent->DestroyComponent();
		DialogueAudioComponent = nullptr;
	}
	bIsPlaying = false;
}

void UPGXDialogueManager::ClearDialogueQueue()
{
	DialogueQueue.Empty();
	PGX_LOG_INFO(LogPGXAudio, TEXT("ClearDialogueQueue — Queue cleared"));
}

void UPGXDialogueManager::PlayNextInQueue()
{
	if (DialogueQueue.Num() == 0)
	{
		bIsPlaying = false;
		return;
	}

	FDialogueEntry Entry = DialogueQueue[0];
	DialogueQueue.RemoveAt(0);

	USoundBase* Sound = Entry.Sound.Get();
	if (!Sound)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlayNextInQueue — Sound was garbage collected, skipping"));
		PlayNextInQueue();
		return;
	}

	// EN: Get world context from owner subsystem / ES: Obtener contexto de mundo del subsistema propietario
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
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlayNextInQueue — No valid world context"));
		return;
	}

	// EN: Destroy previous component before creating new one / ES: Destruir componente previo antes de crear uno nuevo
	if (DialogueAudioComponent)
	{
		DialogueAudioComponent->OnAudioFinished.RemoveDynamic(this, &UPGXDialogueManager::OnDialogueFinished);
		DialogueAudioComponent->Stop();
		DialogueAudioComponent->DestroyComponent();
		DialogueAudioComponent = nullptr;
	}

	// EN: Create audio component for dialogue / ES: Crear componente de audio para dialogo
	DialogueAudioComponent = UGameplayStatics::SpawnSound2D(World, Sound, 1.0f, 1.0f, 0.0f, nullptr, false, false);

	if (DialogueAudioComponent)
	{
		bIsPlaying = true;
		CurrentSpeakerTag = Entry.SpeakerTag;

		// EN: Bind completion delegate / ES: Vincular delegado de completado
		DialogueAudioComponent->OnAudioFinished.AddDynamic(this, &UPGXDialogueManager::OnDialogueFinished);

		PGX_LOG_INFO(LogPGXAudio, TEXT("PlayNextInQueue — Playing dialogue: %s (Speaker: %s)"),
			*Sound->GetName(), *Entry.SpeakerTag.ToString());

		// EN: Broadcast subtitle delegate / ES: Difundir delegado de subtitulo
		if (OwnerSubsystem)
		{
			OwnerSubsystem->OnDialogueSubtitle.Broadcast(Entry.SubtitleText, Entry.Duration, Entry.SpeakerTag);
			OwnerSubsystem->OnDialogueSubtitleNative.Broadcast(Entry.SubtitleText, Entry.Duration, Entry.SpeakerTag);

			OwnerSubsystem->OnSoundPlayed.Broadcast(FPGXSoundHandle(), TAG_PGX_Audio_Profile_Default);
			OwnerSubsystem->OnSoundPlayedNative.Broadcast(FPGXSoundHandle(), TAG_PGX_Audio_Profile_Default);
		}
	}
	else
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("PlayNextInQueue — Failed to spawn audio component"));
		bIsPlaying = false;
	}
}

void UPGXDialogueManager::OnDialogueFinished()
{
	PGX_LOG_INFO(LogPGXAudio, TEXT("OnDialogueFinished — Dialogue completed"));

	// EN: Destroy the finished component / ES: Destruir el componente terminado
	if (DialogueAudioComponent)
	{
		DialogueAudioComponent->OnAudioFinished.RemoveDynamic(this, &UPGXDialogueManager::OnDialogueFinished);
		DialogueAudioComponent->DestroyComponent();
		DialogueAudioComponent = nullptr;
	}

	bIsPlaying = false;

	// EN: Play next in queue if available / ES: Reproducir siguiente en cola si esta disponible
	if (OwnerSubsystem)
	{
		PlayNextInQueue();
	}
}

int32 UPGXDialogueManager::GetPriorityValue(const FGameplayTag& PriorityTag) const
{
	if (PriorityTag == TAG_PGX_Audio_Dialogue_Priority_Critical) return 3;
	if (PriorityTag == TAG_PGX_Audio_Dialogue_Priority_High)     return 2;
	if (PriorityTag == TAG_PGX_Audio_Dialogue_Priority_Normal)   return 1;
	if (PriorityTag == TAG_PGX_Audio_Dialogue_Priority_Low)      return 0;
	return 1; // EN: Default to Normal / ES: Default a Normal
}
