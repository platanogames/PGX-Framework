// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXSettings.h"
#include "Construction/PGXConstructionTypes.h"
#include "PGXConstructionSettings.generated.h"

class UPGXActorConstruction;
class UPGXGameModeConstruction;
class UPGXPlayerControllerConstruction;
class UPGXGameStateConstruction;
class UPGXPlayerStateConstruction;
class UPGXCharacterConstruction;
class UPGXPawnConstruction;
class UPGXHUDConstruction;

class APGXActorBase;
class APGXGameModeBase;
class APGXPlayerControllerBase;
class APGXGameStateBase;
class APGXPlayerStateBase;
class APGXCharacterBase;
class APGXPawnBase;
class AHUD;

/**
 * EN: Project-level construction settings. Visible in
 *     Project Settings > PGX > Framework Construction.
 *     Each slot configures: class source (Default/C++/Blueprint) + class picker + DA.
 *     The DA is always available regardless of class source (UX-First: the DA is the box).
 *     Per-level overrides via UPGXWorldConstructionSubsystem.
 * ES: Configuracion de construccion a nivel de proyecto. Visible en
 *     Project Settings > PGX > Framework Construction.
 *     Cada slot configura: fuente de clase (Default/C++/Blueprint) + picker de clase + DA.
 *     El DA siempre esta disponible independiente del class source (UX-First: el DA es la caja).
 *     Overrides por nivel via UPGXWorldConstructionSubsystem.
 */
UCLASS(config = PGX, defaultconfig, meta = (DisplayName = "PGX Framework Construction"))
class PGXCORERUNTIME_API UPGXConstructionSettings : public UPGXSettings
{
	GENERATED_BODY()

public:
	//~ Begin UDeveloperSettings Interface
	FName GetSectionName() const override { return FName(TEXT("FrameworkConstruction")); }
	FText GetSectionText() const override;
	FText GetSectionDescription() const override;
	//~ End UDeveloperSettings Interface

	// ═══════════════════════════════════════════════════════════════════
	// Actor (Generic)
	// ═══════════════════════════════════════════════════════════════════

	/** EN: Construction DA for generic APGXActorBase actors.
	  *     Applied in BeginPlay to any actor deriving from APGXActorBase.
	  * ES: DA de construccion para actores genericos APGXActorBase.
	  *     Se aplica en BeginPlay a cualquier actor derivado de APGXActorBase. */
	UPROPERTY(config, EditAnywhere, Category = "Actor", meta = (DisplayName = "Construction DA"))
	TSoftObjectPtr<UPGXActorConstruction> ActorConstruction;

	// ═══════════════════════════════════════════════════════════════════
	// GameMode
	// ═══════════════════════════════════════════════════════════════════

	/** EN: How the GameMode class is provided / ES: Como se provee la clase GameMode */
	UPROPERTY(config, EditAnywhere, Category = "GameMode", meta = (DisplayName = "Class Source"))
	EPGXClassSourceMode GameModeClassSource = EPGXClassSourceMode::Default;

	/** EN: C++ class derived from APGXGameModeBase / ES: Clase C++ derivada de APGXGameModeBase */
	UPROPERTY(config, EditAnywhere, Category = "GameMode",
		meta = (DisplayName = "C++ Class",
				EditCondition = "GameModeClassSource == EPGXClassSourceMode::CppClass",
				EditConditionHides))
	TSubclassOf<APGXGameModeBase> GameModeClassCPP;

	/** EN: Blueprint class derived from APGXGameModeBase / ES: Clase BP derivada de APGXGameModeBase */
	UPROPERTY(config, EditAnywhere, Category = "GameMode",
		meta = (DisplayName = "Blueprint Class",
				EditCondition = "GameModeClassSource == EPGXClassSourceMode::Blueprint",
				EditConditionHides))
	TSoftClassPtr<APGXGameModeBase> GameModeClassBP;

