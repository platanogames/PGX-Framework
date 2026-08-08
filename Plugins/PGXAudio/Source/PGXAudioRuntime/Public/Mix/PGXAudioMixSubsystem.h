// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXWorldSubsystem.h"
#include "PGXAudioTypes.h"
#include "PGXAudioDelegates.h"
#include "AudioMixerBlueprintLibrary.h"
#include "PGXAudioMixSubsystem.generated.h"

// Forward declarations
class UPGXAudioSubsystem;
class UPGXAudioBackend;
class UPGXLevelAudioConfig;
class UPGXAudioDuckingConfig;

/**
 * EN: Per-level audio mix subsystem (WorldSubsystem scope — per-world lifecycle).
 *     Manages the 5-layer mix model, ducking rules, ambient zones,
 *     HDR/LDR effect chain swap, and HRTF toggle.
 *     Auto-created/destroyed with UWorld. The main AudioSubsystem (GameInstance)
 *     gracefully detects null MixSubsystem during world transitions.
 *     Architecture validated by Lyra's ULyraAudioMixEffectsSubsystem.
 *
 * ES: Subsistema de mezcla de audio por nivel (scope WorldSubsystem — ciclo de vida por mundo).
 *     Gestiona el modelo de mezcla de 5 capas, reglas de ducking, zonas ambientales,
 *     swap de cadenas de efectos HDR/LDR y toggle de HRTF.
 *     Auto-creado/destruido con UWorld. El AudioSubsystem principal (GameInstance)
 *     detecta gracilmente MixSubsystem null durante transiciones de mundo.
 *     Arquitectura validada por ULyraAudioMixEffectsSubsystem de Lyra.
 */
UCLASS(BlueprintType)
class PGXAUDIORUNTIME_API UPGXAudioMixSubsystem : public UPGXWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ── Mix Layer Management ──

	/**
	 * EN: Apply a mix layer with per-channel values.
	 * ES: Aplicar una capa de mezcla con valores por canal.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Mix")
	void ActivateMixLayer(EPGXMixLayer Layer, const TMap<FGameplayTag, float>& ChannelValues, const FString& SourceName = TEXT(""));

	/**
	 * EN: Deactivate a mix layer, restoring previous values.
	 * ES: Desactivar una capa de mezcla, restaurando valores previos.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Mix")
	void DeactivateMixLayer(EPGXMixLayer Layer);

	/** EN: Get state of a specific mix layer / ES: Obtener estado de una capa de mezcla especifica */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Mix")
	FPGXMixLayerState GetMixLayerState(EPGXMixLayer Layer) const;

	/** EN: Get all mix layer states / ES: Obtener todos los estados de capas de mezcla */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Mix")
	TArray<FPGXMixLayerState> GetAllMixLayerStates() const;

	// ── Level Audio Config ──

	/**
	 * EN: Apply a level audio config. Called on level load or manually.
	 * ES: Aplicar una config de audio de nivel. Llamado al cargar nivel o manualmente.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Mix")
	void SetLevelAudioConfig(UPGXLevelAudioConfig* InConfig);

	/** EN: Get the active level audio config / ES: Obtener la config de audio de nivel activa */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Mix")
	const UPGXLevelAudioConfig* GetLevelAudioConfig() const { return ActiveLevelConfig; }

	// ── Ducking ──

	/** EN: Get active ducking rules / ES: Obtener reglas de ducking activas */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Ducking")
	TArray<FPGXDuckingRule> GetActiveDuckingRules() const;

	/** EN: Enable/disable ducking evaluation / ES: Habilitar/deshabilitar evaluacion de ducking */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Ducking")
	void SetDuckingEnabled(bool bEnabled);

	/** EN: Check if ducking is enabled / ES: Comprobar si el ducking esta habilitado */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Ducking")
	bool IsDuckingEnabled() const { return bDuckingEnabled; }

	// ── HDR / LDR ──

	/** EN: Toggle HDR audio (submix effect chain swap) / ES: Activar/desactivar audio HDR (swap de cadena de efectos submix) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|HDR")
	void SetHDRAudioEnabled(bool bEnabled);

	/** EN: Check if HDR audio is active / ES: Comprobar si el audio HDR esta activo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|HDR")
	bool IsHDRAudioEnabled() const { return bHDRAudioEnabled; }

	// ── HRTF ──

	/** EN: Toggle HRTF (binaural spatialization) / ES: Activar/desactivar HRTF (espacializacion binaural) */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|HRTF")
	void SetHRTFEnabled(bool bEnabled);

	/** EN: Check if HRTF is active / ES: Comprobar si HRTF esta activo */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|HRTF")
	bool IsHRTFEnabled() const { return bHRTFEnabled; }

	// ── Audio Device ──

	/** EN: Get available audio output devices / ES: Obtener dispositivos de salida de audio disponibles */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Device")
	TArray<FString> GetAvailableAudioDevices() const;

	/** EN: Get current audio output device name / ES: Obtener nombre del dispositivo de salida de audio actual */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PGX|Audio|Device")
	FString GetCurrentAudioDevice() const;

	/**
	 * EN: Request audio output device switch (async). Returns true if request was submitted.
	 *     CurrentAudioDevice is only updated on confirmed success via completion callback.
	 * ES: Solicitar cambio de dispositivo de salida de audio (async). Retorna true si la solicitud se envio.
	 *     CurrentAudioDevice solo se actualiza cuando el exito se confirma via callback de completado.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Audio|Device")
	bool SetAudioDevice(const FString& DeviceName);

private:
	/** EN: Internal callback for async device swap result / ES: Callback interno para resultado de swap de dispositivo async */
	UFUNCTION()
	void OnDeviceSwapCompleted(const FSwapAudioOutputResult& SwapResult);

	/** EN: Pending device name being switched to / ES: Nombre de dispositivo pendiente al que se esta cambiando */
	FString PendingDeviceName;

