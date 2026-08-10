// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXAudioTypes.h"
#include "Data/PGXAudioDuckingConfig.h"
#include "PGXAudioConfig.generated.h"

/**
 * EN: Config DataAsset for the PGX Audio system v1.1.
 *     Single point of configuration: backend type, pool sizes, HDR/LDR chains,
 *     HRTF defaults, ducking config reference, trace settings.
 *     Auto-discovered via AssetRegistry at subsystem initialization.
 *
 * ES: Config DataAsset para el sistema PGX Audio v1.1.
 *     Punto unico de configuracion: tipo de backend, tamanos de pool, cadenas HDR/LDR,
 *     defaults de HRTF, referencia a config de ducking, settings de trace.
 *     Auto-descubierto via AssetRegistry en la inicializacion del subsistema.
 */
UCLASS(BlueprintType)
class PGXAUDIORUNTIME_API UPGXAudioConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ── Backend ──

	/** EN: Which audio backend to use / ES: Que backend de audio usar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Backend")
	EPGXAudioBackendType BackendType = EPGXAudioBackendType::Auto;

	// ── Default Volumes ──

	/** EN: Default master volume (0.0 - 1.0) / ES: Volumen master por defecto (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Defaults", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	/** EN: Default music volume (0.0 - 1.0) / ES: Volumen de musica por defecto (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Defaults", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.7f;

	/** EN: Default SFX volume (0.0 - 1.0) / ES: Volumen de efectos por defecto (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Defaults", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SFXVolume = 1.0f;

	/** EN: Default voice volume (0.0 - 1.0) / ES: Volumen de voz por defecto (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Defaults", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VoiceVolume = 1.0f;

	// ── Music ──

	/** EN: Default crossfade duration for music transitions (seconds) / ES: Duracion de crossfade por defecto para transiciones de musica (segundos) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music", meta = (AdvancedDisplay, ClampMin = "0.0"))
	float DefaultMusicCrossfadeDuration = 2.0f;

	// ── Pool ──

	/** EN: Initial capacity of the sound instance pool / ES: Capacidad inicial del pool de instancias de sonido */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Pool", meta = (AdvancedDisplay, ClampMin = "1"))
	int32 SoundPoolInitialSize = 32;

	/** EN: Maximum pool capacity (0 = unlimited growth) / ES: Capacidad maxima del pool (0 = crecimiento ilimitado) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Pool", meta = (AdvancedDisplay, ClampMin = "0"))
	int32 SoundPoolMaxSize = 128;

	// ── Concurrency ──

	/** EN: Default concurrency policy when limit is reached / ES: Politica de concurrencia por defecto cuando se alcanza el limite */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Concurrency", meta = (AdvancedDisplay))
	EPGXAudioConcurrencyPolicy DefaultConcurrencyPolicy = EPGXAudioConcurrencyPolicy::StopOldest;

	/** EN: Default max concurrent sounds per channel (0 = unlimited) / ES: Maximo de sonidos concurrentes por canal por defecto (0 = ilimitado) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Concurrency", meta = (AdvancedDisplay, ClampMin = "0"))
	int32 DefaultMaxConcurrentPerChannel = 0;

	// ── HDR / LDR ──

	/** EN: HDR submix effect chain (for high dynamic range audio) / ES: Cadena de efectos submix HDR (para audio de alto rango dinamico) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|HDR", meta = (AdvancedDisplay, AllowedClasses = "/Script/Engine.SoundEffectSubmixPreset"))
	TSoftObjectPtr<UObject> HDREffectChain;

	/** EN: LDR submix effect chain (standard range) / ES: Cadena de efectos submix LDR (rango estandar) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|HDR", meta = (AdvancedDisplay, AllowedClasses = "/Script/Engine.SoundEffectSubmixPreset"))
	TSoftObjectPtr<UObject> LDREffectChain;

	/** EN: Whether HDR audio is enabled by default / ES: Si el audio HDR esta habilitado por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|HDR", meta = (AdvancedDisplay))
	bool bDefaultHDRAudioEnabled = false;

	// ── HRTF ──

	/** EN: Whether HRTF (binaural spatialization) is enabled by default / ES: Si HRTF (espacializacion binaural) esta habilitado por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|HRTF", meta = (AdvancedDisplay))
	bool bDefaultHRTFEnabled = false;

	// ── Ducking ──

	/** EN: Global ducking config (can be overridden per-level) / ES: Config de ducking global (puede ser overrideada por nivel) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking", meta = (AdvancedDisplay))
	TSoftObjectPtr<UPGXAudioDuckingConfig> GlobalDuckingConfig;

	// ── Occlusion ──

	/** EN: Enable audio occlusion checks / ES: Habilitar chequeos de oclusion de audio */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Spatial", meta = (AdvancedDisplay))
	bool bEnableAudioOcclusion = false;

	// ── Trace / Debug ──

	/** EN: Enable PGX_TRACE_SCOPE for audio operations / ES: Habilitar PGX_TRACE_SCOPE para operaciones de audio */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Debug", meta = (AdvancedDisplay))
	bool bEnableTrace = false;

	/** EN: Max entries in event history ring buffer / ES: Maximo de entradas en el buffer circular de historial de eventos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Debug", meta = (AdvancedDisplay, ClampMin = "10", ClampMax = "1000"))
	int32 MaxEventHistorySize = 200;

	/** EN: Allow console commands that mutate audio runtime state outside Shipping / ES: Permitir comandos de consola que mutan estado de audio fuera de Shipping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Debug", meta = (AdvancedDisplay))
	bool bAllowConsoleMutations = false;

	/** EN: Keep diagnostic event history in Shipping builds / ES: Mantener historial diagnostico de eventos en builds Shipping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Debug", meta = (AdvancedDisplay))
	bool bRecordEventHistoryInShipping = false;

	/** EN: Expose event history query results in Shipping builds / ES: Exponer resultados de historial de eventos en builds Shipping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Debug", meta = (AdvancedDisplay))
	bool bExposeEventHistoryInShipping = false;
};
