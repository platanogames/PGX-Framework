// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Data/PGXObjectDataAsset.h"
#include "Observability/PGXObservable.h"
#include "PGXAudioTypes.h"
#include "PGXMusicPlaylist.generated.h"

/**
 * EN: Music Playlist DataAsset — ordered collection of music tracks.
 *     Supports loop, shuffle, and state-based activation via GameFlow.
 *     Auto-discovered via AssetRegistry.
 *
 * ES: DataAsset de Playlist Musical — coleccion ordenada de pistas de musica.
 *     Soporta loop, shuffle y activacion basada en estado via GameFlow.
 *     Auto-descubierto via AssetRegistry.
 */
UCLASS(BlueprintType)
class PGXAUDIORUNTIME_API UPGXMusicPlaylist : public UPGXObjectDataAsset, public IPGXObservable
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

	/** EN: Tag identifying this playlist / ES: Tag que identifica este playlist */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music",
		meta = (Categories = "PGX.Audio"))
	FGameplayTag PlaylistTag;

	// ── Tracks ──

	/** EN: Ordered list of music tracks / ES: Lista ordenada de pistas de musica */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music")
	TArray<FPGXMusicTrack> Tracks;

	// ── Playback ──

	/** EN: Loop the playlist when it reaches the end / ES: Hacer loop del playlist al llegar al final */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music")
	bool bLoop = true;

	/** EN: Shuffle track order on each loop / ES: Mezclar orden de pistas en cada loop */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music")
	bool bShuffle = false;

	// ── GameFlow Integration ──

	/** EN: GameFlow state tag that auto-activates this playlist / ES: Tag de estado GameFlow que auto-activa este playlist */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music|GameFlow",
		meta = (AdvancedDisplay, Categories = "PGX.GameFlow.Phase"))
	FGameplayTag ActivationStateTag;

	/** EN: Default crossfade duration between tracks (seconds) / ES: Duracion de crossfade por defecto entre pistas (segundos) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Audio|Music", meta = (AdvancedDisplay, ClampMin = "0.0"))
	float DefaultCrossfadeDuration = 2.0f;
};
