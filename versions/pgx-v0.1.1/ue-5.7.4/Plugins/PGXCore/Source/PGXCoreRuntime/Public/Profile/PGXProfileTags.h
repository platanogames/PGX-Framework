// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * EN: Native gameplay tags for the PGX Profile System.
 *     Using UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG pattern.
 *     Definitions in Private/Tags/PGXProfileTags.cpp.
 *     Declared in L1 (PGXCoreRuntime) so all L2 plugins can reference them.
 *     Ownership: Profile system. Not dev-extensible.
 * ES: Tags nativos de gameplay para el Sistema de Profile PGX.
 *     Usando patron UE_DECLARE_GAMEPLAY_TAG_EXTERN / UE_DEFINE_GAMEPLAY_TAG.
 *     Definiciones en Private/Tags/PGXProfileTags.cpp.
 *     Declarados en L1 (PGXCoreRuntime) para que todos los plugins L2 los referencien.
 */

// -- Root --
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile);

// -- Mode --
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Mode_Game);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Mode_VR);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Mode_Enterprise);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Mode_Tooling);

// -- Platform --
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Platform_PC);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Platform_Console_PS);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Platform_Console_Xbox);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Platform_Console_Switch);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Platform_Mobile_iOS);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Platform_Mobile_Android);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_Platform_XR);

// -- State --
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_State_Unresolved);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_State_Resolved);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_State_Changing);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PGX_Profile_State_Locked);
