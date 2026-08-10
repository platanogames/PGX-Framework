// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Tags/PGXLoadingTags.h"

// ============================================================================
// EN: Loading Screen Tags — context and state identifiers.
//     Hierarchy: PGX.Loading.{Category}.{Value}
//
// ES: Tags de Pantalla de Carga — contexto y estado.
//     Jerarquia: PGX.Loading.{Categoria}.{Valor}
// ============================================================================

// -- Context tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_Context_Default,         "PGX.Loading.Context.Default");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_Context_LevelTransition, "PGX.Loading.Context.LevelTransition");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_Context_NetworkWait,     "PGX.Loading.Context.NetworkWait");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_Context_Streaming,       "PGX.Loading.Context.Streaming");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_Context_Initialization,  "PGX.Loading.Context.Initialization");

// -- State tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_State_Idle,         "PGX.Loading.State.Idle");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_State_Active,       "PGX.Loading.State.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_State_WaitingClose, "PGX.Loading.State.WaitingClose");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_State_Complete,     "PGX.Loading.State.Complete");

// -- PSO bridge tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_PSO_State,    "PGX.Loading.PSO.State");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_PSO_Progress, "PGX.Loading.PSO.Progress");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Loading_PSO_Complete, "PGX.Loading.PSO.Complete");
