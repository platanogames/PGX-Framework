// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// ============================================================================
// EN: System Identification and Trace State Tags for PGX Framework.
//     System tags: one per PGX plugin/system, used for filtering and identification.
//     Trace state tags: mark operation phases (Enter/Exit/Success/Error).
//
//     These complement PGXNativeGameplayTags.h (system state) and PGXLogTags.h (log context).
//
// ES: Tags de Identificacion de Sistema y Estado de Traza para el Framework PGX.
//     Tags de sistema: uno por plugin/sistema PGX, para filtrado e identificacion.
//     Tags de estado de traza: marcan fases de operacion (Enter/Exit/Success/Error).
//
//     Complementan PGXNativeGameplayTags.h (estado de sistema) y PGXLogTags.h (contexto de log).
// ============================================================================

// ────────────────────────────────────────────────────────────
// EN: System Identification Tags (one per PGX system)
//     Declared with PGXCORERUNTIME_API for cross-DLL visibility (L2 plugins).
// ES: Tags de Identificacion de Sistema (uno por sistema PGX)
//     Declarados con PGXCORERUNTIME_API para visibilidad cross-DLL (plugins L2).
// ────────────────────────────────────────────────────────────

extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Core;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Save;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_GameFlow;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_PSO;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Log;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Loading;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Audio;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Input;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Camera;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_UI;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Interaction;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Inventory;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Ability;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Spawn;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_AI;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Online;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Animation;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_VFX;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_System_Materials;

// ────────────────────────────────────────────────────────────
// EN: Trace State Tags (operation lifecycle)
//     Also exported for L2 plugins that use FPGXScopedTrace or TraceHelper.
// ES: Tags de Estado de Traza (ciclo de vida de operacion)
//     Tambien exportados para plugins L2 que usen FPGXScopedTrace o TraceHelper.
// ────────────────────────────────────────────────────────────

extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Trace_State_Enter;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Trace_State_Exit;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Trace_State_Success;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Trace_State_Error;
