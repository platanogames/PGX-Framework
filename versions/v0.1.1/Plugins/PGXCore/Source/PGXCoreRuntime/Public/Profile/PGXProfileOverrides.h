// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Profile/PGXProfileTypes.h"
#include "PGXProfileOverrides.generated.h"

// ============================================================================
// EN: Override structs for platform-specific and build-specific profile adjustments.
//     Each override can selectively replace layers from the base profile.
//     Uses EditCondition to show/hide override data based on toggle bools.
//
// ES: Structs de override para ajustes especificos de plataforma y build.
//     Cada override puede reemplazar selectivamente capas del profile base.
//     Usa EditCondition para mostrar/ocultar datos de override basado en bools toggle.
// ============================================================================

/**
 * EN: Platform-specific override — can override Capabilities, Budgets, and Features.
 * ES: Override especifico de plataforma — puede sobreescribir Capabilities, Budgets y Features.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXPlatformOverride
{
	GENERATED_BODY()

	// ── Capabilities Override ──

	/** EN: Enable capabilities override for this platform / ES: Habilitar override de capabilities para esta plataforma */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Platform")
	bool bOverrideCapabilities = false;

	/** EN: Platform-specific capabilities (only used if bOverrideCapabilities is true) / ES: Capabilities especificas de plataforma */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Platform",
		meta = (EditCondition = "bOverrideCapabilities", EditConditionHides))
	FPGXProfileCapabilities Capabilities;

	// ── Budgets Override ──

	/** EN: Enable budgets override for this platform / ES: Habilitar override de budgets para esta plataforma */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Platform")
	bool bOverrideBudgets = false;

	/** EN: Platform-specific budgets (only used if bOverrideBudgets is true) / ES: Budgets especificos de plataforma */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Platform",
		meta = (EditCondition = "bOverrideBudgets", EditConditionHides))
	FPGXProfileBudgets Budgets;

	// ── Features Override ──

	/** EN: Enable features override for this platform / ES: Habilitar override de features para esta plataforma */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Platform")
	bool bOverrideFeatures = false;

	/** EN: Platform-specific features (only used if bOverrideFeatures is true) / ES: Features especificos de plataforma */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Platform",
		meta = (EditCondition = "bOverrideFeatures", EditConditionHides))
	FPGXProfileFeatureMatrix Features;
};

/**
 * EN: Build-context-specific override — can override Capabilities and Policies.
 * ES: Override especifico de contexto de build — puede sobreescribir Capabilities y Policies.
 */
USTRUCT(BlueprintType)
struct PGXCORERUNTIME_API FPGXBuildOverride
{
	GENERATED_BODY()

	// ── Capabilities Override ──

	/** EN: Enable capabilities override for this build context / ES: Habilitar override de capabilities para este contexto de build */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Build")
	bool bOverrideCapabilities = false;

	/** EN: Build-specific capabilities (only used if bOverrideCapabilities is true) / ES: Capabilities especificas del build */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Build",
		meta = (EditCondition = "bOverrideCapabilities", EditConditionHides))
	FPGXProfileCapabilities Capabilities;

	// ── Policies Override ──

	/** EN: Enable policies override for this build context / ES: Habilitar override de policies para este contexto de build */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Build")
	bool bOverridePolicies = false;

	/** EN: Build-specific policies (only used if bOverridePolicies is true) / ES: Policies especificas del build */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PGX|Profile|Override|Build",
		meta = (EditCondition = "bOverridePolicies", EditConditionHides))
	FPGXProfilePolicies Policies;
};
