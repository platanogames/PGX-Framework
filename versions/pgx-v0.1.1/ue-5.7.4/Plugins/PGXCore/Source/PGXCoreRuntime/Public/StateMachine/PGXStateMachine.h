// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXStateMachine.generated.h"

/**
 * EN: State definition for the PGX State Machine.
 * ES: Definicion de estado para la Maquina de Estados PGX.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXState
{
	GENERATED_BODY()

	/** EN: Tag identifying this state / ES: Tag que identifica este estado */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StateTag;

	/** EN: Human-readable name for this state / ES: Nombre legible para este estado */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StateName;
};

/**
 * EN: Transition rule between states.
 * ES: Regla de transicion entre estados.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXTransition
{
	GENERATED_BODY()

	/** EN: State to transition from / ES: Estado desde el cual transicionar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag FromState;

	/** EN: State to transition to / ES: Estado al cual transicionar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ToState;

	/** EN: Priority for conflict resolution / ES: Prioridad para resolucion de conflictos */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

/**
 * EN: Generic state machine driven by Gameplay Tags.
 *     Reusable for game flow, AI, UI, animations, and any
 *     system that needs state management.
 * ES: Maquina de estados generica dirigida por Gameplay Tags.
 *     Reutilizable para game flow, AI, UI, animaciones, y cualquier
 *     sistema que necesite gestion de estados.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXStateMachine
{
	GENERATED_BODY()

	/** EN: All available states / ES: Todos los estados disponibles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPGXState> States;

	/** EN: All defined transitions / ES: Todas las transiciones definidas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPGXTransition> Transitions;

	/** EN: Currently active state / ES: Estado actualmente activo */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag CurrentState;
};
