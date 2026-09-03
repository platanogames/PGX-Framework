// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags owned by PGXInput. Use RequestGameplayTag for cross-module runtime lookup.
 * ES: Tags nativos propiedad de PGXInput. Usar RequestGameplayTag para lookup runtime cross-module.
 */

// -- Context tags --
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Input_Context_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Input_Context_Menu);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Input_Context_Gameplay);

// -- Action tags --
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Input_Action_Move);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Input_Action_Look);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Input_Action_Jump);

// -- Device tags --
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Input_Device_KeyboardMouse);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Input_Device_Gamepad);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Input_Device_Touch);
