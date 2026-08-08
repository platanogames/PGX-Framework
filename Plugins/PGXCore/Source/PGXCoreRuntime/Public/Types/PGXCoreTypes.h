// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXCoreTypes.generated.h"

/**
 * EN: Central types header for PGX framework. Contains shared enumerations,
 *     type aliases, forward declarations, and common structures used across all PGX plugins.
 * ES: Header central de tipos para el framework PGX. Contiene enumeraciones compartidas,
 *     alias de tipos, forward declarations, y estructuras comunes usadas en todos los plugins PGX.
 */

/** EN: Result of a PGX operation / ES: Resultado de una operacion PGX */
UENUM(BlueprintType)
enum class EPGXOperationResult : uint8
{
	Success,
	Failed,
	Pending,
	Cancelled
};

/** EN: System initialization state / ES: Estado de inicializacion del sistema */
UENUM(BlueprintType)
enum class EPGXSystemState : uint8
{
	Uninitialized,
	Initializing,
	Ready,
	ShuttingDown,
	Error
};

/** EN: Priority levels for PGX systems / ES: Niveles de prioridad para sistemas PGX */
UENUM(BlueprintType)
enum class EPGXPriority : uint8
{
	Low,
	Normal,
	High,
	Critical
};
