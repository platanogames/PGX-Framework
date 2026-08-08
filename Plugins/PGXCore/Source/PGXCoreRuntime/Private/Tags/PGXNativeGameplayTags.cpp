// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Tags/PGXNativeGameplayTags.h"

// ============================================================================
// EN: System State Tags — PGX subsystem lifecycle states.
//     Hierarchy: PGX.System.State.{Phase}
//     Used by: subsystem state machine, health dashboard, test harness.
//
// ES: Tags de Estado de Sistema — estados del ciclo de vida de subsistemas PGX.
//     Jerarquia: PGX.System.State.{Fase}
//     Usados por: maquina de estados de subsistema, dashboard de salud, test harness.
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_State_Uninitialized, "PGX.System.State.Uninitialized");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_State_Initializing,  "PGX.System.State.Initializing");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_State_Ready,         "PGX.System.State.Ready");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_State_ShuttingDown,  "PGX.System.State.ShuttingDown");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_State_Error,         "PGX.System.State.Error");

// ============================================================================
// EN: Game Flow Phase Tags — GameFlow state machine phases.
//     Hierarchy: PGX.GameFlow.Phase.{PhaseName}
//
// ES: Tags de Fases de Game Flow — fases de la maquina de estados de GameFlow.
//     Jerarquia: PGX.GameFlow.Phase.{NombreFase}
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_GameFlow_Phase,          "PGX.GameFlow.Phase");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_GameFlow_Phase_None,     "PGX.GameFlow.Phase.None");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_GameFlow_Phase_Loading,  "PGX.GameFlow.Phase.Loading");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_GameFlow_Phase_MainMenu, "PGX.GameFlow.Phase.MainMenu");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_GameFlow_Phase_InGame,   "PGX.GameFlow.Phase.InGame");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_GameFlow_Phase_Paused,   "PGX.GameFlow.Phase.Paused");

// ============================================================================
// EN: Generic State Machine Tags — reusable state identifiers.
//     Hierarchy: PGX.State.{StateName}
//
// ES: Tags de Maquina de Estados Generica — identificadores de estado reutilizables.
//     Jerarquia: PGX.State.{NombreEstado}
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_State,               "PGX.State");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_State_Active,        "PGX.State.Active");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_State_Inactive,      "PGX.State.Inactive");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_State_Transitioning, "PGX.State.Transitioning");

// ============================================================================
// EN: Save System Tags — save context, domain, and operation identifiers.
//     Hierarchy: PGX.Save.{Category}.{Value}
//
// ES: Tags del Sistema de Guardado — contexto, dominio y operaciones.
//     Jerarquia: PGX.Save.{Categoria}.{Valor}
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save,                     "PGX.Save");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save_Context,             "PGX.Save.Context");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save_Domain,              "PGX.Save.Domain");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save_Operation,           "PGX.Save.Operation");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save_Operation_Save,      "PGX.Save.Operation.Save");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save_Operation_Load,      "PGX.Save.Operation.Load");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save_Operation_Delete,    "PGX.Save.Operation.Delete");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save_Operation_AutoSave,  "PGX.Save.Operation.AutoSave");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save_Operation_QuickSave, "PGX.Save.Operation.QuickSave");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Save_Operation_QuickLoad, "PGX.Save.Operation.QuickLoad");
