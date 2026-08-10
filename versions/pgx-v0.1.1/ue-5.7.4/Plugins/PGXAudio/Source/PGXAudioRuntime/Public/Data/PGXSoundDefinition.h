// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXObjectDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXAudioTypes.h"
#include "PGXSoundDefinition.generated.h"

/**
 * EN: Sound Definition DataAsset — defines a sound with context-aware variants.
 *     Tag-query resolution: the system matches caller context tags against
 *     each variant's required tags to select the appropriate sound.
 *     Auto-discovered via AssetRegistry.
 *
 * ES: DataAsset de Definicion de Sonido — define un sonido con variantes conscientes del contexto.
 *     Resolucion tag-query: el sistema compara los tags de contexto del llamador contra
 *     los tags requeridos de cada variante para seleccionar el sonido apropiado.
 *     Auto-descubierto via AssetRegistry.
 */
UCLASS(BlueprintType)
class PGXAUDIORUNTIME_API UPGXSoundDefinition : public UPGXObjectDataAsset, public IPGXObservable
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

	/** EN: Tag identifying this sound definition / ES: Tag que identifica esta definicion de sonido */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Sound", meta = (Categories = "PGX.Audio"))
	FGameplayTag SoundTag;

	// ── Variants ──

	/** EN: Sound variants with context-tag matching / ES: Variantes de sonido con matching de tags de contexto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Sound")
	TArray<FPGXSoundVariant> Variants;

	/** EN: How to select from matching variants / ES: Como seleccionar de variantes que coinciden */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Sound")
	EPGXSoundSelectionMode SelectionMode = EPGXSoundSelectionMode::Random;

	// ── Defaults ──

	/** EN: Default channel tag for this sound / ES: Tag de canal por defecto para este sonido */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Sound", meta = (Categories = "PGX.Audio.Channel"))
	FGameplayTag DefaultChannelTag;

	/** EN: Default audio profile tag / ES: Tag de perfil de audio por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Sound", meta = (AdvancedDisplay, Categories = "PGX.Audio.Profile"))
	FGameplayTag DefaultProfileTag;

	// ── Concurrency ──

	/** EN: Max concurrent instances of this sound (0 = use global config) / ES: Max instancias concurrentes de este sonido (0 = usar config global) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Sound|Concurrency", meta = (AdvancedDisplay, ClampMin = "0"))
	int32 MaxConcurrent = 0;

	/** EN: Concurrency policy for this sound / ES: Politica de concurrencia para este sonido */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Sound|Concurrency", meta = (AdvancedDisplay))
	EPGXAudioConcurrencyPolicy ConcurrencyPolicy = EPGXAudioConcurrencyPolicy::StopOldest;
};