	/** EN: Construction DA — always configurable regardless of class source.
	  *     NOTE: Also set your GameMode class in Project Settings > Maps & Modes > Default GameMode,
	  *     and/or per-level in World Settings > GameMode Override.
	  * ES: DA de construccion — siempre configurable independiente del class source.
	  *     NOTA: Tambien configure su clase GameMode en Project Settings > Maps & Modes > Default GameMode,
	  *     y/o por nivel en World Settings > GameMode Override. */
	UPROPERTY(config, EditAnywhere, Category = "GameMode", meta = (DisplayName = "Construction DA"))
	TSoftObjectPtr<UPGXGameModeConstruction> GameModeConstruction;

	// ═══════════════════════════════════════════════════════════════════
	// PlayerController
	// ═══════════════════════════════════════════════════════════════════

	/** EN: How the PlayerController class is provided / ES: Como se provee la clase PlayerController */
	UPROPERTY(config, EditAnywhere, Category = "PlayerController", meta = (DisplayName = "Class Source"))
	EPGXClassSourceMode PlayerControllerClassSource = EPGXClassSourceMode::Default;

	/** EN: C++ class derived from APGXPlayerControllerBase / ES: Clase C++ derivada de APGXPlayerControllerBase */
	UPROPERTY(config, EditAnywhere, Category = "PlayerController",
		meta = (DisplayName = "C++ Class",
				EditCondition = "PlayerControllerClassSource == EPGXClassSourceMode::CppClass",
				EditConditionHides))
	TSubclassOf<APGXPlayerControllerBase> PlayerControllerClassCPP;

	/** EN: Blueprint class derived from APGXPlayerControllerBase / ES: Clase BP derivada de APGXPlayerControllerBase */
	UPROPERTY(config, EditAnywhere, Category = "PlayerController",
		meta = (DisplayName = "Blueprint Class",
				EditCondition = "PlayerControllerClassSource == EPGXClassSourceMode::Blueprint",
				EditConditionHides))
	TSoftClassPtr<APGXPlayerControllerBase> PlayerControllerClassBP;

	/** EN: Construction DA. GameMode determines PlayerController class — set it in your
	  *     GameMode DA (PlayerControllerClass field) or in the GameMode Blueprint.
	  * ES: DA de construccion. El GameMode determina la clase PlayerController — configurela en
	  *     el DA de GameMode (campo PlayerControllerClass) o en el Blueprint de GameMode. */
	UPROPERTY(config, EditAnywhere, Category = "PlayerController", meta = (DisplayName = "Construction DA"))
	TSoftObjectPtr<UPGXPlayerControllerConstruction> PlayerControllerConstruction;

	// ═══════════════════════════════════════════════════════════════════
	// GameState
	// ═══════════════════════════════════════════════════════════════════

	/** EN: How the GameState class is provided / ES: Como se provee la clase GameState */
	UPROPERTY(config, EditAnywhere, Category = "GameState", meta = (DisplayName = "Class Source"))
	EPGXClassSourceMode GameStateClassSource = EPGXClassSourceMode::Default;

	/** EN: C++ class derived from APGXGameStateBase / ES: Clase C++ derivada de APGXGameStateBase */
	UPROPERTY(config, EditAnywhere, Category = "GameState",
		meta = (DisplayName = "C++ Class",
				EditCondition = "GameStateClassSource == EPGXClassSourceMode::CppClass",
				EditConditionHides))
	TSubclassOf<APGXGameStateBase> GameStateClassCPP;

	/** EN: Blueprint class derived from APGXGameStateBase / ES: Clase BP derivada de APGXGameStateBase */
	UPROPERTY(config, EditAnywhere, Category = "GameState",
		meta = (DisplayName = "Blueprint Class",
				EditCondition = "GameStateClassSource == EPGXClassSourceMode::Blueprint",
				EditConditionHides))
	TSoftClassPtr<APGXGameStateBase> GameStateClassBP;

	/** EN: Construction DA. GameMode determines GameState class — set it in your
	  *     GameMode DA (GameStateClass field).
	  * ES: DA de construccion. El GameMode determina la clase GameState — configurela en
	  *     el DA de GameMode (campo GameStateClass). */
	UPROPERTY(config, EditAnywhere, Category = "GameState", meta = (DisplayName = "Construction DA"))
	TSoftObjectPtr<UPGXGameStateConstruction> GameStateConstruction;

