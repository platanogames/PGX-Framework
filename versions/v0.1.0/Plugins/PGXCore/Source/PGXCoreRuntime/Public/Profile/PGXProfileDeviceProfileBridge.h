// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Profile/PGXProfileTypes.h"
#include "PGXProfileDeviceProfileBridge.generated.h"

// ============================================================================
// EN: Bridge between PGX Profile System and UE Device Profiles.
//     Validates that current Device Profile CVars are consistent with the
//     resolved PGX profile, and can generate suggested .ini configurations.
//
// ES: Puente entre el Sistema de Profile PGX y UE Device Profiles.
//     Valida que los CVars del Device Profile actual sean consistentes con el
//     profile PGX resuelto, y puede generar configuraciones .ini sugeridas.
// ============================================================================

/**
 * EN: Single CVar validation result.
 * ES: Resultado de validacion de un CVar individual.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXDeviceProfileValidation
{
	GENERATED_BODY()

	/** EN: CVar name / ES: Nombre del CVar */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile|DeviceProfile")
	FString CVarName;

	/** EN: Expected value based on profile / ES: Valor esperado basado en el profile */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile|DeviceProfile")
	FString ExpectedValue;

	/** EN: Current actual value / ES: Valor actual real */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile|DeviceProfile")
	FString ActualValue;

	/** EN: Whether values match / ES: Si los valores coinciden */
	UPROPERTY(BlueprintReadOnly, Category = "PGX|Profile|DeviceProfile")
	bool bMatches = true;
};

UCLASS()
class PGXCORERUNTIME_API UPGXProfileDeviceProfileBridge : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * EN: Validate current Device Profile CVars against the resolved PGX profile.
	 *     Returns mismatches that may indicate configuration drift.
	 * ES: Validar CVars del Device Profile actual contra el profile PGX resuelto.
	 *     Retorna discrepancias que pueden indicar deriva de configuracion.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|DeviceProfile",
		meta = (WorldContext = "WorldContextObject"))
	static TArray<FPGXDeviceProfileValidation> ValidateDeviceProfiles(
		const UObject* WorldContextObject,
		const FPGXResolvedProfile& Profile);

	/**
	 * EN: Generate suggested DefaultDeviceProfiles.ini content based on profile.
	 * ES: Generar contenido sugerido de DefaultDeviceProfiles.ini basado en profile.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Profile|DeviceProfile")
	static FString GenerateDeviceProfileINI(const FPGXResolvedProfile& Profile);

#if WITH_EDITOR
	/**
	 * EN: Preview which CVars would be affected by the profile.
	 * ES: Previsualizar cuales CVars serian afectados por el profile.
	 */
	static TArray<FPGXDeviceProfileValidation> PreviewCVarImpact(const FPGXResolvedProfile& Profile);
#endif
};
