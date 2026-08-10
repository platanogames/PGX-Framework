// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX Event Handler System.
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXEventHandlerTags.cpp.
 *     Root tags for event categories and handler grouping.
 *     Ownership: EventHandler system. Dev-extensible (generic bus).
 * ES: Tags nativos de gameplay para el Sistema de Event Handler PGX.
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXEventHandlerTags.cpp.
 *     Tags raiz para categorias de eventos y agrupacion de handlers.
 */

// EN: Root event tags / ES: Tags raiz de eventos
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event_Item);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event_Interact);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event_Flow);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event_UI);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Event_System);

// EN: Handler category tags / ES: Tags de categoria de handlers
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Category);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Category_Core);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Category_Gameplay);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Category_Audio);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Category_UI);

// EN: Failure taxonomy / ES: Taxonomia de fallos
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Validation_InvalidEventTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Validation_PayloadMismatch);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Validation_HandlerDisabled);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Validation_CanExecuteFalse);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Result_HandlerNotFound);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Result_AcquireHandlerFailed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Result_ChainBudgetExceeded);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_EventHandler_Result_ChainCycle);
