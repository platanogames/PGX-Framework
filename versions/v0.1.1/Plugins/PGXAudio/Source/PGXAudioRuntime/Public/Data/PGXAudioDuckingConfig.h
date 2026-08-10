// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXConfigDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXAudioTypes.h"
#include "PGXAudioDuckingConfig.generated.h"

/**
 * EN: Data-driven ducking configuration.
 *     Contains an array of FPGXDuckingRule entries defining
 *     which channels duck which, with attack/release times.
 *     Can be referenced globally (AudioConfig) or per-level (LevelAudioConfig).
 *
 * ES: Configuracion de ducking data-driven.
 *     Contiene un array de entradas FPGXDuckingRule definiendo
 *     que canales atenuan a cuales, con tiempos de attack/release.
 *     Puede referenciarse globalmente (AudioConfig) o por nivel (LevelAudioConfig).
 */
UCLASS(BlueprintType)
class PGXAUDIORUNTIME_API UPGXAudioDuckingConfig : public UPGXConfigDataAsset, public IPGXObservable
{
	GENERATED_BODY()

public:
	//~ Begin IPGXObservable
	virtual FPGXJsonValue ToJson() const override;
	virtual FPGXValidationResult FromJson(const FPGXJsonValue& Json) override;
	virtual FName GetSchemaVersion() const override;
	virtual FPGXSchemaDescriptor GetSchemaDescriptor() const override;
	//~ End IPGXObservable

	/** EN: Ducking rules to evaluate / ES: Reglas de ducking a evaluar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Ducking")
	TArray<FPGXDuckingRule> Rules;
};
