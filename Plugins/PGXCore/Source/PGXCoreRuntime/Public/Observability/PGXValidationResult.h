// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXValidationResult.generated.h"

/**
 * EN: Per-field validation error reported by the PGX Observability Framework's `FromJson()`
 *     and asset-validation pipelines. Carries a typed code (FName), a JSON-pointer-style
 *     field path (e.g. `data.MaxScreenStackDepth`), and a human-readable message (FText —
 *     localizable per editor culture). Attach to `FPGXValidationResult::Errors` for hard
 *     failures.
 * ES: Error de validacion por-campo. Codigo tipado + path + mensaje localizable.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXValidationError
{
	GENERATED_BODY()

	/** EN: Typed error code (e.g. `SchemaMismatch`, `MissingRequired`, `OutOfRange`). */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FName Code;

	/** EN: JSON-pointer-style path to the offending field (e.g. `data.MaxScreenStackDepth`). */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FString Path;

	/** EN: Human-readable diagnostic message. Localizable. */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FText Message;

	static FPGXValidationError Make(FName InCode, const FString& InPath, const FText& InMessage)
	{
		FPGXValidationError E;
		E.Code = InCode;
		E.Path = InPath;
		E.Message = InMessage;
		return E;
	}
};

/**
 * EN: Per-field validation warning. Same shape as Error, but soft-fail policy: warnings do
 *     NOT cause `FPGXValidationResult::bValid` to flip to false. Use for migration tooling
 *     hints, deprecation notices, schema-version-coercion notes, etc.
 * ES: Warning de validacion. Misma forma que Error, soft-fail.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXValidationWarning
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FName Code;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FString Path;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	FText Message;

	static FPGXValidationWarning Make(FName InCode, const FString& InPath, const FText& InMessage)
	{
		FPGXValidationWarning W;
		W.Code = InCode;
		W.Path = InPath;
		W.Message = InMessage;
		return W;
	}
};

/**
 * EN: Outcome of `IPGXObservable::FromJson()` and asset-validation pipelines. `bValid`
 *     is the canonical success predicate: false when any `Errors` entry is present;
 *     `Warnings` do not affect `bValid`. Soft-fail mode (warnings only) is opt-in for
 *     migration tooling — check `Warnings.Num() > 0 && bValid` to detect migration-worthy
 *     payloads that still parsed successfully.
 *
 *     Default-constructed value is `bValid=true` with empty arrays — represents a clean
 *     pass.
 *
 * ES: Resultado de FromJson() y pipelines de validacion de asset. bValid=false cuando hay
 *     Errors. Warnings no afectan bValid. Soft-fail-mode opt-in para tooling de migracion.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXValidationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	bool bValid = true;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	TArray<FPGXValidationError> Errors;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Observability")
	TArray<FPGXValidationWarning> Warnings;

	/** EN: Append an error AND set bValid=false. */
	void AddError(FName Code, const FString& Path, const FText& Message)
	{
		Errors.Add(FPGXValidationError::Make(Code, Path, Message));
		bValid = false;
	}

	/** EN: Append a warning. Does NOT flip bValid. */
	void AddWarning(FName Code, const FString& Path, const FText& Message)
	{
		Warnings.Add(FPGXValidationWarning::Make(Code, Path, Message));
	}

	/** EN: Default-constructed pass. */
	static FPGXValidationResult MakeValid()
	{
		return FPGXValidationResult{};
	}

	/** EN: Single-error failure shortcut. */
	static FPGXValidationResult MakeFailure(FName Code, const FString& Path, const FText& Message)
	{
		FPGXValidationResult R;
		R.AddError(Code, Path, Message);
		return R;
	}

	/**
	 * EN: Ergonomic shortcut for callers that only have an `int32` error code
	 *     and a plain FString message (no Path, no FText localization). Converts
	 *     the int32 to FName (via FString::FromInt) and the FString to FText
	 *     (via FText::FromString) and emits a single error. Path is left empty.
	 *
	 *     Use this when migrating a Phase 2-audited plugin that has its own
	 *     `FPgxResult` (bSuccess + int32 Code + FString Message) —
	 *     `MakeFromCode(existingCode, existingMsg)` gives
	 *     a one-liner bridge into FPGXValidationResult.
	 *
	 * ES: Atajo ergonomico para callers que solo tienen un codigo int32 y un
	 *     FString plain (sin Path, sin FText localizable). Convierte el int32
	 *     a FName y el FString a FText y emite un unico error.
	 */
	static FPGXValidationResult MakeFromCode(int32 Code, const FString& Message)
	{
		FPGXValidationResult R;
		R.AddError(FName(*FString::FromInt(Code)), FString(), FText::FromString(Message));
		return R;
	}
};
