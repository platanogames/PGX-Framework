// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

// EN: Native gameplay tags for the Test Harness — MUST live in a Runtime module
//     (Editor modules cannot define native tags; UE requires Client/Server tag parity)
// ES: Tags nativos de gameplay para el Test Harness — DEBEN vivir en modulo Runtime
//     (Modulos Editor no pueden definir native tags; UE requiere paridad Client/Server)

#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_Audio_SFX,    "PGX.Test.Audio.Channel.SFX");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_Audio_Music,  "PGX.Test.Audio.Channel.Music");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_Save_Context, "PGX.Test.Save.Context");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_Save_Domain,  "PGX.Test.Save.Domain.Main");

// EN: EventHandler test tags — used by PGXEventHandlerTestUtility via RequestGameplayTag()
// ES: Tags de test de EventHandler — usados por PGXEventHandlerTestUtility via RequestGameplayTag()
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Quick,     "PGX.Test.EventHandler.Quick");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Singleton, "PGX.Test.EventHandler.Singleton");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Cached,    "PGX.Test.EventHandler.Cached");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Ephemeral, "PGX.Test.EventHandler.Ephemeral");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Payload,   "PGX.Test.EventHandler.Payload");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Condition, "PGX.Test.EventHandler.Condition");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Seq1,      "PGX.Test.EventHandler.Seq1");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Seq2,      "PGX.Test.EventHandler.Seq2");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Seq3,      "PGX.Test.EventHandler.Seq3");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_EventHandler_Cache,     "PGX.Test.EventHandler.Cache");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PGXTest_Allow,                  "PGX.Test.Allow");
