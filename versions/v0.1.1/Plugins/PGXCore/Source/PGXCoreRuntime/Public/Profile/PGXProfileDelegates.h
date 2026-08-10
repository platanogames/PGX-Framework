// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Profile/PGXProfileTypes.h"
#include "PGXProfileDelegates.generated.h"

// ============================================================================
// EN: Native delegate declarations for the PGX Profile System.
//     Dynamic delegates are declared in PGXProfileTypes.h (for UHT).
//     These native delegates are for C++ listeners (Slate, other subsystems).
//
// ES: Declaraciones de delegados nativos para el Sistema de Profile PGX.
//     Delegados dinamicos estan declarados en PGXProfileTypes.h (para UHT).
//     Estos delegados nativos son para listeners C++ (Slate, otros subsistemas).
// ============================================================================

// --- Native Delegates (C++) ---

/** EN: Fired when profile is first resolved / ES: Se dispara cuando el profile se resuelve por primera vez */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXProfileResolvedNative, const FPGXResolvedProfile& /*ResolvedProfile*/);

/** EN: Fired when profile changes (old, new) / ES: Se dispara cuando el profile cambia (viejo, nuevo) */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXProfileChangedNative, const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& /*NewProfile*/);

/** EN: Fired before profile change (allows preparation) / ES: Se dispara antes del cambio de profile (permite preparacion) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXProfilePreChangeNative, const FPGXResolvedProfile& /*CurrentProfile*/);

/**
 * EN: Dummy USTRUCT to satisfy UHT — this file declares delegates via macros
 *     that need a GENERATED_BODY() somewhere in the translation unit.
 * ES: USTRUCT dummy para satisfacer UHT — este archivo declara delegados via macros
 *     que necesitan un GENERATED_BODY() en alguna parte de la unidad de traduccion.
 */
USTRUCT()
struct FPGXProfileDelegatesRegistrar
{
	GENERATED_BODY()
};
