// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXGameFlowTypes.h"
#include "PGXGameFlowDelegates.generated.h"

// ============================================================================
// EN: GameFlow delegate declarations.
//     8 dynamic multicast delegates (one per channel, Blueprint-assignable)
//     + 1 native multicast delegate (C++ only, carries channel param).
// ES: Declaraciones de delegados de GameFlow.
//     8 delegados multicast dinamicos (uno por canal, asignables en Blueprint)
//     + 1 delegado multicast nativo (solo C++, lleva parametro de canal).
// ============================================================================

// --- Dynamic Delegates (Blueprint) ---
// EN: Each channel fires its own delegate so BP users only bind what they need.
// ES: Cada canal dispara su propio delegado para que los usuarios BP solo enlacen lo que necesitan.

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXFlowGlobalChanged,      FGameplayTag, FlowTag, UObject*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXFlowUIChanged,          FGameplayTag, FlowTag, UObject*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXFlowCharactersChanged,  FGameplayTag, FlowTag, UObject*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXFlowAIChanged,          FGameplayTag, FlowTag, UObject*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXFlowCamerasChanged,     FGameplayTag, FlowTag, UObject*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXFlowSystemsChanged,     FGameplayTag, FlowTag, UObject*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXFlowLevelLogicChanged,  FGameplayTag, FlowTag, UObject*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXFlowActorsChanged,      FGameplayTag, FlowTag, UObject*, Source);

// --- Native Delegate (C++) ---
// EN: Single generic delegate for C++ listeners (Slate, other subsystems).
//     Carries channel enum so listeners can filter internally.
// ES: Delegado generico unico para listeners C++ (Slate, otros subsistemas).
//     Lleva enum de canal para que los listeners filtren internamente.
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPGXFlowStateChangedNative, EPGXFlowChannel /*Channel*/, FGameplayTag /*FlowTag*/, UObject* /*Source*/);

/**
 * EN: Dummy USTRUCT to satisfy UHT — this file declares delegates via macros
 *     that need a GENERATED_BODY() somewhere in the translation unit.
 * ES: USTRUCT dummy para satisfacer UHT — este archivo declara delegados via macros
 *     que necesitan un GENERATED_BODY() en alguna parte de la unidad de traduccion.
 */
USTRUCT()
struct FPGXGameFlowDelegatesRegistrar
{
	GENERATED_BODY()
};
