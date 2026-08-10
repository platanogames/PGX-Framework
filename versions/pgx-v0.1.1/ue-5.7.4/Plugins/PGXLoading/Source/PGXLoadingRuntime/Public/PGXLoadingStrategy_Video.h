// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXLoadingStrategyBase.h"
#include "PGXLoadingStrategy_Video.generated.h"

class UMediaPlayer;
class UMediaSource;
class UMediaTexture;

/**
 * EN: Video loading strategy — plays a video from the profile's VideoSource via MediaPlayer.
 *     Creates MediaPlayer on demand, handles codec failures with fallback to Image strategy behavior.
 *     MediaPlayer is explicitly closed before releasing reference.
 *
 * ES: Estrategia de video — reproduce un video desde el VideoSource del perfil via MediaPlayer.
 *     Crea MediaPlayer bajo demanda, maneja fallos de codec con fallback a comportamiento Image.
 *     MediaPlayer se cierra explicitamente antes de liberar referencia.
 */
UCLASS()
class PGXLOADINGRUNTIME_API UPGXLoadingStrategy_Video : public UPGXLoadingStrategyBase
{
	GENERATED_BODY()

public:
	void InitializeStrategy(UPGXLoadingProfile* Profile) override;
	void Activate(UPGXLoadingWidget* Widget) override;
	void UpdateVisual(float DeltaTime) override;
	void Deactivate() override;
	FText GetCurrentTip() const override;

private:
	void OnVideoSourceLoaded();

	UPROPERTY()
	TObjectPtr<UMediaPlayer> Player;

	UPROPERTY()
	TObjectPtr<UMediaSource> LoadedSource;

	UPROPERTY()
	TObjectPtr<UMediaTexture> MediaTex;

	bool bVideoReady = false;
	int32 CurrentTipIndex = 0;
	float TipTimer = 0.0f;
};
