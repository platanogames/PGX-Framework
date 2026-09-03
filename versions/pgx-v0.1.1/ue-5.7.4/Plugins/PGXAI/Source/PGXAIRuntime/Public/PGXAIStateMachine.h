// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXAIStateMachine.generated.h"

/**
 * EN: AI-specific state machine built on FPGXStateMachine from PGXCore.
 *     Extends the generic state machine with AI-specific features like
 *     behavior tree integration, perception-driven transitions, and squad awareness.
 *
 * ES: Maquina de estados especifica para IA construida sobre FPGXStateMachine de PGXCore.
 *     Extiende la maquina de estados generica con caracteristicas especificas de IA como
 *     integracion con behavior trees, transiciones por percepcion, y awareness de squad.
 */
UCLASS()
class PGXAIRUNTIME_API UPGXAIStateMachine : public UObject
{
	GENERATED_BODY()
};
