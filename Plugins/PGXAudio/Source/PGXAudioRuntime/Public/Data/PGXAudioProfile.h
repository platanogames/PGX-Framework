// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXAudioTypes.h"
#include "PGXAudioProfile.generated.h"

// Forward declarations
class USoundAttenuation;
class USoundConcurrency;

/**
 * EN: Audio Profile DataAsset — reusable playback configuration.
 *     Profiles encapsulate volume/pitch multipliers, attenuation settings,
 *     concurrency, fade durations, and 2D/3D toggle.
 *     Referenced by tag from FPGXAudioPlayParams.
 *     Auto-discovered via AssetRegistry.
 *
 * ES: DataAsset de Perfil de Audio — configuracion de reproduccion reutilizable.
 *     Los perfiles encapsulan multiplicadores de volumen/pitch, settings de atenuacion,
 *     concurrencia, duraciones de fade y toggle 2D/3D.
 *     Referenciado por tag desde FPGXAudioPlayParams.
 *     Auto-descubierto via AssetRegistry.
 */
UCLASS(BlueprintType)
class PGXAUDIORUNTIME_API UPGXAudioProfile : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	// ── Identity ──

	/** EN: Tag identifying this profile / ES: Tag que identifica este perfil */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Profile", meta = (Categories = "PGX.Audio.Profile"))
	FGameplayTag ProfileTag;

	// ── Volume & Pitch ──

	/** EN: Volume multiplier applied to sounds using this profile / ES: Multiplicador de volumen aplicado a sonidos que usan este perfil */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Profile", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.0f;

	/** EN: Pitch multiplier applied to sounds using this profile / ES: Multiplicador de pitch aplicado a sonidos que usan este perfil */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Profile", meta = (ClampMin = "0.1", ClampMax = "4.0"))
	float PitchMultiplier = 1.0f;

	// ── Spatial ──

	/** EN: Attenuation settings for 3D sounds / ES: Settings de atenuacion para sonidos 3D */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Profile|Spatial", meta = (AdvancedDisplay))
	TSoftObjectPtr<USoundAttenuation> AttenuationSettings;

	/** EN: Concurrency settings override / ES: Override de settings de concurrencia */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Profile|Concurrency", meta = (AdvancedDisplay))
	TSoftObjectPtr<USoundConcurrency> ConcurrencySettings;

	// ── Fade ──

	/** EN: Fade-in duration in seconds (0 = instant) / ES: Duracion del fade-in en segundos (0 = instantaneo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Profile|Fade", meta = (AdvancedDisplay, ClampMin = "0.0"))
	float FadeInDuration = 0.0f;

	/** EN: Fade-out duration in seconds (0 = instant) / ES: Duracion del fade-out en segundos (0 = instantaneo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Profile|Fade", meta = (AdvancedDisplay, ClampMin = "0.0"))
	float FadeOutDuration = 0.5f;

	// ── Playback Mode ──

	/** EN: Whether this profile forces 2D playback (no spatialization) / ES: Si este perfil fuerza reproduccion 2D (sin espacializacion) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Profile")
	bool bIs2D = false;
};
