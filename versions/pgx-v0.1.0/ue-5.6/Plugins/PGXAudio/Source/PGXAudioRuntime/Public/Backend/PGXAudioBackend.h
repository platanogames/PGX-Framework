// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXAudioTypes.h"
#include "PGXAudioBackend.generated.h"

// Forward declarations
class UPGXAudioConfig;

// ============================================================================
// EN: Strategy interface for audio backend implementations.
//     Two concrete backends: Legacy (SoundMix/SoundClass) and Modulation (ControlBus).
//     The AudioSubsystem delegates all volume/mix operations to the active backend.
//     Hot-switchable at runtime via AudioSubsystem::SwitchBackend().
//
// ES: Interfaz Strategy para implementaciones de backend de audio.
//     Dos backends concretos: Legacy (SoundMix/SoundClass) y Modulation (ControlBus).
//     El AudioSubsystem delega todas las operaciones de volumen/mix al backend activo.
//     Hot-switchable en runtime via AudioSubsystem::SwitchBackend().
// ============================================================================

/**
 * EN: Abstract backend interface. Concrete implementations handle the actual
 *     UE audio API calls (SoundMix vs ControlBus).
 * ES: Interfaz abstracta de backend. Las implementaciones concretas manejan las
 *     llamadas reales al API de audio de UE (SoundMix vs ControlBus).
 */
UCLASS(Abstract)
class PGXAUDIORUNTIME_API UPGXAudioBackend : public UObject
{
	GENERATED_BODY()

public:
	// ── Lifecycle ──

	/**
	 * EN: Initialize the backend with config reference.
	 * ES: Inicializar el backend con referencia a config.
	 * @param Config The audio config DA with backend-specific settings.
	 * @return true if initialization succeeded.
	 */
	virtual bool InitializeBackend(const UPGXAudioConfig* /*Config*/) PURE_VIRTUAL(UPGXAudioBackend::InitializeBackend, return false;);

	/**
	 * EN: Shutdown and release all backend resources.
	 * ES: Apagar y liberar todos los recursos del backend.
	 */
	virtual void ShutdownBackend() PURE_VIRTUAL(UPGXAudioBackend::ShutdownBackend,);

	/**
	 * EN: Returns the backend type this implementation represents.
	 * ES: Retorna el tipo de backend que representa esta implementacion.
	 */
	virtual EPGXAudioBackendType GetBackendType() const PURE_VIRTUAL(UPGXAudioBackend::GetBackendType, return EPGXAudioBackendType::Legacy;);

	// ── Volume Control ──

	/**
	 * EN: Set the volume for a specific channel.
	 * ES: Establecer el volumen para un canal especifico.
	 * @param ChannelTag The channel to modify.
	 * @param Volume The target volume (0.0 - 1.0).
	 */
	virtual void SetChannelVolume(const FGameplayTag& /*ChannelTag*/, float /*Volume*/) PURE_VIRTUAL(UPGXAudioBackend::SetChannelVolume,);

	/**
	 * EN: Get the current volume for a channel.
	 * ES: Obtener el volumen actual de un canal.
	 */
	virtual float GetChannelVolume(const FGameplayTag& /*ChannelTag*/) const PURE_VIRTUAL(UPGXAudioBackend::GetChannelVolume, return 1.0f;);

	/**
	 * EN: Set mute state for a channel.
	 * ES: Establecer estado de mute para un canal.
	 */
	virtual void SetChannelMuted(const FGameplayTag& /*ChannelTag*/, bool /*bMuted*/) PURE_VIRTUAL(UPGXAudioBackend::SetChannelMuted,);

	/**
	 * EN: Check if a channel is muted.
	 * ES: Comprobar si un canal esta muteado.
	 */
	virtual bool IsChannelMuted(const FGameplayTag& /*ChannelTag*/) const PURE_VIRTUAL(UPGXAudioBackend::IsChannelMuted, return false;);

	// ── Mix Integration ──

	/**
	 * EN: Apply a mix layer's volume values through this backend.
	 * ES: Aplicar los valores de volumen de una capa de mezcla a traves de este backend.
	 * @param Layer The mix layer being applied.
	 * @param ChannelValues Map of channel tag → volume value.
	 */
	virtual void ApplyMixLayer(EPGXMixLayer /*Layer*/, const TMap<FGameplayTag, float>& /*ChannelValues*/) PURE_VIRTUAL(UPGXAudioBackend::ApplyMixLayer,);

	/**
	 * EN: Remove a mix layer's influence (restore previous values).
	 * ES: Quitar la influencia de una capa de mezcla (restaurar valores previos).
	 */
	virtual void RemoveMixLayer(EPGXMixLayer /*Layer*/) PURE_VIRTUAL(UPGXAudioBackend::RemoveMixLayer,);

	// ── Query ──

	/**
	 * EN: Get human-readable status text for inspector/console.
	 * ES: Obtener texto de estado legible para inspector/consola.
	 */
	virtual FString GetStatusText() const PURE_VIRTUAL(UPGXAudioBackend::GetStatusText, return TEXT("Unknown"););

	/**
	 * EN: Check if this backend is fully operational.
	 * ES: Comprobar si este backend esta completamente operativo.
	 */
	virtual bool IsOperational() const PURE_VIRTUAL(UPGXAudioBackend::IsOperational, return false;);
};
