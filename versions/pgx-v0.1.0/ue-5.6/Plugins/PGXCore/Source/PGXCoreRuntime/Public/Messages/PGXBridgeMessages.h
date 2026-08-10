// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXBridgeMessages.generated.h"

// =================================================================
// GameFlow Bridge Messages
// =================================================================

/**
 * EN: Published by GameFlow (or GameMode) when game state changes.
 *     Consumed by base classes that opted into bListenToGameFlowChanges and by L2
 *     plugins listening through the canonical Message bus using distributed ownership
 *     and a cross-module star topology.
 *
 *     Backward-compatible extension using an appended-field policy:
 *     Channel + TransitionSourceTag + RequestId provide full channel/provenance payload
 *     clarity. New fields default-construct so existing publishers (existing GameFlow)
 *     continue to compile and broadcast unchanged; older listeners ignore them via
 *     property navigation. The UStruct serialization invariant (parent layout prefix is
 *     stable) plus PGXMessageSubsystem IsChildOf compatibility (existing subsystem behavior)
 *     guarantee safe payload backward compatibility.
 *
 * ES: Publicado por GameFlow (o GameMode) cuando el estado de juego cambia.
 *     Extension backward-compatible: Channel + TransitionSourceTag + RequestId aniadidos
 *     con appended-field policy backward-compat. Los publishers existentes no rompen
 *     (default-construct); listeners antiguos ignoran via property navigation.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXBridgeGameFlowChanged
{
	GENERATED_BODY()

	/** EN: Previous game flow state / ES: Estado de game flow anterior */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag OldState;

	/** EN: New game flow state / ES: Nuevo estado de game flow */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag NewState;

	/** EN: World time of the transition / ES: Tiempo de mundo de la transicion */
	UPROPERTY(BlueprintReadOnly)
	double Timestamp = 0.0;

	/**
	 * EN: GameFlow channel (within the framework taxonomy: UI / Game / Audio / etc.) on
	 *     which the transition occurred. FGameplayTag chosen over EPGXFlowChannel for
	 *     cross-plugin transparency (consumers other than GameFlow do not need to depend
	 *     on the GameFlow-internal enum). Default empty preserves backward-compat with
	 *     legacy publishers that do not yet populate this field.
	 * ES: Canal GameFlow (taxonomia framework: UI / Game / Audio / etc.) en el que
	 *     ocurrio la transicion. FGameplayTag por transparencia cross-plugin.
	 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Channel;

	/**
	 * EN: Source/cause tag — what triggered the transition (user input tag, AI decision
	 *     tag, save-domain restore tag, EventHandler resolution tag, etc.). Empty when
	 *     the cause is implicit/internal. Default empty preserves backward-compat.
	 * ES: Tag de origen/causa — que disparo la transicion. Vacio cuando la causa es
	 *     implicita/interna.
	 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag TransitionSourceTag;

	/**
	 * EN: Per-transition unique correlation identifier. Enables consumer-side dedup,
	 *     saga tracking, and audit-trail reconstruction across plugins. Publishers may
	 *     either generate a fresh FGuid::NewGuid() or propagate an upstream RequestId
	 *     (e.g., from a GameFlow batch operation or EventHandler request). Default
	 *     constructed (zero-FGuid, IsValid()==false) preserves backward-compat.
	 * ES: Identificador unico de correlacion por transicion. Habilita dedup en consumer,
	 *     saga tracking, y reconstruccion de audit-trail cross-plugin.
	 */
	UPROPERTY(BlueprintReadOnly)
	FGuid RequestId;
};

// =================================================================
// LevelFlow Bridge Messages
// =================================================================

/**
 * EN: Published by LevelFlow when a level transition starts or completes.
 * ES: Publicado por LevelFlow cuando una transicion de nivel comienza o se completa.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXBridgeLevelTransition
{
	GENERATED_BODY()

	/** EN: Tag identifying the transition / ES: Tag que identifica la transicion */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag TransitionTag;

	/** EN: Target level name / ES: Nombre del nivel objetivo */
	UPROPERTY(BlueprintReadOnly)
	FString TargetLevelName;

	/** EN: Is this the start or end of the transition? / ES: Es el inicio o fin de la transicion? */
	UPROPERTY(BlueprintReadOnly)
	bool bIsStart = true;

	UPROPERTY(BlueprintReadOnly)
	double Timestamp = 0.0;
};

// =================================================================
// Loading Bridge Messages
// =================================================================

/**
 * EN: Published by Loading when the loading screen shows or hides.
 * ES: Publicado por Loading cuando la pantalla de carga aparece o desaparece.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXBridgeLoadingState
{
	GENERATED_BODY()

	/** EN: True when loading screen is visible / ES: True cuando la pantalla de carga es visible */
	UPROPERTY(BlueprintReadOnly)
	bool bIsLoading = false;

	/** EN: Loading progress [0.0 - 1.0] / ES: Progreso de carga [0.0 - 1.0] */
	UPROPERTY(BlueprintReadOnly)
	float Progress = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	double Timestamp = 0.0;
};

// =================================================================
// Audio Bridge Messages
// =================================================================

/**
 * EN: Published by base classes to request a sound play. Consumed by Audio subsystem.
 * ES: Publicado por clases base para solicitar reproduccion de sonido. Consumido por subsistema Audio.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXBridgeAudioRequest
{
	GENERATED_BODY()

	/** EN: Sound event tag from PGX.Audio.* / ES: Tag de evento de sonido de PGX.Audio.* */
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag SoundEventTag;

	/** EN: World location (FVector::ZeroVector for 2D sound) / ES: Ubicacion en mundo */
	UPROPERTY(BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** EN: Requesting actor (for attenuation) / ES: Actor solicitante (para atenuacion) */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> Instigator;
};

// =================================================================
// Save Bridge Messages
// =================================================================

/**
 * EN: Published by Save subsystem before/after save/load operations.
 *     Base classes listen to know when to push/pull their state.
 * ES: Publicado por subsistema Save antes/despues de operaciones save/load.
 *     Clases base escuchan para saber cuando empujar/extraer su estado.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXBridgeSaveNotification
{
	GENERATED_BODY()

	/** EN: Domain being saved/loaded / ES: Dominio siendo guardado/cargado */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DomainTag;

	/** EN: Slot name / ES: Nombre del slot */
	UPROPERTY(BlueprintReadOnly)
	FString SlotName;

	/** EN: true = pre-save/post-load, false = completed/failed / ES: indicador de fase */
	UPROPERTY(BlueprintReadOnly)
	bool bIsPreOperation = true;

	UPROPERTY(BlueprintReadOnly)
	double Timestamp = 0.0;
};

/**
 * EN: Published by base classes to register/unregister as saveable via Message bus.
 *     Save subsystem listens for this and calls RegisterSaveable/UnregisterSaveable.
 * ES: Publicado por clases base para registrarse/desregistrarse como saveable via Message bus.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXBridgeSaveableRegistration
{
	GENERATED_BODY()

	/** EN: The object implementing IPGXSaveable / ES: El objeto que implementa IPGXSaveable */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> SaveableObject;

	/** EN: Domain to register under / ES: Dominio bajo el cual registrarse */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DomainTag;

	/** EN: true = register, false = unregister / ES: true = registrar, false = desregistrar */
	UPROPERTY(BlueprintReadOnly)
	bool bRegister = true;
};
