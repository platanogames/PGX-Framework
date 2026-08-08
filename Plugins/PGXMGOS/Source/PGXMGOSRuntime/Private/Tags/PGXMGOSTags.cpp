// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Tags/PGXMGOSTags.h"

// ============================================================================
// EN: MGOS Tags — GC observability mode, state, incident, and identity tags.
//     Hierarchy: PGX.MGOS.{Category}.{Value}
//
// ES: Tags de MGOS — modo de observabilidad GC, estado, incidente e identidad.
//     Jerarquia: PGX.MGOS.{Categoria}.{Valor}
// ============================================================================

// -- Mode tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Mode_Off,       "PGX.MGOS.Mode.Off");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Mode_Passive,   "PGX.MGOS.Mode.Passive");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Mode_Snapshot,  "PGX.MGOS.Mode.Snapshot");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Mode_DeepTrack, "PGX.MGOS.Mode.DeepTrack");

// -- Profile State tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_State_Stable,                "PGX.MGOS.State.Stable");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_State_Accumulation,          "PGX.MGOS.State.Accumulation");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_State_LeakSuspected,         "PGX.MGOS.State.LeakSuspected");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_State_BurstClean,            "PGX.MGOS.State.BurstClean");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_State_PendingKillSaturation, "PGX.MGOS.State.PendingKillSaturation");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_State_RootExpansion,         "PGX.MGOS.State.RootExpansion");

// -- Incident tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Incident_LeakSuspected,           "PGX.MGOS.Incident.LeakSuspected");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Incident_PendingKillSaturation,   "PGX.MGOS.Incident.PendingKillSaturation");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Incident_BurstClean,              "PGX.MGOS.Incident.BurstClean");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Incident_RootExpansion,           "PGX.MGOS.Incident.RootExpansion");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Incident_NonUObjectLeakSuspected, "PGX.MGOS.Incident.NonUObjectLeakSuspected");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_MGOS_Incident_GCPressureWarning,       "PGX.MGOS.Incident.GCPressureWarning");

// -- System identity tag --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_System_MGOS, "PGX.System.MGOS");
