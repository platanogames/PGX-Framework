// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX Loading Screen System.
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXLoadingTags.cpp.
 *     Context tags trigger loading by scenario; State tags reflect loading screen status.
 *     Ownership: Loading system. Context tags dev-extensible, State tags fixed.
 * ES: Tags nativos de gameplay para el Sistema de Pantalla de Carga PGX.
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXLoadingTags.cpp.
 *     Tags de contexto activan la carga por escenario; tags de estado reflejan el status.
 */

// -- Context tags (trigger loading by context) --
// EN: Default/generic loading context / ES: Contexto de carga por defecto/generico
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_Context_Default);
// EN: Level change transition / ES: Transicion de cambio de nivel
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_Context_LevelTransition);
// EN: Network/matchmaking wait / ES: Espera de red/matchmaking
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_Context_NetworkWait);
// EN: Heavy streaming without level change / ES: Streaming pesado sin cambio de nivel
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_Context_Streaming);
// EN: Subsystem initialization wait / ES: Espera de inicializacion de subsistema
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_Context_Initialization);

// -- State tags (current loading screen state) --
// EN: Loading screen is idle (no active screen) / ES: Pantalla de carga en reposo (sin pantalla activa)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_State_Idle);
// EN: Loading screen is active and visible / ES: Pantalla de carga activa y visible
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_State_Active);
// EN: Loading screen is waiting for close conditions / ES: Pantalla de carga esperando condiciones de cierre
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_State_WaitingClose);
// EN: Loading screen completed successfully / ES: Pantalla de carga completada exitosamente
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_State_Complete);

// -- PSO bridge tags (owned by Loading listener; broadcast by PSO via message bus) --
// EN: PSO state snapshot for Loading wait gating / ES: Snapshot PSO para gate de espera de Loading
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_PSO_State);
// EN: PSO warm-up progress for Loading combined progress / ES: Progreso PSO para progreso combinado de Loading
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_PSO_Progress);
// EN: PSO warm-up completion for Loading close gate / ES: Completitud PSO para gate de cierre de Loading
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_PSO_Complete);
