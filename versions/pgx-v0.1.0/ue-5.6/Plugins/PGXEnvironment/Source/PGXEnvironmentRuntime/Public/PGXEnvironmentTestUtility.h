// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXEnvironmentTestUtility.generated.h"

class UPGXEnvironmentSubsystem;

/**
 * Blueprint-callable validation helpers for type contracts, native tags,
 * authored defaults and world-backed environment behavior.
 *
 * Validators that require runtime state accept a WorldContextObject. Modifier
 * and threshold checks also require an ActiveConfig with at least one variable
 * and zone definition. RunAllEnvironmentTests aggregates individual failures in
 * OutIssues and returns false when a required check fails.
 */
UCLASS()
class PGXENVIRONMENTRUNTIME_API UPGXEnvironmentTestUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: World-independent validators (NewObject + static tag reads only)
	// ES: Validadores world-independent (NewObject + lecturas estaticas tag)
	// ========================================================================

	/**
	 * EN: Validate FPGXEnvironmentResult MakeSuccess / MakeFail roundtrip
	 *     and EPGXEnvironmentResultCode default. No world access.
	 * ES: Validar roundtrip MakeSuccess / MakeFail de FPGXEnvironmentResult
	 *     y default de EPGXEnvironmentResultCode. Sin acceso al world.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Environment|Test")
	static bool ValidateTypeContracts(TArray<FString>& OutIssues);

	/**
	 * EN: Validate the four parent native gameplay tag handles + the five
	 *     Severity branch tags resolve to valid tags. Catches the
	 *     RequestGameplayTag(name, false) regression class at startup.
	 * ES: Validar que los cuatro tag handles padre nativos + los cinco
	 *     tags de la rama Severity resuelven a tags validos. Captura la
	 *     clase de regresion RequestGameplayTag(name, false) en startup.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Environment|Test")
	static bool ValidateTagHandles(TArray<FString>& OutIssues);

	/**
	 * EN: Validate UPGXEnvironmentVariable / UPGXEnvironmentZoneDefinition /
	 *     UPGXEnvironmentTickProfile / UPGXEnvironmentConfig /
	 *     UPGXEnvironmentSettings construct cleanly with sane defaults
	 *     (InitialValue / ClampMin/Max within ranges, severity = None,
	 *     verbose flag = false, etc.).
	 * ES: Validar que UPGXEnvironmentVariable / UPGXEnvironmentZoneDefinition
	 *     / UPGXEnvironmentTickProfile / UPGXEnvironmentConfig /
	 *     UPGXEnvironmentSettings construyen limpios con defaults sanos
	 *     (InitialValue / ClampMin/Max en rangos, severity = None, flag
	 *     verbose = false, etc.).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Environment|Test")
	static bool ValidateDataAssetDefaults(TArray<FString>& OutIssues);

	// ========================================================================
	// EN: World-bound validators
	// ES: Validadores world-bound
	// ========================================================================

	/**
	 * EN: Validate the world has a UPGXEnvironmentSubsystem reachable
	 *     through the standard GetWorld()->GetSubsystem path. Returns
	 *     false if WorldContextObject does not resolve to a world or the
	 *     subsystem is missing.
	 * ES: Validar que el world tiene un UPGXEnvironmentSubsystem
	 *     alcanzable via el path standard GetWorld()->GetSubsystem.
	 *     Retorna false si WorldContextObject no resuelve a un world o
	 *     el subsistema esta ausente.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Environment|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ValidateSubsystemReady(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/**
	 * EN: Validate the public zone-registry surface end-to-end on the
	 *     live subsystem: invalid-tag rejection, RegisterZone success,
	 *     IsZoneRegistered query, GetRegisteredZoneTags membership,
	 *     duplicate-register rejection, UnregisterZone success,
	 *     unregistered-unregister rejection. Cleans up after itself.
	 * ES: Validar el surface publico del zone-registry end-to-end sobre
	 *     el subsistema vivo: rechazo de tag invalido, exito RegisterZone,
	 *     query IsZoneRegistered, membership GetRegisteredZoneTags,
	 *     rechazo register-duplicado, exito UnregisterZone, rechazo
	 *     unregister-no-registrado. Limpia tras de si.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Environment|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ValidateZoneRegistration(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/**
	 * EN: Validate ApplyVariableModifier honors clamp bounds + emits a
	 *     finite Description. Requires an authored ActiveConfig with at
	 *     least one Variable + ZoneDefinition; gracefully reports
	 *     authoring gap when absent (does NOT contaminate result with
	 *     environment skew).
	 * ES: Validar que ApplyVariableModifier honra los bounds de clamp y
	 *     emite una Description finita. Requiere un ActiveConfig
	 *     authoring con al menos una Variable + ZoneDefinition; reporta
	 *     graceful el gap de authoring cuando esta ausente (NO contamina
	 *     resultado con skew del environment).
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Environment|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ValidateModifierAndClamp(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	/**
	 * EN: Validate threshold-band transition emits OnZoneSeverityChangedNative
	 *     when ApplyVariableModifier moves a value across an authored band
	 *     boundary. Requires an authored ActiveConfig with at least one
	 *     Variable that has at least one ThresholdBand + at least one
	 *     ZoneDefinition seeded with that variable. Gracefully reports
	 *     authoring gap when absent.
	 * ES: Validar que la transicion de banda de threshold emite
	 *     OnZoneSeverityChangedNative cuando ApplyVariableModifier mueve
	 *     un valor a traves de un boundary de banda authoring. Requiere un
	 *     ActiveConfig authoring con al menos una Variable que tenga al
	 *     menos un ThresholdBand + al menos una ZoneDefinition seeded con
	 *     esa variable. Reporta graceful el gap de authoring cuando esta
	 *     ausente.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Environment|Test", meta = (WorldContext = "WorldContextObject"))
	static bool ValidateThresholdTransition(const UObject* WorldContextObject, TArray<FString>& OutIssues);

	// ========================================================================
	// EN: Aggregate
	// ES: Agregado
	// ========================================================================

	/**
	 * EN: Run all seven validators sequentially and aggregate. Returns
	 *     false if ANY leg failed. OutIssues accumulates every failure
	 *     reason across all legs prefixed by the leg name for triage.
	 * ES: Ejecutar los siete validadores secuencialmente y agregar.
	 *     Retorna false si CUALQUIER pierna fallo. OutIssues acumula
	 *     cada razon de fallo a traves de todas las piernas prefijado por
	 *     el nombre de la pierna para triage.
	 */
	UFUNCTION(BlueprintCallable, Category = "PGX|Environment|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunAllEnvironmentTests(const UObject* WorldContextObject, TArray<FString>& OutIssues);

private:
	static UPGXEnvironmentSubsystem* GetSubsystem(const UObject* WorldContextObject);
};
