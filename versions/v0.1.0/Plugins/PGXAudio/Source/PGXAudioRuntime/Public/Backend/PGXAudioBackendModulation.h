// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Backend/PGXAudioBackend.h"
#include "PGXAudioBackendModulation.generated.h"

// Forward declarations
class UPGXAudioChannelConfig;
class USoundControlBus;
class USoundControlBusMix;

/**
 * EN: Modulation audio backend using ControlBus and ControlBusMix.
 *     Controls channel volumes through AudioModulation system (Epic's plugin).
 *     Requires AudioModulation plugin to be enabled.
 *     Provides finer-grained control and better performance than Legacy.
 *
 * ES: Backend de audio Modulation usando ControlBus y ControlBusMix.
 *     Controla volumenes de canal a traves del sistema AudioModulation (plugin de Epic).
 *     Requiere que el plugin AudioModulation este habilitado.
 *     Provee control mas fino y mejor rendimiento que Legacy.
 */
UCLASS()
class PGXAUDIORUNTIME_API UPGXAudioBackendModulation : public UPGXAudioBackend
{
	GENERATED_BODY()

public:
	//~ Begin UPGXAudioBackend Interface
	bool InitializeBackend(const UPGXAudioConfig* Config) override;
	void ShutdownBackend() override;
	EPGXAudioBackendType GetBackendType() const override { return EPGXAudioBackendType::Modulation; }
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
		TWeakObjectPtr<USoundControlBus> ControlBus;
	};

	/** EN: Channel states keyed by tag / ES: Estados de canal indexados por tag */
	TMap<FGameplayTag, FChannelState> ChannelStates;

	/** EN: Whether backend is initialized / ES: Si el backend esta inicializado */
	bool bInitialized = false;

	/** EN: Apply the effective volume through ControlBus / ES: Aplicar el volumen efectivo a traves de ControlBus */
	void ApplyVolumeToControlBus(const FGameplayTag& ChannelTag, float EffectiveVolume);
};
