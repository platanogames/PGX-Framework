// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

// EN: Bridge message tags for L1 <-> L2 communication.
//     These tags use distributed ownership and a cross-module star topology: PGXCore
//     owns them while participating plugins (Save / GameFlow /
//     Audio / UI / Loading / LevelFlow). Cross-module DLL boundary requires PGXCORERUNTIME_API
//     export so the linker can resolve the FNativeGameplayTag symbol from consumer plugins.
//     UE_DECLARE_GAMEPLAY_TAG_EXTERN does NOT add an API macro; we expand it manually with the
//     PGXCORERUNTIME_API export prefix. The matching UE_DEFINE_GAMEPLAY_TAG in
//     Private/Messages/Tags/PGXBridgeTags.cpp picks up the dllexport via header inclusion.
// ES: Tags de mensajes bridge para comunicacion L1 <-> L2. Cross-module DLL boundary requiere
//     PGXCORERUNTIME_API export para que el linker resuelva FNativeGameplayTag desde plugins
//     consumidores. Expand manual del macro porque UE_DECLARE_GAMEPLAY_TAG_EXTERN no anade API.

extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_GameFlow_StateChanged;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_LevelFlow_TransitionStarted;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_LevelFlow_TransitionCompleted;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_Loading_ScreenShown;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_Loading_ScreenClosed;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_Audio_PlayRequest;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_Audio_StopRequest;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_Save_PreSave;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_Save_PostLoad;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_Save_Register;
extern PGXCORERUNTIME_API FNativeGameplayTag TAG_PGX_Bridge_Save_Unregister;
