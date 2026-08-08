// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "EventHandler/Tags/PGXEventHandlerTags.h"

// ============================================================================
// EN: Event Handler Tags — root event categories and handler grouping.
//     Hierarchy: PGX.Event.{Category}, PGX.EventHandler.Category.{Group}
//
// ES: Tags de Event Handler — categorias raiz de eventos y agrupacion de handlers.
//     Jerarquia: PGX.Event.{Categoria}, PGX.EventHandler.Category.{Grupo}
// ============================================================================

// -- Root event tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Event,          "PGX.Event");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Event_Item,     "PGX.Event.Item");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Event_Interact, "PGX.Event.Interact");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Event_Flow,     "PGX.Event.Flow");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Event_UI,       "PGX.Event.UI");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Event_System,   "PGX.Event.System");

// -- Handler category tags --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_EventHandler_Category,          "PGX.EventHandler.Category");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_EventHandler_Category_Core,     "PGX.EventHandler.Category.Core");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_EventHandler_Category_Gameplay, "PGX.EventHandler.Category.Gameplay");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_EventHandler_Category_Audio,    "PGX.EventHandler.Category.Audio");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_EventHandler_Category_UI,       "PGX.EventHandler.Category.UI");
