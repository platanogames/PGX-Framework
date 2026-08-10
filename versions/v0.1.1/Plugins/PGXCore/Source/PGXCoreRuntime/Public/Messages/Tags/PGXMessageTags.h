// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX Message System.
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXMessageTags.cpp.
 *     Root tags: PGX.Message, PGX.Message.System, PGX.Message.Gameplay
 *     Ownership: Message system. Dev-extensible (generic bus).
 * ES: Tags nativos de gameplay para el Sistema de Mensajes PGX.
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXMessageTags.cpp.
 *     Tags raiz: PGX.Message, PGX.Message.System, PGX.Message.Gameplay
 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Message);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Message_System);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Message_System_Startup);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Message_System_Shutdown);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Message_System_Error);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Message_Gameplay);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Message_UI);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Message_Audio);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Message_Debug);
