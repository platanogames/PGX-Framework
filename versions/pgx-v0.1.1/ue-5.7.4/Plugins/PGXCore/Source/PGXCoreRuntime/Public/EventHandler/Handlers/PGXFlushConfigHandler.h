// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "EventHandler/PGXEventHandlerBase.h"
#include "PGXFlushConfigHandler.generated.h"

/**
 * EN: Built-in handler that flushes the config system (GConfig->Flush).
 *     Useful after runtime setting changes.
 *     Lifecycle: Singleton. Category: Core.
 * ES: Handler built-in que hace flush del sistema de config (GConfig->Flush).
 *     Util despues de cambios de settings en runtime.
 *     Ciclo de vida: Singleton. Categoria: Core.
 */
UCLASS(BlueprintType)
class PGXCORERUNTIME_API UPGXFlushConfigHandler : public UPGXEventHandlerBase
{
	GENERATED_BODY()

public:
	EPGXEventResult Execute_Implementation(const FPGXEventContext& Context, const FInstancedStruct& Payload) override;
};
