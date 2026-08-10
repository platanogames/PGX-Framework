// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Profile/PGXProfileTags.h"

// ============================================================================
// EN: Profile System Tags — mode, platform, and state identifiers.
//     Hierarchy: PGX.Profile.{Category}.{Value}
//
// ES: Tags del Sistema de Profile — modo, plataforma y estado.
//     Jerarquia: PGX.Profile.{Categoria}.{Valor}
// ============================================================================

// -- Root --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile, "PGX.Profile");

// -- Mode --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Mode_Game,       "PGX.Profile.Mode.Game");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Mode_VR,         "PGX.Profile.Mode.VR");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Mode_Enterprise, "PGX.Profile.Mode.Enterprise");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Mode_Tooling,    "PGX.Profile.Mode.Tooling");

// -- Platform --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Platform_PC,             "PGX.Profile.Platform.PC");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Platform_Console_PS,     "PGX.Profile.Platform.Console.PS");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Platform_Console_Xbox,   "PGX.Profile.Platform.Console.Xbox");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Platform_Console_Switch, "PGX.Profile.Platform.Console.Switch");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Platform_Mobile_iOS,     "PGX.Profile.Platform.Mobile.iOS");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Platform_Mobile_Android, "PGX.Profile.Platform.Mobile.Android");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_Platform_XR,             "PGX.Profile.Platform.XR");

// -- State --
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_State_Unresolved, "PGX.Profile.State.Unresolved");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_State_Resolved,   "PGX.Profile.State.Resolved");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_State_Changing,   "PGX.Profile.State.Changing");
UE_DEFINE_GAMEPLAY_TAG(TAG_PGX_Profile_State_Locked,     "PGX.Profile.State.Locked");
