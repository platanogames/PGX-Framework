// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EventHandler/PGXEventHandlerTypes.h"
#include "PGXEventHandlerDelegates.generated.h"

/**
 * EN: Delegates for the PGX Event Handler System.
 * ES: Delegates para el Sistema de Event Handler PGX.
 */

/** EN: Fired after a handler is executed (Blueprint) / ES: Se dispara despues de ejecutar un handler */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPGXHandlerExecuted, FGameplayTag, EventTag, EPGXEventResult, Result, const FString&, HandlerClassName);

/** EN: Fired after a handler is executed (Native) / ES: Se dispara despues de ejecutar un handler (Nativo) */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPGXHandlerExecutedNative, FGameplayTag /*EventTag*/, EPGXEventResult /*Result*/, double /*ExecutionTimeMs*/);

/** EN: Fired when no handler is found for a tag / ES: Se dispara cuando no se encuentra handler para un tag */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXHandlerNotFound, FGameplayTag, EventTag);

/** EN: Fired when the handler cache changes / ES: Se dispara cuando cambia el cache de handlers */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXHandlerCacheChanged, FGameplayTag, EventTag, bool, bCached);

/**
 * EN: Dummy class to force UHT generation for this header.
 * ES: Clase dummy para forzar generacion UHT de este header.
 */
UCLASS(MinimalAPI)
class UPGXEventHandlerDelegatesHelper : public UObject
{
	GENERATED_BODY()
};
