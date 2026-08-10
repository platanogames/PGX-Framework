// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "EventHandler/PGXEventHandlerBase.h"
#include "PGXAutoSaveHandler.generated.h"

/**
 * EN: Built-in handler that triggers an auto-save through the PGX Save Subsystem.
 *     Lifecycle: Singleton. Category: Core.
 * ES: Handler built-in que dispara un auto-save a traves del PGX Save Subsystem.
 *     Ciclo de vida: Singleton. Categoria: Core.
 */
UCLASS(BlueprintType)
class PGXCORERUNTIME_API UPGXAutoSaveHandler : public UPGXEventHandlerBase
{
	GENERATED_BODY()

public:
	EPGXEventResult Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& Payload) override;
};
