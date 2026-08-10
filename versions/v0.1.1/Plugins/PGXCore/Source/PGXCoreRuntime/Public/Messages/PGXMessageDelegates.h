// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXMessageDelegates.generated.h"

/**
 * EN: Delegates for the PGX Message System.
 * ES: Delegates para el Sistema de Mensajes PGX.
 */

/** EN: Fired on every message broadcast (Blueprint) / ES: Se dispara en cada broadcast de mensaje (Blueprint) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPGXMessageBroadcast, FGameplayTag, Channel, const FString&, PayloadTypeName);

/** EN: Fired on every message broadcast (Native, includes timing) / ES: Se dispara en cada broadcast (Nativo, incluye timing) */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPGXMessageBroadcastNative, FGameplayTag /*Channel*/, const UScriptStruct* /*StructType*/, double /*Timestamp*/);

/** EN: Fired when a listener is registered / ES: Se dispara cuando se registra un listener */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXListenerRegistered, FGameplayTag, Channel);

/** EN: Fired when a listener is unregistered / ES: Se dispara cuando se desregistra un listener */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPGXListenerUnregistered, FGameplayTag, Channel);

/**
 * EN: Dummy class to force UHT generation for this header.
 * ES: Clase dummy para forzar generacion UHT de este header.
 */
UCLASS(MinimalAPI)
class UPGXMessageDelegatesHelper : public UObject
{
	GENERATED_BODY()
};
