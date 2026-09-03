// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "EventHandler/PGXEventHandlerBase.h"
#include "EventHandler/PGXEventHandlerTypes.h"

#include "PGXHarnessHandlerStub.generated.h"

/**
 * EN: Private compatibility fixture used to verify event-handler registration.
 *     Always returns Success and avoids instantiating the abstract base type.
 *
 * ES: Fixture privado de compatibilidad para verificar el registro de handlers.
 *     Siempre retorna Success y evita instanciar el tipo base abstracto.
 */
UCLASS(Transient, Hidden, NotBlueprintable)
class UPGXHarnessHandlerStub : public UPGXEventHandlerBase
{
	GENERATED_BODY()

public:
	EPGXEventResult Execute_Implementation(const FPGXEventContext& /*Context*/, const FInstancedStruct& /*Payload*/) override
	{
		return EPGXEventResult::Success;
	}
};
