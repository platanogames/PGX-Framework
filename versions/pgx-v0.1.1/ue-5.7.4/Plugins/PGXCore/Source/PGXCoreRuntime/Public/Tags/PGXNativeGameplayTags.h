// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX Core Framework.
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXNativeGameplayTags.cpp.
 *     Tags for: System State, GameFlow, Generic State Machine, Save.
 *     Ownership: Core (System.State, State) | GameFlow (Phase) | Save (Save.*)
 *     Dev-extensible: GameFlow.Phase.*, Save.Domain.*, Save.Operation.*
 * ES: Tags nativos de gameplay para el Framework PGX Core.
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXNativeGameplayTags.cpp.
 *     Tags para: Estado de Sistema, GameFlow, Maquina de Estados Generica, Save.
 */

// EN: System state tags / ES: Tags de estado de sistema
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_System_State_Uninitialized);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_System_State_Initializing);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_System_State_Ready);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_System_State_ShuttingDown);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_System_State_Error);

// EN: Game flow phase tags / ES: Tags de fases de game flow
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_Phase);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_Phase_None);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_Phase_Loading);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_Phase_MainMenu);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_Phase_InGame);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_GameFlow_Phase_Paused);

// EN: Generic state machine tags / ES: Tags de maquina de estados generica
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_State);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_State_Active);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_State_Inactive);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_State_Transitioning);

// EN: Save system tags / ES: Tags del sistema de guardado
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save_Context);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save_Domain);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save_Operation);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save_Operation_Save);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save_Operation_Load);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save_Operation_Delete);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save_Operation_AutoSave);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save_Operation_QuickSave);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Save_Operation_QuickLoad);
