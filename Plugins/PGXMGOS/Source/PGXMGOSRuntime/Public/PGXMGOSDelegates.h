// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXMGOSTypes.h"
#include "PGXMGOSDelegates.generated.h"

// ============================================================================
// Native Delegates (C++ only)
// ============================================================================

/** EN: Native delegate — GC cycle started / ES: Delegado nativo — ciclo GC iniciado */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXGCStartedNative, uint64 /*CycleID*/);

/** EN: Native delegate — GC cycle completed / ES: Delegado nativo — ciclo GC completado */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPGXGCCompletedNative, uint64 /*CycleID*/, const FPGXGCSnapshot& /*PostSnapshot*/, const FPGXGCSnapshotDiff& /*Diff*/);

/** EN: Native delegate — profile state changed / ES: Delegado nativo — estado de perfil cambio */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPGXGCProfileStateChangedNative, EPGXGCProfileState /*NewState*/, EPGXGCProfileState /*OldState*/);

/** EN: Native delegate — incident raised / ES: Delegado nativo — incidente generado */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPGXGCIncidentRaisedNative, const FPGXGCIncident& /*Incident*/);

/**
 * EN: Dummy USTRUCT for UHT registration of this header.
 * ES: USTRUCT dummy para registro UHT de este header.
 */
USTRUCT()
struct FPGXMGOSDelegatesRegistrar
{
	GENERATED_BODY()
};
