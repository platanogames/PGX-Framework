// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXAudioTypes.h"
#include "PGXLevelAudioConfig.generated.h"

// Forward declarations
class UPGXAudioDuckingConfig;
class UReverbEffect;

/**
 * EN: Per-level audio configuration DataAsset.
 *     Overrides global audio settings for a specific level/map.
 *     Referenced by level designers to define ambient zones, ducking overrides,
 *     reverb settings, and music state for their level.
 *     Auto-discovered via AssetRegistry.
 *
 * ES: DataAsset de configuracion de audio por nivel.
 *     Sobreescribe settings de audio globales para un nivel/mapa especifico.
 *     Referenciado por level designers para definir zonas ambientales, overrides de ducking,
 *     settings de reverb y estado musical para su nivel.
 *     Auto-descubierto via AssetRegistry.
 */
UCLASS(BlueprintType)
class PGXAUDIORUNTIME_API UPGXLevelAudioConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ── Level Identity ──

	/** EN: Level/map name this config applies to (soft match) / ES: Nombre de nivel/mapa al que aplica esta config (match suave) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Level")
	FName LevelName;

	// ── Mix Overrides ──

	/** EN: Per-channel volume overrides for the Level mix layer / ES: Overrides de volumen por canal para la capa de mezcla Level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Level|Mix")
	TMap<FGameplayTag, float> ChannelVolumeOverrides;

	// ── Ducking ──

	/** EN: Level-specific ducking config (overrides global if set) / ES: Config de ducking especifica del nivel (sobreescribe global si esta definida) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Level|Ducking", meta = (AdvancedDisplay))
	TSoftObjectPtr<UPGXAudioDuckingConfig> LevelDuckingConfig;

	// ── Ambient Zones ──

	/** EN: Ambient audio zone configurations for this level / ES: Configuraciones de zona de audio ambiental para este nivel */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Level|Ambient")
	TArray<FPGXAmbientZoneConfig> AmbientZones;

	// ── Reverb ──

	/** EN: Default reverb effect for this level / ES: Efecto de reverb por defecto para este nivel */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Level|Reverb", meta = (AdvancedDisplay))
	TSoftObjectPtr<UReverbEffect> DefaultReverbEffect;

	/** EN: Reverb wet/dry mix (0 = dry, 1 = full reverb) / ES: Mezcla reverb wet/dry (0 = seco, 1 = reverb completo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Level|Reverb", meta = (AdvancedDisplay, ClampMin = "0.0", ClampMax = "1.0"))
	float ReverbWetLevel = 0.0f;

	// ── Music ──

	/** EN: Music state tag to activate when this level loads / ES: Tag de estado musical para activar cuando carga este nivel */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Level|Music", meta = (AdvancedDisplay))
	FGameplayTag LevelMusicStateTag;

	// ── Spatial ──

	/** EN: Override HDR audio setting for this level (-1 = use global) / ES: Override de HDR audio para este nivel (-1 = usar global) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Level|Spatial", meta = (AdvancedDisplay))
	int32 HDRAudioOverride = -1;

	/** EN: Override HRTF setting for this level (-1 = use global) / ES: Override de HRTF para este nivel (-1 = usar global) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Level|Spatial", meta = (AdvancedDisplay))
	int32 HRTFOverride = -1;
};
