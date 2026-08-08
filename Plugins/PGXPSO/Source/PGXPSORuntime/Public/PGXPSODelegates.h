// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXPSOTypes.h"

// ============================================================================
// EN: Native Multicast Delegates (C++ only, for Slate and subsystem listeners)
//     Dynamic delegates live in PGXPSOTypes.h (UHT requires .generated.h)
// ES: Delegados Native Multicast (solo C++, para Slate y listeners de subsistema)
//     Delegados dinamicos viven en PGXPSOTypes.h (UHT requiere .generated.h)
// ============================================================================

/** EN: Native version of warm-up begin / ES: Version nativa del inicio de warm-up */
DECLARE_MULTICAST_DELEGATE(FOnPGXPSOWarmUpBeginNative);

/** EN: Native version of warm-up progress / ES: Version nativa del progreso de warm-up */
DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnPGXPSOWarmUpProgressNative,
	int32 /*Completed*/,
	int32 /*Total*/,
	float /*Percent*/);

/** EN: Native version of warm-up complete / ES: Version nativa del warm-up completado */
DECLARE_MULTICAST_DELEGATE(FOnPGXPSOWarmUpCompleteNative);

/** EN: Native state change delegate (carries new state) / ES: Delegado nativo de cambio de estado (lleva nuevo estado) */
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnPGXPSOStateChangedNative,
	EPGXPSOWarmUpState /*NewState*/);