	// ═══════════════════════════════════════════════════════════════════
	// PlayerState
	// ═══════════════════════════════════════════════════════════════════

	/** EN: How the PlayerState class is provided / ES: Como se provee la clase PlayerState */
	UPROPERTY(config, EditAnywhere, Category = "PlayerState", meta = (DisplayName = "Class Source"))
	EPGXClassSourceMode PlayerStateClassSource = EPGXClassSourceMode::Default;

	/** EN: C++ class derived from APGXPlayerStateBase / ES: Clase C++ derivada de APGXPlayerStateBase */
	UPROPERTY(config, EditAnywhere, Category = "PlayerState",
		meta = (DisplayName = "C++ Class",
				EditCondition = "PlayerStateClassSource == EPGXClassSourceMode::CppClass",
				EditConditionHides))
	TSubclassOf<APGXPlayerStateBase> PlayerStateClassCPP;

	/** EN: Blueprint class derived from APGXPlayerStateBase / ES: Clase BP derivada de APGXPlayerStateBase */
	UPROPERTY(config, EditAnywhere, Category = "PlayerState",
		meta = (DisplayName = "Blueprint Class",
				EditCondition = "PlayerStateClassSource == EPGXClassSourceMode::Blueprint",
				EditConditionHides))
	TSoftClassPtr<APGXPlayerStateBase> PlayerStateClassBP;

	/** EN: Construction DA. GameMode determines PlayerState class — set it in your
	  *     GameMode DA (PlayerStateClass field).
	  * ES: DA de construccion. El GameMode determina la clase PlayerState — configurela en
	  *     el DA de GameMode (campo PlayerStateClass). */
	UPROPERTY(config, EditAnywhere, Category = "PlayerState", meta = (DisplayName = "Construction DA"))
	TSoftObjectPtr<UPGXPlayerStateConstruction> PlayerStateConstruction;

	// ═══════════════════════════════════════════════════════════════════
	// Character
	// ═══════════════════════════════════════════════════════════════════

	/** EN: How the Character class is provided / ES: Como se provee la clase Character */
	UPROPERTY(config, EditAnywhere, Category = "Character", meta = (DisplayName = "Class Source"))
	EPGXClassSourceMode CharacterClassSource = EPGXClassSourceMode::Default;

	/** EN: C++ class derived from APGXCharacterBase / ES: Clase C++ derivada de APGXCharacterBase */
	UPROPERTY(config, EditAnywhere, Category = "Character",
		meta = (DisplayName = "C++ Class",
				EditCondition = "CharacterClassSource == EPGXClassSourceMode::CppClass",
				EditConditionHides))
	TSubclassOf<APGXCharacterBase> CharacterClassCPP;

	/** EN: Blueprint class derived from APGXCharacterBase / ES: Clase BP derivada de APGXCharacterBase */
	UPROPERTY(config, EditAnywhere, Category = "Character",
		meta = (DisplayName = "Blueprint Class",
				EditCondition = "CharacterClassSource == EPGXClassSourceMode::Blueprint",
				EditConditionHides))
	TSoftClassPtr<APGXCharacterBase> CharacterClassBP;

	/** EN: Construction DA. GameMode determines Default Pawn class — set it in your
	  *     GameMode DA (DefaultPawnClass field) or World Settings.
	  * ES: DA de construccion. El GameMode determina la clase Default Pawn — configurela en
	  *     el DA de GameMode (campo DefaultPawnClass) o en World Settings. */
	UPROPERTY(config, EditAnywhere, Category = "Character", meta = (DisplayName = "Construction DA"))
	TSoftObjectPtr<UPGXCharacterConstruction> CharacterConstruction;

	// ═══════════════════════════════════════════════════════════════════
	// Pawn
	// ═══════════════════════════════════════════════════════════════════

