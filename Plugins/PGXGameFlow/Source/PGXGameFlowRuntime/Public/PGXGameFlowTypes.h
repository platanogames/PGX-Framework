// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "PGXGameFlowTypes.generated.h"

// ============================================================================
// EN: Core types for the PGX GameFlow system.
//     Defines channels, result codes, rules, history, and per-channel state.
// ES: Tipos centrales para el sistema PGX GameFlow.
//     Define canales, codigos de resultado, reglas, historial y estado por canal.
// ============================================================================

/**
 * EN: The 8 independent FSM channels in the GameFlow system.
 *     Each channel operates as its own state machine with independent rules.
 * ES: Los 8 canales FSM independientes del sistema GameFlow.
 *     Cada canal opera como su propia maquina de estados con reglas independientes.
 */
UENUM(BlueprintType)
enum class EPGXFlowChannel : uint8
{
	Global      = 0  UMETA(DisplayName = "Global"),
	UI          = 1  UMETA(DisplayName = "UI"),
	Characters  = 2  UMETA(DisplayName = "Characters"),
	AI          = 3  UMETA(DisplayName = "AI"),
	Cameras     = 4  UMETA(DisplayName = "Cameras"),
	Systems     = 5  UMETA(DisplayName = "Systems"),
	LevelLogic  = 6  UMETA(DisplayName = "Level Logic"),
	Actors      = 7  UMETA(DisplayName = "Actors"),
	MAX         = 8  UMETA(Hidden)
};

/** EN: Number of GameFlow channels / ES: Numero de canales GameFlow */
inline constexpr int32 PGX_FLOW_CHANNEL_COUNT = static_cast<int32>(EPGXFlowChannel::MAX);

/**
 * EN: Result codes for GameFlow operations.
 * ES: Codigos de resultado para operaciones de GameFlow.
 */
UENUM(BlueprintType)
enum class EPGXFlowResultCode : uint8
{
	Success          = 0  UMETA(DisplayName = "Success"),
	ValidationError  = 1  UMETA(DisplayName = "Validation Error"),
	RedundantState   = 2  UMETA(DisplayName = "Redundant State"),
	InvalidContext   = 3  UMETA(DisplayName = "Invalid Context"),
	InternalFailure  = 4  UMETA(DisplayName = "Internal Failure"),
	ConfigConflict   = 5  UMETA(DisplayName = "Config Conflict")
};

/**
 * EN: Deterministic policy used when more than one FlowRulesConfig targets the same channel.
 * ES: Politica deterministica usada cuando mas de un FlowRulesConfig apunta al mismo canal.
 */
UENUM(BlueprintType)
enum class EPGXFlowDuplicateRulesPolicy : uint8
{
	/** EN: Keep the first deterministic candidate / ES: Mantener el primer candidato deterministico */
	FirstWins            UMETA(DisplayName = "First Wins"),

	/** EN: Use highest ConflictPriority, then deterministic asset path tie-break / ES: Mayor prioridad y desempate por path */
	HighestPriorityWins  UMETA(DisplayName = "Highest Priority Wins"),

	/** EN: Keep the last deterministic candidate / ES: Mantener el ultimo candidato deterministico */
	LastWins             UMETA(DisplayName = "Last Wins")
};

/**
 * EN: Result struct returned by all GameFlow Set/Validate operations.
 *     Contains success flag, description, and error code.
 * ES: Struct de resultado retornado por todas las operaciones Set/Validate de GameFlow.
 *     Contiene flag de exito, descripcion y codigo de error.
 */
USTRUCT(BlueprintType)
struct PGXGAMEFLOWRUNTIME_API FPGXFlowResult
{
	GENERATED_BODY()

	/** EN: Whether the operation succeeded / ES: Si la operacion fue exitosa */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|GameFlow")
	bool bSuccess = false;

	/** EN: Human-readable description of the result / ES: Descripcion legible del resultado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|GameFlow")
	FString Description;

	/** EN: Machine-readable result code / ES: Codigo de resultado legible por maquina */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|GameFlow")
	EPGXFlowResultCode Code = EPGXFlowResultCode::Success;

	// --- Static Factory Helpers / Fabricas estaticas ---

	static FPGXFlowResult MakeSuccess(const FString& Desc = TEXT(""))
	{
		FPGXFlowResult R;
		R.bSuccess = true;
		R.Description = Desc;
		R.Code = EPGXFlowResultCode::Success;
		return R;
	}

	static FPGXFlowResult MakeFail(EPGXFlowResultCode InCode, const FString& Desc = TEXT(""))
	{
		FPGXFlowResult R;
		R.bSuccess = false;
		R.Description = Desc;
		R.Code = InCode;
		return R;
	}
};

/**
 * EN: Transition rule for a specific origin state.
 *     Defines which destinations are allowed/disallowed from that state.
 *     Stored in TMap<FGameplayTag, FPGXFlowRule> where key = origin state tag.
 * ES: Regla de transicion para un estado de origen especifico.
 *     Define que destinos estan permitidos/prohibidos desde ese estado.
 *     Almacenado en TMap<FGameplayTag, FPGXFlowRule> donde key = tag del estado origen.
 */