protected:
	/** EN: Load ducking config (global or level-specific) / ES: Cargar config de ducking (global o por nivel) */
	void LoadDuckingConfig();

	/** EN: Recalculate effective volumes from all active layers / ES: Recalcular volumenes efectivos de todas las capas activas */
	void RecalculateEffectiveVolumes();

	/** EN: Get reference to the main AudioSubsystem / ES: Obtener referencia al AudioSubsystem principal */
	UPGXAudioSubsystem* GetAudioSubsystem() const;

	/** EN: Get the active backend from the main subsystem / ES: Obtener el backend activo del subsistema principal */
	UPGXAudioBackend* GetActiveBackend() const;

private:
	// ── Mix Layer State ──

	/** EN: Array of mix layer states (indexed by EPGXMixLayer) / ES: Array de estados de capa de mezcla (indexado por EPGXMixLayer) */
	TArray<FPGXMixLayerState> MixLayers;

	// ── Level Config ──

	/** EN: Active level audio config / ES: Config de audio de nivel activa */
	UPROPERTY()
	TObjectPtr<UPGXLevelAudioConfig> ActiveLevelConfig = nullptr;

	// ── Ducking ──

	/** EN: Active ducking config / ES: Config de ducking activa */
	UPROPERTY()
	TObjectPtr<UPGXAudioDuckingConfig> ActiveDuckingConfig = nullptr;

	/** EN: Whether ducking evaluation is enabled / ES: Si la evaluacion de ducking esta habilitada */
	bool bDuckingEnabled = true;

	// ── Spatial State ──

	/** EN: Whether HDR audio is currently active / ES: Si el audio HDR esta activo actualmente */
	bool bHDRAudioEnabled = false;

	/** EN: Whether HRTF is currently active / ES: Si HRTF esta activo actualmente */
	bool bHRTFEnabled = false;

	/** EN: Current audio device name / ES: Nombre del dispositivo de audio actual */
	FString CurrentAudioDevice;

	/** EN: Cached list of available audio devices / ES: Lista cacheada de dispositivos de audio disponibles */
	TArray<FString> CachedAudioDevices;
};
