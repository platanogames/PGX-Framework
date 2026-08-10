// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

// EN: Framework-owned canonical GameFlow lifecycle states.
//     Projects may extend PGX.GameFlow.State.* but must not replace these states.
// ES: Estados canonicos de ciclo de vida GameFlow propiedad del framework.
//     Los proyectos pueden extender PGX.GameFlow.State.* pero no reemplazar estos estados.

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_State);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_State_Boot);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_State_MainMenu);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_State_Loading);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_State_InWorld);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_State_Paused);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_State_Sleeping);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_State_Cinematic);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_State_Shutdown);

// EN: Reserved metadata tags for transition-source classification. They remain
//     unconsumed until bridge payloads expose matching transition metadata fields.
// ES: Tags de metadata reservados para clasificar el origen de transiciones. No se
//     consumen hasta que los payloads bridge expongan campos de metadata equivalentes.

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_TransitionSource);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_TransitionSource_Player);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_TransitionSource_System);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_TransitionSource_Save_Restore);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_TransitionSource_Console);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_TransitionSource_Editor);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_TransitionSource_Project);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_TransitionSource_Debug);

// EN: Loading -> GameFlow command channels. Declared in GameFlow to avoid a reverse L2 dependency from GameFlow to Loading.
// ES: Canales de comando Loading -> GameFlow. Declarados en GameFlow para evitar dependencia L2 inversa.
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_GameFlow_SetState);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Loading_GameFlow_Revert);
