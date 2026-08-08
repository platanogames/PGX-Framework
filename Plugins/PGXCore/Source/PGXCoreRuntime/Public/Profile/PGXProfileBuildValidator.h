// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "Profile/PGXProfileTypes.h"
#include "PGXProfileBuildValidator.generated.h"

// ============================================================================
// EN: Build validator for the PGX Profile System.
//     Validates that project assets and settings comply with the resolved profile.
//     Used by the editor and by the CI/CD commandlet.
//     Note: UENUM/USTRUCT/UCLASS must NOT be inside #if WITH_EDITOR (UHT limitation).
//     The non-UObject class FPGXProfileBuildValidator IS guarded since it has no reflection.
//
// ES: Validador de build para el Sistema de Profile PGX.
//     Valida que los assets y settings del proyecto cumplen con el profile resuelto.
//     Usado por el editor y por el commandlet CI/CD.
//     Nota: UENUM/USTRUCT/UCLASS NO deben estar dentro de #if WITH_EDITOR (limitacion UHT).
//     La clase no-UObject FPGXProfileBuildValidator SI esta guardada ya que no tiene reflexion.
// ============================================================================

/** EN: Severity level of a validation result / ES: Nivel de severidad de un resultado de validacion */
UENUM(BlueprintType)
enum class EPGXBuildValidationSeverity : uint8
{
	Info,
	Warning,
	Error,
	Blocker
};

/**
 * EN: Single validation result from the build validator.
 * ES: Resultado individual de validacion del validador de build.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXBuildValidationResult
{
	GENERATED_BODY()

	/** EN: Human-readable description of the issue / ES: Descripcion legible del problema */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile|Validation")
	FText Description;

	/** EN: Severity of this finding / ES: Severidad de este hallazgo */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile|Validation")
	EPGXBuildValidationSeverity Severity = EPGXBuildValidationSeverity::Info;

	/** EN: Path to the offending asset (if applicable) / ES: Ruta al asset ofensivo (si aplica) */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile|Validation")
	FString AssetPath;

	/** EN: Name of the rule that triggered this result / ES: Nombre de la regla que disparo este resultado */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile|Validation")
	FName RuleName;
};

#if WITH_EDITOR

/**
 * EN: Build validator — checks project compliance against the resolved profile.
 *     This is a plain C++ class (no UObject reflection), safe inside #if WITH_EDITOR.
 * ES: Validador de build — verifica cumplimiento del proyecto contra el profile resuelto.
 *     Es una clase C++ pura (sin reflexion UObject), segura dentro de #if WITH_EDITOR.
 */
class PGXCORERUNTIME_API FPGXProfileBuildValidator
{
public:
	/**
	 * EN: Run all validations against the profile. Returns all findings.
	 * ES: Ejecutar todas las validaciones contra el profile. Retorna todos los hallazgos.
	 */
	static void ValidateForBuild(const FPGXResolvedProfile& Profile, TArray<FPGXBuildValidationResult>& OutResults);

	/**
	 * EN: Validate that current assets respect budget limits.
	 * ES: Validar que los assets actuales respeten los limites de presupuesto.
	 */
	static void ValidateBudgetCompliance(const FPGXResolvedProfile& Profile, TArray<FPGXBuildValidationResult>& OutResults);

	/**
	 * EN: Validate that feature matrix is consistent with project settings.
	 * ES: Validar que la matriz de features sea consistente con project settings.
	 */
	static void ValidateFeatureConsistency(const FPGXResolvedProfile& Profile, TArray<FPGXBuildValidationResult>& OutResults);

	/**
	 * EN: Validate that policies are coherent (no contradictions).
	 * ES: Validar que las politicas sean coherentes (sin contradicciones).
	 */
	static void ValidatePolicyCompliance(const FPGXResolvedProfile& Profile, TArray<FPGXBuildValidationResult>& OutResults);

	/**
	 * EN: Check if any results are blockers.
	 * ES: Verificar si alguno de los resultados son blockers.
	 */
	static bool HasBlockers(const TArray<FPGXBuildValidationResult>& Results);
};

#endif // WITH_EDITOR

// ============================================================================
// EN: Commandlet for CI/CD validation — invocable via RunUAT.
//     Exit code: 0 = pass, 1 = blocker found
//     Usage: RunUAT ... -run=PGXProfileValidate
//     UCLASS must be outside #if WITH_EDITOR for UHT.
//
// ES: Commandlet para validacion CI/CD — invocable via RunUAT.
//     Codigo de salida: 0 = pasa, 1 = blocker encontrado
//     UCLASS debe estar fuera de #if WITH_EDITOR para UHT.
// ============================================================================

UCLASS()
class PGXCORERUNTIME_API UPGXProfileValidateCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	int32 Main(const FString& Params) override;
};
