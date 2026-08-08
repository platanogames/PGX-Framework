// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Tags/PGXPSOTags.h"

// ============================================================================
// EN: PSO System Tags — context and state identifiers for shader warm-up.
//     Hierarchy: PGX.PSO.{Category}.{Value}
//
// ES: Tags del Sistema PSO — contexto y estado para warm-up de shaders.
//     Jerarquia: PGX.PSO.{Categoria}.{Valor}
// ============================================================================

// -- Context tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_PSO_Context_Global,   "PGX.PSO.Context.Global");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_PSO_Context_Menu,     "PGX.PSO.Context.Menu");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_PSO_Context_Gameplay, "PGX.PSO.Context.Gameplay");

// -- State tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_PSO_State_Idle,      "PGX.PSO.State.Idle");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_PSO_State_WarmingUp, "PGX.PSO.State.WarmingUp");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_PSO_State_Complete,  "PGX.PSO.State.Complete");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_PSO_State_Failed,    "PGX.PSO.State.Failed");

// -- Loading bridge query tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_PSO_Loading_QueryState, "PGX.PSO.Loading.QueryState");
