// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Tags/PGXLevelFlowTags.h"

// ============================================================================
// EN: LevelFlow State Tags — transition pipeline state identifiers.
//     Hierarchy: PGX.LevelFlow.State.{Phase}
//
// ES: Tags de Estado de LevelFlow — identificadores de estado del pipeline de transicion.
//     Jerarquia: PGX.LevelFlow.State.{Fase}
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_LevelFlow_State_Idle,     "PGX.LevelFlow.State.Idle");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_LevelFlow_State_Loading,  "PGX.LevelFlow.State.Loading");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_LevelFlow_State_PostLoad, "PGX.LevelFlow.State.PostLoad");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_LevelFlow_State_Complete, "PGX.LevelFlow.State.Complete");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_LevelFlow_State_Failed,   "PGX.LevelFlow.State.Failed");
