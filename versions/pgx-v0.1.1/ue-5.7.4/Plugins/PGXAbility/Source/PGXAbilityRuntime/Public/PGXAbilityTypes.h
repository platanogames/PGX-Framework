// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "ActiveGameplayEffectHandle.h"
#include "PGXAbilityTypes.generated.h"

class UGameplayAbility;
class UGameplayEffect;

/**
 * EN: Typed result codes for PGXAbility public API surfaces. Mirrors the result-policy
 *     convention used across FPGXAIResult / FPGXTradeResult / FPGXCraftingResult.
 * ES: Codigos de resultado tipados para la API publica de PGXAbility. Sigue la convencion
 *     uniforme FPGXAIResult / FPGXTradeResult / FPGXCraftingResult.
 */
UENUM(BlueprintType)
enum class EPGXAbilityResultCode : uint8
{
	Success                 UMETA(DisplayName = "Success"),
	SubsystemUnavailable    UMETA(DisplayName = "Subsystem Unavailable"),
	ComponentUnavailable    UMETA(DisplayName = "Ability Component Unavailable"),
	InvalidInput            UMETA(DisplayName = "Invalid Input"),
	NotFound                UMETA(DisplayName = "Not Found"),
	AlreadyGranted          UMETA(DisplayName = "Already Granted"),
	ActivationFailed        UMETA(DisplayName = "Activation Failed"),
	AttributeNotFound       UMETA(DisplayName = "Attribute Not Found"),
	OutOfBounds             UMETA(DisplayName = "Out Of Bounds"),
	Failure                 UMETA(DisplayName = "Failure")
};

/**
 * EN: Typed result for PGXAbility public API. `bSucceeded` is the canonical success
 *     predicate; `Code` carries the enum reason; `DiagnosticMessage` is human-readable
 *     detail for logs and inspector telemetry.
 * ES: Resultado tipado para la API publica de PGXAbility. `bSucceeded` es el predicado
 *     canonico de exito; `Code` lleva la razon enum; `DiagnosticMessage` es texto
 *     humano-legible para logs e inspector.
 */
USTRUCT(BlueprintType)
struct PGXABILITYRUNTIME_API FPGXAbilityResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	bool bSucceeded = true;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	EPGXAbilityResultCode Code = EPGXAbilityResultCode::Success;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	FString DiagnosticMessage;

	static FPGXAbilityResult MakeSuccess(const FString& InMessage = FString())
	{
		FPGXAbilityResult R;
		R.bSucceeded = true;
		R.Code = EPGXAbilityResultCode::Success;
		R.DiagnosticMessage = InMessage;
		return R;
	}

	static FPGXAbilityResult MakeFailure(EPGXAbilityResultCode InCode, const FString& InMessage)
	{
		FPGXAbilityResult R;
		R.bSucceeded = false;
		R.Code = InCode;
		R.DiagnosticMessage = InMessage;
		return R;
	}
};

/**
 * EN: Stable PGX-facing handle for a granted ability. Wraps the raw GAS
 *     `FGameplayAbilitySpecHandle` so other PGX plugins never see GAS spec types directly
 *     (architecture design section 4 boundary). Carries the ability tag for display/debug without a
 *     second lookup.
 * ES: Handle PGX estable para una ability concedida. Envuelve el `FGameplayAbilitySpecHandle`
 *     crudo de GAS para que otros plugins PGX nunca vean tipos spec de GAS directamente.
 */
USTRUCT(BlueprintType)
struct PGXABILITYRUNTIME_API FPGXAbilityHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	FGameplayTag AbilityTag;

	/** EN: Internal GAS handle. Not BlueprintReadOnly by design — Blueprint identifies abilities by AbilityTag, never by raw spec handle. */
	FGameplayAbilitySpecHandle SpecHandle;

	bool IsValid() const { return SpecHandle.IsValid(); }

	bool operator==(const FPGXAbilityHandle& Other) const { return SpecHandle == Other.SpecHandle; }
	bool operator!=(const FPGXAbilityHandle& Other) const { return !(*this == Other); }
};

FORCEINLINE uint32 GetTypeHash(const FPGXAbilityHandle& Handle)
{
	return GetTypeHash(Handle.SpecHandle);
}

/**
 * EN: Stable PGX-facing handle for an active gameplay effect. Wraps the raw GAS
 *     `FActiveGameplayEffectHandle`, same boundary rationale as `FPGXAbilityHandle`.
 * ES: Handle PGX estable para un gameplay effect activo. Envuelve el
 *     `FActiveGameplayEffectHandle` crudo de GAS.
 */
USTRUCT(BlueprintType)
struct PGXABILITYRUNTIME_API FPGXEffectHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	FGameplayTag EffectTag;

	/** EN: Internal GAS handle. Not BlueprintReadOnly by design. */
	FActiveGameplayEffectHandle SpecHandle;

	bool IsValid() const { return SpecHandle.IsValid(); }

	bool operator==(const FPGXEffectHandle& Other) const { return SpecHandle == Other.SpecHandle; }
	bool operator!=(const FPGXEffectHandle& Other) const { return !(*this == Other); }
};

/** EN: Read-only snapshot of one granted ability, for query/inspector consumers. / ES: Snapshot de solo lectura de una ability concedida. */
USTRUCT(BlueprintType)
struct PGXABILITYRUNTIME_API FPGXAbilitySnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	FPGXAbilityHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	bool bIsActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	float CooldownRemainingSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	int32 Level = 1;
};

/** EN: Read-only snapshot of one tracked attribute, for query/inspector consumers. / ES: Snapshot de solo lectura de un atributo rastreado. */
USTRUCT(BlueprintType)
struct PGXABILITYRUNTIME_API FPGXAttributeSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	FGameplayTag AttributeTag;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	float CurrentValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	float BaseValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	float ClampMin = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	float ClampMax = 0.0f;
};

/** EN: Read-only snapshot of one active gameplay effect, for query/inspector consumers. / ES: Snapshot de solo lectura de un efecto activo. */
USTRUCT(BlueprintType)
struct PGXABILITYRUNTIME_API FPGXEffectSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	FPGXEffectHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	float RemainingDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Ability")
	int32 StackCount = 1;
};
