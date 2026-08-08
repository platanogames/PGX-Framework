// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXSaveTypes.h"
#include "PGXSaveDelegates.generated.h"

class UPGXSaveGame;

// ============================================================================
// EN: Dynamic delegates (Blueprint-compatible, reflection overhead)
// ES: Delegates dinamicos (compatibles con Blueprint, overhead de reflexion)
// ============================================================================

/** EN: Fired when a save operation completes / ES: Disparado cuando una operacion de guardado se completa */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPGXSaveCompleted,
	const FString&, SlotName,
	EPGXSaveResult, Result);

/** EN: Fired when a load operation completes / ES: Disparado cuando una operacion de carga se completa */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXLoadCompleted,
	const FString&, SlotName,
	EPGXSaveResult, Result,
	UPGXSaveGame*, SaveGame);

/** EN: Fired when a slot is deleted / ES: Disparado cuando un slot es eliminado */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPGXSlotDeleted,
	const FString&, SlotName);

/** EN: Fired when an auto-save is triggered / ES: Disparado cuando se activa un auto-guardado */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPGXAutoSaveTriggered,
	FGameplayTag, ContextTag);

/** EN: Fired to report save/load progress (0.0 to 1.0) / ES: Disparado para reportar progreso de save/load (0.0 a 1.0) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPGXSaveProgress,
	float, Progress);

// ============================================================================
// EN: Native delegates (C++ only, no reflection overhead, for subsystem-to-subsystem)
// ES: Delegates nativos (solo C++, sin overhead de reflexion, para subsistema-a-subsistema)
// ============================================================================

/** EN: Native save completed (for Slate widgets, C++ listeners) / ES: Save completado nativo (para Slate widgets, listeners C++) */
DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnPGXSaveCompletedNative,
	const FString& /*SlotName*/,
	EPGXSaveResult /*Result*/);

/** EN: Native load completed / ES: Load completado nativo */
DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXLoadCompletedNative,
	const FString& /*SlotName*/,
	EPGXSaveResult /*Result*/,
	UPGXSaveGame* /*SaveGame*/);

/** EN: Native slot deleted / ES: Slot eliminado nativo */
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnPGXSlotDeletedNative,
	const FString& /*SlotName*/);

/** EN: Native auto-save triggered (for Slate widgets, C++ listeners) / ES: Auto-save nativo disparado */
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnPGXAutoSaveTriggeredNative,
	FGameplayTag /*ContextTag*/);

/**
 * EN: Dummy USTRUCT to force UHT to process this header.
 *     Required because DECLARE_DYNAMIC_MULTICAST_DELEGATE needs .generated.h.
 *
 * ES: USTRUCT dummy para forzar a UHT a procesar este header.
 *     Necesario porque DECLARE_DYNAMIC_MULTICAST_DELEGATE necesita .generated.h.
 */
USTRUCT()
struct FPGXSaveDelegatesHelper
{
	GENERATED_BODY()
};
