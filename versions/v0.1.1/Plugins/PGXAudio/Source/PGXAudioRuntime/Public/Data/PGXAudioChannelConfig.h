// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "GameplayTagContainer.h"
#include "Observability/PGXObservable.h"
#include "Sound/SoundClass.h"
#include "SoundControlBus.h"
#include "PGXAudioChannelConfig.generated.h"

/**
 * EN: Per-channel configuration DataAsset.
 *     Each audio channel (Music, SFX, Voice, etc.) has one of these.
 *     Contains BOTH Legacy (SoundClass) and Modulation (ControlBus) references.
 *     The active backend reads the appropriate reference, ignores the other.
 *     Same DA works for both backends — no duplication.
 *     Auto-discovered via AssetRegistry.
 *
 * ES: DataAsset de configuracion por canal.
 *     Cada canal de audio (Music, SFX, Voice, etc.) tiene uno de estos.
 *     Contiene AMBAS referencias Legacy (SoundClass) y Modulation (ControlBus).
 *     El backend activo lee la referencia apropiada, ignora la otra.
 *     El mismo DA funciona para ambos backends — sin duplicacion.
 *     Auto-descubierto via AssetRegistry.
 */
UCLASS(BlueprintType)
class PGXAUDIORUNTIME_API UPGXAudioChannelConfig : public UPGXConfigDataAsset, public IPGXObservable
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

	/** EN: Channel tag this config maps to / ES: Tag de canal al que mapea esta config */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Channel", meta = (Categories = "PGX.Audio.Channel"))
	FGameplayTag ChannelTag;

	/** EN: Display name for inspector/UI / ES: Nombre para mostrar en inspector/UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Channel")
	FText ChannelDisplayName;

	// ── Volume ──

	/** EN: Default volume for this channel (0.0-1.0) / ES: Volumen por defecto para este canal (0.0-1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Channel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultVolume = 1.0f;

	/** EN: Max concurrent sounds on this channel (0 = unlimited) / ES: Maximo de sonidos concurrentes en este canal (0 = ilimitado) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Channel", meta = (AdvancedDisplay, ClampMin = "0"))
	int32 MaxConcurrent = 0;

	// ── Legacy Backend ──

	/** EN: SoundClass for Legacy backend volume control / ES: SoundClass para control de volumen del backend Legacy */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Channel|Legacy", meta = (AdvancedDisplay))
	TSoftObjectPtr<USoundClass> SoundClassRef;

	// ── Modulation Backend ──

	/** EN: ControlBus for Modulation backend volume control / ES: ControlBus para control de volumen del backend Modulation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Channel|Modulation", meta = (AdvancedDisplay))
	TSoftObjectPtr<USoundControlBus> ControlBusRef;

	// ── Flags ──

	/** EN: Whether this channel participates in the User mix layer (player-adjustable) / ES: Si este canal participa en la capa de mezcla User (ajustable por jugador) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Channel", meta = (AdvancedDisplay))
	bool bUserAdjustable = true;

	/** EN: Whether this channel is affected by global mute / ES: Si este canal es afectado por mute global */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Channel", meta = (AdvancedDisplay))
	bool bAffectedByGlobalMute = true;
};
