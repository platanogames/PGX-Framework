// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PGXEnvironmentTypes.generated.h"

// ============================================================================
// EN: Enumerations
// ES: Enumeraciones
// ============================================================================

/**
 * EN: Typed result codes for PGXEnvironment public API. Authoring Guide the public API
 *     invariant 5 (no silent failures): every error path returns a typed
 *     code + reason string, mirrored to log.
 * ES: Codigos de resultado tipados para la API publica de PGXEnvironment.
 *     Invariante authoring invariant 5 (sin fallos silenciosos): cada path de error retorna
 *     codigo tipado + reason string, espejado a log.
 */
UENUM(BlueprintType)
enum class EPGXEnvironmentResultCode : uint8
{
	Success,                 // EN: Operation completed successfully
	NotFound,                // EN: Requested zone / variable / band not registered
	InvalidConfig,           // EN: Config DA / DataAsset rejected by validation
	AlreadyRegistered,       // EN: Zone tag already in registry
	OutOfBounds,             // EN: Modifier delta or value outside authored ClampMin/Max range
	Unsupported,             // EN: Feature is declared but not yet implemented (placeholder)
	Failed                   // EN: Generic failure (fallback when no more specific code applies)
};

/**
 * EN: Authored severity bands for environmental thresholds. Severity.None
 *     means "outside any authored band" (neutral). Higher levels indicate
 *     escalating environmental hazard intensity.
 * ES: Bandas de severidad authoring para umbrales ambientales. Severity.None
 *     significa "fuera de cualquier banda authoring" (neutral). Niveles mas
 *     altos indican intensidad escalante de peligro ambiental.
 */
UENUM(BlueprintType)
enum class EPGXEnvironmentSeverity : uint8
{
	None,
	Minor,
	Moderate,
	Severe,
	Critical
};

/**
 * EN: Variable kind — drives interpretation of stored value and threshold
 *     comparison rules (e.g. Continuous vs Discrete band semantics).
 *     Runtime supports Continuous; Discrete and Boolean are reserved values
 *     retained to keep the enum stable.
 * ES: Tipo de variable — guia interpretacion del valor almacenado y reglas
 *     de comparacion de umbral (ej. semantica de banda Continuous vs Discrete).
 *     Runtime soporta Continuous; Discrete y Boolean son valores reservados
 *     para mantener el enum estable.
 */
UENUM(BlueprintType)
enum class EPGXEnvironmentVariableKind : uint8
{
	Continuous,
	Discrete,
	Boolean
};

// ============================================================================
// EN: Structs
// ES: Structs
// ============================================================================

/**
 * EN: Stable identity for an environmental variable. Tag-driven keying so
 *     project content can extend the variable taxonomy without source
 *     mutation. the public API.
 * ES: Identidad estable para una variable ambiental. Keying por tag para que
 *     el contenido del proyecto pueda extender la taxonomia de variables sin
 *     mutar source. the public API.
 */
USTRUCT(BlueprintType)
struct PGXENVIRONMENTRUNTIME_API FPGXEnvironmentVariableKey
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment",
		meta = (Categories = "PGX.Environment.Variable"))
	FGameplayTag VariableTag;
};

/**
 * EN: Authored threshold band for a variable. Inclusive lower / exclusive
 *     upper convention. Severity is the band label emitted on transition.
 *     Project / Object DA authoring drives the actual values (no hardcoded
 *     limits in source per authoring invariant 1).
 * ES: Banda de umbral authoring para una variable. Convencion inclusivo en el
 *     limite inferior / exclusivo en el superior. Severity es la label de
 *     banda emitida en transicion. El authoring de Project / Object DA guia
 *     los valores reales (sin limites hardcoded en source per the public API
 *     invariante 1).
 */
USTRUCT(BlueprintType)
struct PGXENVIRONMENTRUNTIME_API FPGXEnvironmentThresholdBand
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Threshold")
	EPGXEnvironmentSeverity Severity = EPGXEnvironmentSeverity::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Threshold")
	float LowerBoundInclusive = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PGX|Environment|Threshold")
	float UpperBoundExclusive = 0.0f;
};

/**
 * EN: Standard return shape for PGXEnvironment public mutations. Pairs a
 *     typed code with a human-readable description (also fed to UE_LOG by
 *     the subsystem before returning, per authoring invariant 5 "typed result +
 *     log + no silent failure"). MakeSuccess / MakeFail factories make
 *     call-site usage uniform.
 * ES: Shape estandar de retorno para mutaciones publicas de PGXEnvironment.
 *     Pareja codigo tipado + descripcion legible (tambien enviado a UE_LOG
 *     por el subsistema antes del return, per authoring invariante 5 "typed
 *     result + log + no silent failure"). Factories MakeSuccess / MakeFail
 *     uniforman uso en call-sites.
 */
USTRUCT(BlueprintType)
struct PGXENVIRONMENTRUNTIME_API FPGXEnvironmentResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Environment")
	EPGXEnvironmentResultCode Code = EPGXEnvironmentResultCode::Failed;

	UPROPERTY(BlueprintReadOnly, Category = "PGX|Environment")
	FString Description;

	static FPGXEnvironmentResult MakeSuccess(const FString& InDescription = FString())
	{
		FPGXEnvironmentResult R;
		R.Code = EPGXEnvironmentResultCode::Success;
		R.Description = InDescription;
		return R;
	}

	static FPGXEnvironmentResult MakeFail(EPGXEnvironmentResultCode InCode, const FString& InDescription)
	{
		FPGXEnvironmentResult R;
		R.Code = InCode;
		R.Description = InDescription;
		return R;
	}
};
