// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Messages/Tags/PGXMessageTags.h"

// ============================================================================
// EN: Message System Tags — root categories for the PGX pub/sub message bus.
//     Hierarchy: PGX.Message.{Category}
//
// ES: Tags del Sistema de Mensajes — categorias raiz para el bus pub/sub PGX.
//     Jerarquia: PGX.Message.{Categoria}
// ============================================================================

UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Message,                 "PGX.Message");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Message_System,          "PGX.Message.System");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Message_System_Startup,  "PGX.Message.System.Startup");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Message_System_Shutdown, "PGX.Message.System.Shutdown");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Message_System_Error,    "PGX.Message.System.Error");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Message_Gameplay,        "PGX.Message.Gameplay");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Message_UI,              "PGX.Message.UI");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Message_Audio,           "PGX.Message.Audio");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Message_Debug,           "PGX.Message.Debug");
