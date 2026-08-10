// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PGXAudioTypes.h"
#include "PGXAudioComponent.generated.h"

class UPGXAudioSubsystem;

/**
 * EN: PGX Audio Component — Actor component for simplified sound playback.
 *     Provides tag-based sound resolution and automatic profile/channel handling.
 *     Place on any Actor that needs to play sounds with PGX integration.
 *
 * ES: Componente de Audio PGX — Componente de Actor para reproduccion simplificada de sonido.
 *     Provee resolucion de sonido basada en tags y manejo automatico de perfil/canal.
 *     Colocar en cualquier Actor que necesite reproducir sonidos con integracion PGX.
 */
UCLASS(ClassGroup = (PGX), meta = (BlueprintSpawnableComponent))
class PGXAUDIORUNTIME_API UPGXAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPGXAudioComponent();

	// ── Playback ──

	/**
	 * EN: Play a sound by tag (resolved through SoundDefinition).
	 * ES: Reproducir un sonido por tag (resuelto a traves de SoundDefinition).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Component")
	FPGXSoundHandle PlaySound(FGameplayTag SoundTag, const FGameplayTagContainer& ContextTags);

	/**
	 * EN: Play a sound asset directly (no tag resolution).
	 * ES: Reproducir un asset de sonido directamente (sin resolucion de tag).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Component")
	FPGXSoundHandle PlaySoundDirect(USoundBase* Sound);

	/**
	 * EN: Stop all sounds started by this component.
	 * ES: Detener todos los sonidos iniciados por este componente.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Component")
	void StopAll(float FadeOutDuration = 0.0f);

	/**
	 * EN: Set the audio profile for subsequent sounds.
	 * ES: Establecer el perfil de audio para sonidos posteriores.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Component")
	void SetProfile(FGameplayTag ProfileTag);

	// ── Configuration ──

	/** EN: Default channel tag for sounds from this component / ES: Tag de canal por defecto para sonidos de este componente */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio|Component", meta = (Categories = "PGX.Audio.Channel"))
	FGameplayTag DefaultChannelTag;

	/** EN: Default audio profile tag / ES: Tag de perfil de audio por defecto */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio|Component", meta = (Categories = "PGX.Audio.Profile"))
	FGameplayTag DefaultProfileTag;

	/** EN: Whether to play sounds at component's world location / ES: Si reproducir sonidos en la ubicacion mundial del componente */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|Audio|Component")
	bool bPlayAtLocation = true;

protected:
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** EN: Get the audio subsystem / ES: Obtener el subsistema de audio */
	UPGXAudioSubsystem* GetAudioSubsystem() const;

	/** EN: Active sound handles from this component / ES: Handles de sonido activos de este componente */
	TArray<FPGXSoundHandle> ActiveHandles;
};
