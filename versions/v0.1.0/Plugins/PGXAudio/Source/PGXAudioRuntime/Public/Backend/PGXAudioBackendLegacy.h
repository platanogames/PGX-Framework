// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Backend/PGXAudioBackend.h"
#include "PGXAudioBackendLegacy.generated.h"

// Forward declarations
class UPGXAudioChannelConfig;

/**
 * EN: Legacy audio backend using SoundMix and SoundClass.
 *     Controls channel volumes by pushing/popping SoundMix modifiers.
 *     Compatible with all UE versions. No AudioModulation plugin required.
 *
 * ES: Backend de audio Legacy usando SoundMix y SoundClass.
 *     Controla volumenes de canal empujando/sacando modificadores SoundMix.
 *     Compatible con todas las versiones de UE. No requiere plugin AudioModulation.
 */
UCLASS()
class PGXAUDIORUNTIME_API UPGXAudioBackendLegacy : public UPGXAudioBackend
{
	GENERATED_BODY()

public:
	//~ Begin UPGXAudioBackend Interface
	bool InitializeBackend(const UPGXAudioConfig* Config) override;
	void ShutdownBackend() override;
	EPGXAudioBackendType GetBackendType() const override { return EPGXAudioBackendType::Legacy; }
	void SetChannelVolume(const FGameplayTag& ChannelTag, float Volume) override;
	float GetChannelVolume(const FGameplayTag& ChannelTag) const override;
	void SetChannelMuted(const FGameplayTag& ChannelTag, bool bMuted) override;
	bool IsChannelMuted(const FGameplayTag& ChannelTag) const override;
	void ApplyMixLayer(EPGXMixLayer Layer, const TMap<FGameplayTag, float>& ChannelValues) override;
	void RemoveMixLayer(EPGXMixLayer Layer) override;
	FString GetStatusText() const override;
	bool IsOperational() const override { return bInitialized; }
	//~ End UPGXAudioBackend Interface

private:
	/** EN: Per-channel runtime state / ES: Estado runtime por canal */
	struct FChannelState
	{
		float Volume = 1.0f;
		bool bMuted = false;
		TWeakObjectPtr<UPGXAudioChannelConfig> Config;
	};

	/** EN: Channel states keyed by tag / ES: Estados de canal indexados por tag */
	TMap<FGameplayTag, FChannelState> ChannelStates;

	/** EN: Whether backend is initialized / ES: Si el backend esta inicializado */
	bool bInitialized = false;

	/** EN: Apply the effective volume through SoundMix / ES: Aplicar el volumen efectivo a traves de SoundMix */
	void ApplyVolumeToSoundClass(const FGameplayTag& ChannelTag, float EffectiveVolume);
};
