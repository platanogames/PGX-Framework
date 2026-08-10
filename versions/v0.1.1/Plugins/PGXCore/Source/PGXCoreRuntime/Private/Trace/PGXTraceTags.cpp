// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Trace/PGXTraceTags.h"

// ============================================================================
// EN: System Identification Tags — one per PGX plugin/system.
//     Hierarchy: PGX.System.{SystemName}
//     Used by: traceability filtering, System Observer Panel, Log Viewer.
//
// ES: Tags de Identificacion de Sistema — uno por plugin/sistema PGX.
//     Jerarquia: PGX.System.{NombreSistema}
//     Usados por: filtrado de trazabilidad, Panel Observer, Log Viewer.
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Core,        "PGX.System.Core");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Save,        "PGX.System.Save");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_GameFlow,    "PGX.System.GameFlow");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_PSO,         "PGX.System.PSO");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Log,         "PGX.System.Log");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Loading,     "PGX.System.Loading");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Audio,       "PGX.System.Audio");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Input,       "PGX.System.Input");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Camera,      "PGX.System.Camera");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_UI,          "PGX.System.UI");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Interaction, "PGX.System.Interaction");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Inventory,   "PGX.System.Inventory");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Ability,     "PGX.System.Ability");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Spawn,       "PGX.System.Spawn");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_AI,          "PGX.System.AI");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Online,      "PGX.System.Online");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Animation,   "PGX.System.Animation");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_VFX,         "PGX.System.VFX");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_Materials,   "PGX.System.Materials");

// ============================================================================
// EN: Trace State Tags — mark operation lifecycle phases.
//     Hierarchy: PGX.Trace.State.{Phase}
//     Used by: FPGXScopedTrace RAII guard, Log Viewer tag filtering.
//
// ES: Tags de Estado de Traza — marcan fases del ciclo de vida de operacion.
//     Jerarquia: PGX.Trace.State.{Fase}
//     Usados por: guardia RAII FPGXScopedTrace, filtrado por tag en Log Viewer.
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Trace_State_Enter,   "PGX.Trace.State.Enter");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Trace_State_Exit,    "PGX.Trace.State.Exit");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Trace_State_Success, "PGX.Trace.State.Success");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Trace_State_Error,   "PGX.Trace.State.Error");
