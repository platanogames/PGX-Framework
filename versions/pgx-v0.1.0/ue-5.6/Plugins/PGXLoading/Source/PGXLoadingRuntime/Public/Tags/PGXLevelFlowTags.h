// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX LevelFlow System.
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXLevelFlowTags.cpp.
 *     Level tags identify loadable levels; State tags reflect transition status.
 *     Ownership: LevelFlow system. State tags fixed, not dev-extensible.
 * ES: Tags nativos de gameplay para el Sistema LevelFlow de PGX.
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXLevelFlowTags.cpp.
 *     Tags de nivel identifican niveles cargables; tags de estado reflejan status de transicion.
 */

// -- State tags (transition pipeline states) --
// EN: LevelFlow subsystem is idle (no transition active) / ES: Subsistema LevelFlow en reposo (sin transicion activa)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_LevelFlow_State_Idle);
// EN: Level transition is loading / ES: Transicion de nivel esta cargando
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_LevelFlow_State_Loading);
// EN: Level transition is in PostLoad wait / ES: Transicion de nivel en espera PostLoad
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_LevelFlow_State_PostLoad);
// EN: Level transition completed successfully / ES: Transicion de nivel completada exitosamente
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_LevelFlow_State_Complete);
// EN: Level transition failed / ES: Transicion de nivel fallida
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_LevelFlow_State_Failed);
