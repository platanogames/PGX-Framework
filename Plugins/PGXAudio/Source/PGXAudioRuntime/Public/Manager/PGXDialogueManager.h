// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "PGXAudioTypes.h"
#include "PGXDialogueManager.generated.h"

// Forward declarations
class UPGXAudioSubsystem;
class UAudioComponent;

/**
 * EN: Dialogue management system — priority queue, subtitle dispatch, auto-ducking.
 *     Owned by UPGXAudioSubsystem. Handles queuing dialogue lines with priority,
 *     interruption policies, and broadcasts subtitle info for UI.
 *
 * ES: Sistema de gestion de dialogo — cola de prioridad, despacho de subtitulos, auto-ducking.
 *     Owned por UPGXAudioSubsystem. Maneja el encolamiento de lineas de dialogo con prioridad,
 *     politicas de interrupcion y difunde info de subtitulos para UI.
 */
UCLASS()
class PGXAUDIORUNTIME_API UPGXDialogueManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * EN: Initialize with owner subsystem reference.
	 * ES: Inicializar con referencia al subsistema propietario.
	 */
	void Initialize(UPGXAudioSubsystem* InOwner);

	/** EN: Shutdown and clear queue / ES: Apagar y limpiar cola */
	void Deinitialize();

	// ── Dialogue API ──

	/**
	 * EN: Queue a dialogue line for playback.
	 * ES: Encolar una linea de dialogo para reproduccion.
	 * @param Sound The dialogue audio asset.
	 * @param SubtitleText Subtitle text for UI display.
	 * @param Duration Duration in seconds (0 = use sound duration).
	 * @param SpeakerTag Tag identifying the speaker.
	 * @param PriorityTag Priority level tag (Low/Normal/High/Critical).
	 * @param Policy How to handle current playback.
	 * @return true if dialogue was queued/started.
	 */
	bool QueueDialogue(USoundBase* Sound, const FText& SubtitleText, float Duration,
		FGameplayTag SpeakerTag, FGameplayTag PriorityTag,
		EPGXDialogueInterruptPolicy Policy = EPGXDialogueInterruptPolicy::QueueBehind);

	/** EN: Stop current dialogue with optional fade / ES: Detener dialogo actual con fade opcional */
	void StopDialogue(float FadeOutDuration = 0.0f);

	/** EN: Clear all queued dialogues / ES: Limpiar todos los dialogos encolados */
	void ClearDialogueQueue();

	/** EN: Get number of dialogues in queue / ES: Obtener numero de dialogos en cola */
	int32 GetDialogueQueueCount() const { return DialogueQueue.Num(); }

	/** EN: Check if dialogue is currently playing / ES: Comprobar si un dialogo esta reproduciendose */
	bool IsDialoguePlaying() const { return bIsPlaying; }

private:
	/** EN: Internal dialogue entry for the queue / ES: Entrada interna de dialogo para la cola */
	struct FDialogueEntry
	{
		TWeakObjectPtr<USoundBase> Sound;
		FText SubtitleText;
		float Duration = 0.0f;
		FGameplayTag SpeakerTag;
		FGameplayTag PriorityTag;
		EPGXDialogueInterruptPolicy Policy = EPGXDialogueInterruptPolicy::QueueBehind;
	};

	/** EN: Play the next dialogue in queue / ES: Reproducir el siguiente dialogo en cola */
	void PlayNextInQueue();

	/** EN: Callback when current dialogue finishes / ES: Callback cuando el dialogo actual termina */
	UFUNCTION()
	void OnDialogueFinished();

	/** EN: Convert priority tag to numeric value for comparison / ES: Convertir tag de prioridad a valor numerico para comparacion */
	int32 GetPriorityValue(const FGameplayTag& PriorityTag) const;

	/** EN: Owner subsystem reference / ES: Referencia al subsistema propietario */
	UPROPERTY()
	TObjectPtr<UPGXAudioSubsystem> OwnerSubsystem = nullptr;

	/** EN: Active dialogue audio component / ES: Componente de audio de dialogo activo */
	UPROPERTY()
	TObjectPtr<UAudioComponent> DialogueAudioComponent = nullptr;

	/** EN: Priority queue of pending dialogues / ES: Cola de prioridad de dialogos pendientes */
	TArray<FDialogueEntry> DialogueQueue;

	/** EN: Whether dialogue is currently playing / ES: Si un dialogo esta reproduciendose actualmente */
	bool bIsPlaying = false;

	/** EN: Current dialogue info for subtitle tracking / ES: Info del dialogo actual para rastreo de subtitulos */
	FGameplayTag CurrentSpeakerTag;
};
