// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX PSO System.
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXPSOTags.cpp.
 *     Context tags define warm-up scope; State tags reflect subsystem status.
 *     Ownership: PSO system. Context tags dev-extensible, State tags fixed.
 * ES: Tags nativos de gameplay para el Sistema PSO de PGX.
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXPSOTags.cpp.
 *     Tags de contexto definen el alcance del warm-up; tags de estado reflejan el status del subsistema.
 */

// -- Context tags (used in FPGXPSOEntry.ContextTag and warm-up filtering) --
// EN: Global context — always active, precached at game start
// ES: Contexto global — siempre activo, precacheado al inicio del juego
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_PSO_Context_Global);
// EN: Menu context — UI/menu materials
// ES: Contexto menu — materiales de UI/menu
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_PSO_Context_Menu);
// EN: Gameplay context — in-game materials
// ES: Contexto gameplay — materiales en juego
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_PSO_Context_Gameplay);

// -- State tags (for subsystem status, usable in GameFlow integration) --
// EN: PSO subsystem is idle / ES: Subsistema PSO esta inactivo
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_PSO_State_Idle);
// EN: PSO subsystem is warming up / ES: Subsistema PSO esta en warm-up
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_PSO_State_WarmingUp);
// EN: PSO warm-up complete / ES: Warm-up de PSO completado
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_PSO_State_Complete);
// EN: PSO warm-up failed / ES: Warm-up de PSO fallido
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_PSO_State_Failed);

// -- Loading bridge query tags (owned by PSO listener; broadcast by Loading via message bus) --
// EN: Loading asks PSO for a synchronous state snapshot / ES: Loading solicita snapshot sincronico de PSO
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_PSO_Loading_QueryState);
