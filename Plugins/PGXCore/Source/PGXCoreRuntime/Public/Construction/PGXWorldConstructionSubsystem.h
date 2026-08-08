// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PGXWorldConstructionSubsystem.generated.h"

class UPGXActorConstruction;
class UPGXGameModeConstruction;
class UPGXPlayerControllerConstruction;
class UPGXGameStateConstruction;
class UPGXPlayerStateConstruction;
class UPGXCharacterConstruction;
class UPGXPawnConstruction;
class UPGXHUDConstruction;

/**
 * EN: Per-world construction DA overrides. When set, these take priority
 *     over UPGXConstructionSettings (Project Settings).
 *     Resolution: WorldOverride ?? ProjectSettings ?? nullptr.
 * ES: Overrides de DA de construccion por mundo. Cuando se configuran, tienen
 *     prioridad sobre UPGXConstructionSettings (Project Settings).
 *     Resolucion: WorldOverride ?? ProjectSettings ?? nullptr.
 */
UCLASS()
class PGXCORERUNTIME_API UPGXWorldConstructionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ─── Per-Level Overrides (null = use ProjectSettings) ───

	UPROPERTY(Transient)
	TObjectPtr<UPGXActorConstruction> ActorOverride;

	UPROPERTY(Transient)
	TObjectPtr<UPGXGameModeConstruction> GameModeOverride;

	UPROPERTY(Transient)
	TObjectPtr<UPGXPlayerControllerConstruction> PlayerControllerOverride;

	UPROPERTY(Transient)
	TObjectPtr<UPGXGameStateConstruction> GameStateOverride;

	UPROPERTY(Transient)
	TObjectPtr<UPGXPlayerStateConstruction> PlayerStateOverride;

	UPROPERTY(Transient)
	TObjectPtr<UPGXCharacterConstruction> CharacterOverride;

	UPROPERTY(Transient)
	TObjectPtr<UPGXPawnConstruction> PawnOverride;

	UPROPERTY(Transient)
	TObjectPtr<UPGXHUDConstruction> HUDOverride;

	// ─── Typed Resolution ───

	/**
	 * EN: Get override for a specific type. Returns nullptr if no override set.
	 * ES: Obtener override para un tipo especifico. Retorna nullptr si no hay override.
	 */
	template<typename T>
	const T* GetOverride() const;
};

// ═══════════════════════════════════════════════════════════════
// Template specializations for GetOverride<T>
// ═══════════════════════════════════════════════════════════════

template<> inline const UPGXActorConstruction* UPGXWorldConstructionSubsystem::GetOverride<UPGXActorConstruction>() const
{
	return ActorOverride;
}

template<> inline const UPGXGameModeConstruction* UPGXWorldConstructionSubsystem::GetOverride<UPGXGameModeConstruction>() const
{
	return GameModeOverride;
}

template<> inline const UPGXPlayerControllerConstruction* UPGXWorldConstructionSubsystem::GetOverride<UPGXPlayerControllerConstruction>() const
{
	return PlayerControllerOverride;
}

template<> inline const UPGXGameStateConstruction* UPGXWorldConstructionSubsystem::GetOverride<UPGXGameStateConstruction>() const
{
	return GameStateOverride;
}

template<> inline const UPGXPlayerStateConstruction* UPGXWorldConstructionSubsystem::GetOverride<UPGXPlayerStateConstruction>() const
{
	return PlayerStateOverride;
}

template<> inline const UPGXCharacterConstruction* UPGXWorldConstructionSubsystem::GetOverride<UPGXCharacterConstruction>() const
{
	return CharacterOverride;
}

template<> inline const UPGXPawnConstruction* UPGXWorldConstructionSubsystem::GetOverride<UPGXPawnConstruction>() const
{
	return PawnOverride;
}

template<> inline const UPGXHUDConstruction* UPGXWorldConstructionSubsystem::GetOverride<UPGXHUDConstruction>() const
{
	return HUDOverride;
}