USTRUCT(BlueprintType)
struct PGXGAMEFLOWRUNTIME_API FPGXFlowRule
{
	GENERATED_BODY()

	/** EN: Display name for this rule (debugging) / ES: Nombre de display de esta regla (debug) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|GameFlow")
	FName RuleName;

	/**
	 * EN: Whitelist — branches the destination MUST be in.
	 *     If empty, no whitelist restriction (all destinations open unless vetoed).
	 *     Uses IsInBranch: exact match OR StartsWith(Rule + ".").
	 * ES: Lista blanca — ramas en las que el destino DEBE estar.
	 *     Si vacio, no hay restriccion de lista blanca (todos los destinos abiertos salvo veto).
	 *     Usa IsInBranch: match exacto O StartsWith(Rule + ".").
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|GameFlow")
	TArray<FGameplayTag> AllowedDestinations;

	/**
	 * EN: Blacklist (veto layer) — branches the destination MUST NOT be in.
	 *     Overrides AllowedDestinations. Blocks exact match + all descendants.
	 * ES: Lista negra (capa de veto) — ramas en las que el destino NO DEBE estar.
	 *     Sobreescribe AllowedDestinations. Bloquea match exacto + todos los descendientes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|GameFlow")
	TArray<FGameplayTag> DisallowedTagQueries;

	/** EN: Whether revert to previous state is allowed from this state / ES: Si se permite revertir al estado anterior desde este estado */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|GameFlow")
	bool bAllowRevert = true;

	/** EN: Human-readable description of this rule / ES: Descripcion legible de esta regla */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|GameFlow")
	FText Description;

	/** EN: Custom error code for failed transitions from this state / ES: Codigo de error personalizado para transiciones fallidas desde este estado */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PGX|GameFlow")
	int32 ErrorCode = 0;
};

/**
 * EN: Single entry in a channel's transition history.
 *     Records the tag and the exact timestamp (with millisecond precision).
 * ES: Entrada individual en el historial de transiciones de un canal.
 *     Registra el tag y la marca de tiempo exacta (con precision de milisegundos).
 */
USTRUCT(BlueprintType)
struct PGXGAMEFLOWRUNTIME_API FPGXFlowHistoryEntry
{
	GENERATED_BODY()

	/** EN: The flow tag at this point in history / ES: El tag de flujo en este punto del historial */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|GameFlow")
	FGameplayTag FlowTag;

	/** EN: When this state was entered (UTC) / ES: Cuando se entro a este estado (UTC) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|GameFlow")
	FDateTime Timestamp;

	FPGXFlowHistoryEntry()
		: Timestamp(FDateTime::UtcNow())
	{
	}

	FPGXFlowHistoryEntry(const FGameplayTag& InTag)
		: FlowTag(InTag)
		, Timestamp(FDateTime::UtcNow())
	{
	}
};

/**
 * EN: Internal per-channel state. Holds current/last tags and transition history.
 *     NOT exposed to Blueprint — used internally by UPGXGameFlowSubsystem.
 * ES: Estado interno por canal. Contiene tags actual/anterior e historial de transiciones.
 *     NO expuesto a Blueprint — usado internamente por UPGXGameFlowSubsystem.
 */
struct FPGXFlowChannelState
{
	/** EN: Current state tag for this channel / ES: Tag de estado actual de este canal */
	FGameplayTag CurrentTag;

	/** EN: Previous state tag (for revert) / ES: Tag de estado anterior (para revertir) */
	FGameplayTag LastTag;

	/** EN: History of current-tag transitions / ES: Historial de transiciones de tag actual */
	TArray<FPGXFlowHistoryEntry> History;

	/** EN: History of last-tag transitions / ES: Historial de transiciones de tag anterior */
	TArray<FPGXFlowHistoryEntry> LastTagHistory;

	void Reset()
	{
		CurrentTag = FGameplayTag();
		LastTag = FGameplayTag();
		History.Empty();
		LastTagHistory.Empty();
	}
};

// ============================================================================
// DataTable Row Structs — Config Resolution via Project Settings
// ============================================================================

class UPGXFlowRulesConfig;

/**
 * EN: DataTable row for flow rules config resolution via Project Settings.
 *     Maps a flow channel to a UPGXFlowRulesConfig DA.
 * ES: Fila de DataTable para resolucion de config de reglas de flujo via Project Settings.
 *     Mapea un canal de flujo a un DA UPGXFlowRulesConfig.
 */
USTRUCT(BlueprintType)
struct PGXGAMEFLOWRUNTIME_API FPGXFlowRulesRow : public FTableRowBase
{
	GENERATED_BODY()

	/** EN: Flow channel this rules config applies to / ES: Canal de flujo al que aplica este config de reglas */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow")
	EPGXFlowChannel Channel = EPGXFlowChannel::Global;

	/** EN: Flow rules config DA / ES: DA de config de reglas de flujo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow")
	TSoftObjectPtr<UPGXFlowRulesConfig> RulesRef;

	/** EN: Optional description / ES: Descripcion opcional */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameFlow")
	FText Description;
};
