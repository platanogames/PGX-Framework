// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXColonyTypes.generated.h"

/**
 * EN: Typed result enum for PGXColony public API surfaces. Mirrors the result-policy convention
 *     used across PGXFlowResult / PGXSaveResult / PGXAIResult — uniform success/failure surface
 *     so callers can branch on the enum without parsing strings or interpreting nullptrs. Behavior
 *     baseline primitive.
 * ES: Enum de resultado tipado para superficies API publicas de PGXColony.
 */
UENUM(BlueprintType)
enum class EPGXColonyResultCode : uint8
{
	/** EN: Operation succeeded. */
	Success                 UMETA(DisplayName = "Success"),

	/** EN: Subsystem not initialized or not available. */
	SubsystemUnavailable    UMETA(DisplayName = "Subsystem Unavailable"),

	/** EN: Required input was invalid (null pointer, invalid handle, malformed tag, etc.). */
	InvalidInput            UMETA(DisplayName = "Invalid Input"),

	/** EN: Lookup failed — survivor / settlement / role / task / need not found. */
	NotFound                UMETA(DisplayName = "Not Found"),

	/** EN: Operation rejected by policy (capacity / phase gate / faction conflict / authority). */
	PolicyRejected          UMETA(DisplayName = "Policy Rejected"),

	/** EN: Generic failure with diagnostic message (avoid when a more specific code applies). */
	Failure                 UMETA(DisplayName = "Failure")
};

/**
 * EN: Typed result for PGXColony public API. `bSucceeded` is the canonical success predicate;
 *     `Code` carries the enum reason; `DiagnosticMessage` is human-readable detail for logs and
 *     inspector telemetry. Default-constructed value represents an inconclusive "no-op" outcome
 *     (Code=Success, bSucceeded=true).
 */
USTRUCT(BlueprintType)
struct PGXCOLONYRUNTIME_API FPGXColonyResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSucceeded = true;

	UPROPERTY(BlueprintReadOnly)
	EPGXColonyResultCode Code = EPGXColonyResultCode::Success;

	UPROPERTY(BlueprintReadOnly)
	FString DiagnosticMessage;

	static FPGXColonyResult MakeSuccess()
	{
		return FPGXColonyResult{};
	}

	static FPGXColonyResult MakeFailure(EPGXColonyResultCode InCode, const FString& InMessage = TEXT(""))
	{
		FPGXColonyResult R;
		R.bSucceeded = false;
		R.Code = InCode;
		R.DiagnosticMessage = InMessage;
		return R;
	}
};

/**
 * EN: Stable handle for a registered colony survivor. Carries the subsystem-assigned `SurvivorId`
 *     plus an optional FGameplayTag identifying the survivor's authored definition. Default-
 *     constructed (SurvivorId==0) handles are invalid; subsystem assigns ids starting at 1.
 *     Development Preview primitive — full survivor lifecycle (recruitment / interview evidence /
 *     hidden traits / morale / faction membership) is outside the current product boundary.
 * ES: Handle estable para un superviviente registrado. SurvivorId asignado por el subsistema +
 *     FGameplayTag opcional identificando la definicion autorada.
 */
USTRUCT(BlueprintType)
struct PGXCOLONYRUNTIME_API FPGXColonySurvivorHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 SurvivorId = 0;

	/**
	 * EN: Tag identifying the authored survivor definition (e.g., `PGX.Colony.Survivor.<Type>`).
	 *     Optional; runtime Object Data Asset resolution does not consume it.
	 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DefinitionTag;

	bool IsValid() const { return SurvivorId != 0; }

	bool operator==(const FPGXColonySurvivorHandle& Other) const { return SurvivorId == Other.SurvivorId; }
	bool operator!=(const FPGXColonySurvivorHandle& Other) const { return !(*this == Other); }
};

FORCEINLINE uint32 GetTypeHash(const FPGXColonySurvivorHandle& Handle)
{
	return GetTypeHash(Handle.SurvivorId);
}

/**
 * EN: Typed identity wrappers for colony role / task / need / event categories. FGameplayTag-based
 *     for star-topology compliance and cross-plugin transparency (other plugins can read these
 *     categories without depending on a colony-internal enum). Development Preview ships the SHAPE;
 *     authored definitions and the resolution pipeline (UPGXSurvivorRoleDefinition,
 *     UPGXColonyTaskDefinition, UPGXColonyNeedDefinition, UPGXColonyEventDefinition Object DAs)
 *     are outside the current product boundary.
 * ES: Wrappers de identidad tipados para categorias colony role/task/need/event basados en
 *     FGameplayTag.
 */
USTRUCT(BlueprintType)
struct PGXCOLONYRUNTIME_API FPGXColonyRoleId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Tag;

	bool IsValid() const { return Tag.IsValid(); }
	bool operator==(const FPGXColonyRoleId& Other) const { return Tag == Other.Tag; }
};

USTRUCT(BlueprintType)
struct PGXCOLONYRUNTIME_API FPGXColonyTaskId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Tag;

	bool IsValid() const { return Tag.IsValid(); }
	bool operator==(const FPGXColonyTaskId& Other) const { return Tag == Other.Tag; }
};

USTRUCT(BlueprintType)
struct PGXCOLONYRUNTIME_API FPGXColonyNeedId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Tag;

	bool IsValid() const { return Tag.IsValid(); }
	bool operator==(const FPGXColonyNeedId& Other) const { return Tag == Other.Tag; }
};
