// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Logging/PGXLogTags.h"

// ============================================================================
// EN: Log Verbosity Level Tags — used for filtering and Config DA matching.
//     Hierarchy: PGX.Log.{Level}
//
// ES: Tags de Nivel de Verbosity de Log — usados para filtrado y matching de Config DA.
//     Jerarquia: PGX.Log.{Nivel}
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Verbose, "PGX.Log.Verbose");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Debug,   "PGX.Log.Debug");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Info,    "PGX.Log.Info");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Warning, "PGX.Log.Warning");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Error,   "PGX.Log.Error");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Fatal,   "PGX.Log.Fatal");

// ============================================================================
// EN: Log Context Tags — identify the origin system of a log entry.
//     Hierarchy: PGX.Log.Context.{Source}
//
// ES: Tags de Contexto de Log — identifican el sistema de origen de un log.
//     Jerarquia: PGX.Log.Context.{Fuente}
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Context_Core,     "PGX.Log.Context.Core");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Context_Editor,   "PGX.Log.Context.Editor");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Context_Gameplay, "PGX.Log.Context.Gameplay");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Context_IO,       "PGX.Log.Context.IO");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Context_Network,  "PGX.Log.Context.Network");

// ============================================================================
// EN: Log Domain Tags — identify the PGX subsystem that produced the log entry.
//     Hierarchy: PGX.Log.Domain.{System}
//
// ES: Tags de Dominio de Log — identifican el subsistema PGX que produjo la entrada.
//     Jerarquia: PGX.Log.Domain.{Sistema}
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_Audio,        "PGX.Log.Domain.Audio");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_Save,         "PGX.Log.Domain.Save");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_GameFlow,     "PGX.Log.Domain.GameFlow");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_PSO,          "PGX.Log.Domain.PSO");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_LevelFlow,    "PGX.Log.Domain.LevelFlow");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_Loading,      "PGX.Log.Domain.Loading");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_Profile,      "PGX.Log.Domain.Profile");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_MGOS,         "PGX.Log.Domain.MGOS");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_DataRegistry, "PGX.Log.Domain.DataRegistry");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_Construction, "PGX.Log.Domain.Construction");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_Message,      "PGX.Log.Domain.Message");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_EventHandler, "PGX.Log.Domain.EventHandler");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Log_Domain_Generic,      "PGX.Log.Domain.Generic");
