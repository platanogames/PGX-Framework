// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX Log Subsystem.
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXLogTags.cpp.
 *     Used for filtering, categorization, and Config DA profile matching.
 *     Ownership: Log system. Not dev-extensible.
 * ES: Tags nativos de gameplay para el PGX Log Subsystem.
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXLogTags.cpp.
 *     Usados para filtrado, categorizacion y matching de perfiles de Config DA.
 */

// EN: Log verbosity level tags / ES: Tags de nivel de verbosity de log
// Detailed trace logging
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Verbose);
// Development diagnostics
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Debug);
// Operational messages
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Info);
// Potential issues
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Warning);
// Errors needing attention
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Error);
// Unrecoverable errors
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Fatal);

// EN: Log context tags (what system generated the log) / ES: Tags de contexto de log (que sistema genero el log)
// Core framework logs
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Context_Core);
// Editor tool logs
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Context_Editor);
// Gameplay system logs
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Context_Gameplay);
// File/network IO logs
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Context_IO);
// Multiplayer/online logs
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Context_Network);

// ═══════════════════════════════════════════════════════════════
// EN: Log Domain Tags — identify which PGX subsystem produced the log entry.
//     Used by the polymorphic Log Viewer to select domain-specific renderers.
//     Hierarchy: PGX.Log.Domain.{System}
//     Ownership: Log system. Not dev-extensible (devs create UPGXLogDomainConfig DAs instead).
//
// ES: Tags de Dominio de Log — identifican cual subsistema PGX produjo la entrada.
//     Usados por el Log Viewer polimorfico para seleccionar renderers especificos de dominio.
//     Jerarquia: PGX.Log.Domain.{Sistema}
//     Propiedad: Sistema Log. No extensible por devs (devs crean DAs UPGXLogDomainConfig).
// ═══════════════════════════════════════════════════════════════

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_Audio);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_Save);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_GameFlow);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_PSO);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_LevelFlow);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_Loading);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_Profile);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_MGOS);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_DataRegistry);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_Construction);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_Message);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_EventHandler);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Log_Domain_Generic);
