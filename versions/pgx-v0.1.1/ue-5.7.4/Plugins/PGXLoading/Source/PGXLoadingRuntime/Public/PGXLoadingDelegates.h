// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXLoadingTypes.h"
#include "PGXLoadingDelegates.generated.h"

// ============================================================================
// EN: Native Multicast Delegates (C++ only, for Slate and subsystem listeners).
//     Dynamic delegates live in PGXLoadingTypes.h (UHT requires .generated.h).
//
// ES: Delegados Native Multicast (solo C++, para Slate y listeners de subsistema).
//     Delegados dinamicos viven en PGXLoadingTypes.h (UHT requiere .generated.h).
// ============================================================================

/** EN: Native version of loading started / ES: Version nativa de loading iniciado */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXLoadingStartedNative, FGameplayTag /*ContextTag*/);

/** EN: Native version of progress update / ES: Version nativa de actualizacion de progreso */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXLoadingProgressNative, float /*Progress*/, const FText& /*Status*/);

/** EN: Native version of loading completed / ES: Version nativa de loading completado */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXLoadingCompletedNative, const FPGXLoadingRecord& /*Record*/);

/** EN: Native state change delegate (carries new and old state) / ES: Delegado nativo de cambio de estado */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXLoadingStateChangedNative, EPGXLoadingScreenState /*New*/, EPGXLoadingScreenState /*Old*/);

/**
 * EN: Dummy USTRUCT to satisfy UHT — this file declares delegates via macros
 *     that need a GENERATED_BODY() somewhere in the translation unit.
 * ES: USTRUCT dummy para satisfacer UHT — este archivo declara delegados via macros
 *     que necesitan un GENERATED_BODY() en alguna parte de la unidad de traduccion.
 */
USTRUCT()
struct FPGXLoadingDelegatesRegistrar
{
	GENERATED_BODY()
};