	/** EN: How the Pawn class is provided / ES: Como se provee la clase Pawn */
	UPROPERTY(config, EditAnywhere, Category = "Pawn", meta = (DisplayName = "Class Source"))
	EPGXClassSourceMode PawnClassSource = EPGXClassSourceMode::Default;

	/** EN: C++ class derived from APGXPawnBase / ES: Clase C++ derivada de APGXPawnBase */
	UPROPERTY(config, EditAnywhere, Category = "Pawn",
		meta = (DisplayName = "C++ Class",
				EditCondition = "PawnClassSource == EPGXClassSourceMode::CppClass",
				EditConditionHides))
	TSubclassOf<APGXPawnBase> PawnClassCPP;

	/** EN: Blueprint class derived from APGXPawnBase / ES: Clase BP derivada de APGXPawnBase */
	UPROPERTY(config, EditAnywhere, Category = "Pawn",
		meta = (DisplayName = "Blueprint Class",
				EditCondition = "PawnClassSource == EPGXClassSourceMode::Blueprint",
				EditConditionHides))
	TSoftClassPtr<APGXPawnBase> PawnClassBP;

	/** EN: Construction DA. GameMode determines Default Pawn class — set it in your
	  *     GameMode DA (DefaultPawnClass field) or World Settings.
	  * ES: DA de construccion. El GameMode determina la clase Default Pawn — configurela en
	  *     el DA de GameMode (campo DefaultPawnClass) o en World Settings. */
	UPROPERTY(config, EditAnywhere, Category = "Pawn", meta = (DisplayName = "Construction DA"))
	TSoftObjectPtr<UPGXPawnConstruction> PawnConstruction;

	// ═══════════════════════════════════════════════════════════════════
	// HUD
	// ═══════════════════════════════════════════════════════════════════

	/** EN: How the HUD class is provided / ES: Como se provee la clase HUD */
	UPROPERTY(config, EditAnywhere, Category = "HUD", meta = (DisplayName = "Class Source"))
	EPGXClassSourceMode HUDClassSource = EPGXClassSourceMode::Default;

	/** EN: C++ class derived from AHUD / ES: Clase C++ derivada de AHUD */
	UPROPERTY(config, EditAnywhere, Category = "HUD",
		meta = (DisplayName = "C++ Class",
				EditCondition = "HUDClassSource == EPGXClassSourceMode::CppClass",
				EditConditionHides))
	TSubclassOf<AHUD> HUDClassCPP;

	/** EN: Blueprint class derived from AHUD / ES: Clase BP derivada de AHUD */
	UPROPERTY(config, EditAnywhere, Category = "HUD",
		meta = (DisplayName = "Blueprint Class",
				EditCondition = "HUDClassSource == EPGXClassSourceMode::Blueprint",
				EditConditionHides))
	TSoftClassPtr<AHUD> HUDClassBP;

	/** EN: Construction DA. GameMode determines HUD class — set it in your
	  *     GameMode DA (HUDClass field).
	  * ES: DA de construccion. El GameMode determina la clase HUD — configurela en
	  *     el DA de GameMode (campo HUDClass). */
	UPROPERTY(config, EditAnywhere, Category = "HUD", meta = (DisplayName = "Construction DA"))
	TSoftObjectPtr<UPGXHUDConstruction> HUDConstruction;

	// ─── Debug ───

	/** EN: Enable verbose construction logging / ES: Habilitar logging detallado de construccion */
	UPROPERTY(config, EditAnywhere, Category = "Debug")
	bool bVerboseConstructionLog = false;

	/** EN: Validate construction settings on editor startup / ES: Validar settings de construccion al iniciar el editor */
	UPROPERTY(config, EditAnywhere, Category = "Debug")
	bool bValidateOnStartup = true;

	/**
	 * EN: Validate class source modes vs class/DA assignment. Returns true if no warnings.
	 * ES: Validar modos de class source vs asignacion de clase/DA. Retorna true si no hay warnings.
	 */
	bool ValidateClassSources(TArray<FText>& OutWarnings, bool bIncludeMapsModesCheck = true) const;
};
