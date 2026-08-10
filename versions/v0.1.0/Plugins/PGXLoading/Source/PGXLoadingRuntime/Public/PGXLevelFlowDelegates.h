// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXLevelFlowTypes.h"
#include "PGXLevelFlowDelegates.generated.h"

// ============================================================================
// EN: LevelFlow delegate declarations.
//     6 dynamic multicast delegates (Blueprint-assignable)
//     + 3 native multicast delegates (C++ only — Slate, editor tools, other subsystems).
//
// ES: Declaraciones de delegados de LevelFlow.
//     6 delegados multicast dinamicos (asignables en Blueprint)
//     + 3 delegados multicast nativos (solo C++ — Slate, herramientas editor, otros subsistemas).
// ============================================================================

// ── Dynamic Delegates (Blueprint) ───────────────────────────────────────────

// EN: Fired when a level transition starts / ES: Disparado cuando inicia una transicion de nivel
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXTransitionStarted, FGameplayTag, LevelTag);

// EN: Fired periodically during loading/postload with progress / ES: Disparado periodicamente durante carga/postload con progreso
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXTransitionProgress, float, Percent, FGameplayTag, LevelTag);

// EN: Fired when a level transition completes successfully / ES: Disparado cuando una transicion de nivel completa exitosamente
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXTransitionCompleted, FGameplayTag, LevelTag);

// EN: Fired when a level transition fails / ES: Disparado cuando una transicion de nivel falla
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXTransitionFailed, FString, ErrorMessage, FGameplayTag, LevelTag);

// EN: Fired when a sub-level finishes loading / ES: Disparado cuando un sub-nivel termina de cargar
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXSubLevelLoaded, FGameplayTag, SubLevelTag);

// EN: Fired when a sub-level is unloaded / ES: Disparado cuando un sub-nivel es descargado
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXSubLevelUnloaded, FGameplayTag, SubLevelTag);

// ── Native Delegates (C++) ──────────────────────────────────────────────────

// EN: C++ delegate for transition start (Slate, editor panels)
// ES: Delegado C++ para inicio de transicion (Slate, paneles editor)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXTransitionStartedNative, FGameplayTag /*LevelTag*/);

// EN: C++ delegate for transition completion
// ES: Delegado C++ para completar transicion
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXTransitionCompletedNative, FGameplayTag /*LevelTag*/);

// EN: C++ delegate for pipeline state changes (carries state + level tag)
// ES: Delegado C++ para cambios de estado del pipeline (lleva estado + tag de nivel)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXLevelFlowStateChangedNative, EPGXLevelFlowState /*NewState*/, FGameplayTag /*LevelTag*/);

/**
 * EN: Dummy USTRUCT to satisfy UHT — this file declares delegates via macros
 *     that need a GENERATED_BODY() somewhere in the translation unit.
 * ES: USTRUCT dummy para satisfacer UHT — este archivo declara delegados via macros
 *     que necesitan un GENERATED_BODY() en alguna parte de la unidad de traduccion.
 */
USTRUCT()
struct FPGXLevelFlowDelegatesRegistrar
{
	GENERATED_BODY()
};
