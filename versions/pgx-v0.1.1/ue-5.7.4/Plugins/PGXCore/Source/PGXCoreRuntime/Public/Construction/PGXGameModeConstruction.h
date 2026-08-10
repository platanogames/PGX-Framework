// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Construction/PGXClassConstruction.h"
#include "PGXGameModeConstruction.generated.h"

/**
 * EN: Construction DA for GameMode. Defines default classes for pawns,
 *     player controllers, game state, and HUD. Verticals extend this
 *     (e.g., FPS adds RoundConfig, RPG adds QuestConfig).
 * ES: DA de construccion para GameMode. Define clases por defecto para pawns,
 *     player controllers, game state y HUD. Verticales extienden esto
 *     (ej: FPS agrega RoundConfig, RPG agrega QuestConfig).
 */
UCLASS(BlueprintType, Blueprintable)
class PGXCORERUNTIME_API UPGXGameModeConstruction : public UPGXClassConstruction
{
	GENERATED_BODY()

public:
	UPGXGameModeConstruction();

	// ─── Class Assignments ───

	/** EN: Default pawn class / ES: Clase de pawn por defecto */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|GameMode")
	TSoftClassPtr<APawn> DefaultPawnClass;

	/** EN: Player controller class / ES: Clase de player controller */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|GameMode")
	TSoftClassPtr<APlayerController> PlayerControllerClass;

	/** EN: Player state class / ES: Clase de player state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|GameMode", meta = (AdvancedDisplay))
	TSoftClassPtr<APlayerState> PlayerStateClass;

	/** EN: Game state class / ES: Clase de game state */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|GameMode", meta = (AdvancedDisplay))
	TSoftClassPtr<AGameStateBase> GameStateClass;

	/** EN: HUD class / ES: Clase de HUD */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|GameMode")
	TSoftClassPtr<AHUD> HUDClass;

	/** EN: Spectator pawn class / ES: Clase de pawn espectador */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Construction|GameMode", meta = (AdvancedDisplay))
	TSoftClassPtr<ASpectatorPawn> SpectatorClass;

	// ─── GameFlow Integration ───

	/** EN: Initial GameFlow state to publish when this GameMode initializes / ES: Estado GameFlow inicial */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|GameFlow", meta = (AdvancedDisplay))
	FGameplayTag InitialGameFlowState;

	// ─── Validation ───
	bool Validate(TArray<FText>& OutErrors) const override;
};
