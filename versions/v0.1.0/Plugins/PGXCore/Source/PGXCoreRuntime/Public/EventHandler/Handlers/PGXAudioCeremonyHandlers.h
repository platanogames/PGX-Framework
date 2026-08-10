// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "EventHandler/PGXEventHandlerBase.h"
#include "PGXAudioCeremonyHandlers.generated.h"

/**
 * EN: Built-in handler that pauses audio through the PGX Audio Subsystem.
 *     Intended for use in flow ceremonies (pause menu, level transitions).
 *     Lifecycle: Singleton. Category: Audio.
 * ES: Handler built-in que pausa audio a traves del PGX Audio Subsystem.
 *     Pensado para ceremonias de flujo (menu de pausa, transiciones de nivel).
 *     Ciclo de vida: Singleton. Categoria: Audio.
 */
UCLASS(BlueprintType)
class PGXCORERUNTIME_API UPGXPauseAudioHandler : public UPGXEventHandlerBase
{
	GENERATED_BODY()

public:
	EPGXEventResult Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& Payload) override;
};

/**
 * EN: Built-in handler that resumes audio through the PGX Audio Subsystem.
 *     Intended for use in flow ceremonies (unpause, level ready).
 *     Lifecycle: Singleton. Category: Audio.
 * ES: Handler built-in que reanuda audio a traves del PGX Audio Subsystem.
 *     Pensado para ceremonias de flujo (unpause, nivel listo).
 *     Ciclo de vida: Singleton. Categoria: Audio.
 */
UCLASS(BlueprintType)
class PGXCORERUNTIME_API UPGXResumeAudioHandler : public UPGXEventHandlerBase
{
	GENERATED_BODY()

public:
	EPGXEventResult Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& Payload) override;
};
