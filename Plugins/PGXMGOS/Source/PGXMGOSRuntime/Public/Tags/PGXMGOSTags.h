// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX MGOS (GC Observability System).
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXMGOSTags.cpp.
 *     Mode tags reflect observer operating mode; State tags map to EPGXGCProfileState;
 *     Incident tags categorize detected anomalies; System tag identifies MGOS.
 *     Ownership: MGOS system. Not dev-extensible.
 * ES: Tags nativos de gameplay para el PGX MGOS (Sistema de Observabilidad GC).
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXMGOSTags.cpp.
 *     Tags de modo reflejan el modo de operacion; tags de estado mapean a EPGXGCProfileState;
 *     tags de incidente categorizan anomalias detectadas; tag de sistema identifica MGOS.
 */

// -- Mode tags --
// EN: Observer is disabled / ES: Observador desactivado
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Mode_Off);
// EN: Lightweight monitoring, zero iteration / ES: Monitoreo ligero, sin iteracion
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Mode_Passive);
// EN: Per-class tracking via TrackedClasses / ES: Rastreo por clase via TrackedClasses
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Mode_Snapshot);
// EN: Full analysis with global iteration / ES: Analisis completo con iteracion global
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Mode_DeepTrack);

// -- Profile State tags --
// EN: Normal operation / ES: Operacion normal
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_State_Stable);
// EN: Growing UObject count over baseline / ES: Conteo UObject creciendo sobre baseline
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_State_Accumulation);
// EN: Persistent growth confirmed / ES: Crecimiento persistente confirmado
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_State_LeakSuspected);
// EN: Massive destruction in single cycle / ES: Destruccion masiva en un solo ciclo
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_State_BurstClean);
// EN: High PendingKill ratio / ES: Alta proporcion PendingKill
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_State_PendingKillSaturation);
// EN: Root set expanding / ES: Root set expandiendose
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_State_RootExpansion);

// -- Incident tags --
// EN: Memory leak suspected / ES: Sospecha de leak de memoria
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Incident_LeakSuspected);
// EN: PendingKill saturation / ES: Saturacion de PendingKill
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Incident_PendingKillSaturation);
// EN: Burst destruction event / ES: Evento de destruccion por rafaga
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Incident_BurstClean);
// EN: Root set expansion / ES: Expansion del root set
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Incident_RootExpansion);
// EN: Non-UObject memory leak suspected [R1] / ES: Sospecha de leak de memoria no-UObject [R1]
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Incident_NonUObjectLeakSuspected);
// EN: GC pressure warning from inter-cycle monitoring [R5] / ES: Advertencia de presion GC del monitoreo inter-ciclo [R5]
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_MGOS_Incident_GCPressureWarning);

// -- System identity tag --
// EN: MGOS system identifier / ES: Identificador del sistema MGOS
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_System_MGOS);
