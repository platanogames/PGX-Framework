// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Profile/PGXProfileBuildValidator.h"

#if WITH_EDITOR

#include "Profile/PGXProfileDeviceProfileBridge.h"
#include "Logging/PGXLogCategories.h"

// ============================================================================
// ValidateForBuild
// ============================================================================

void FPGXProfileBuildValidator::ValidateForBuild(const FPGXResolvedProfile& Profile, TArray<FPGXBuildValidationResult>& OutResults)
{
	ValidateBudgetCompliance(Profile, OutResults);
	ValidateFeatureConsistency(Profile, OutResults);
	ValidatePolicyCompliance(Profile, OutResults);
}

// ============================================================================
// ValidateBudgetCompliance
// ============================================================================

void FPGXProfileBuildValidator::ValidateBudgetCompliance(const FPGXResolvedProfile& Profile, TArray<FPGXBuildValidationResult>& OutResults)
{
	// EN: Budget validation — check for obviously problematic configurations
	// ES: Validacion de presupuesto — verificar configuraciones obviamente problematicas

	if (Profile.Budgets.RAM_MB > 0 && Profile.Budgets.RAM_MB < 512)
	{
		FPGXBuildValidationResult Result;
		Result.Description = FText::FromString(FString::Printf(
			TEXT("RAM budget of %lld MB may be too low for most projects"), Profile.Budgets.RAM_MB));
		Result.Severity = EPGXBuildValidationSeverity::Warning;
		Result.RuleName = TEXT("BudgetRAMMin");
		OutResults.Add(Result);
	}

	if (Profile.Budgets.PSOBudget > 0 && Profile.Budgets.PSOBudget < 10)
	{
		FPGXBuildValidationResult Result;
		Result.Description = FText::FromString(FString::Printf(
			TEXT("PSO budget of %d may be too low for adequate warm-up coverage"), Profile.Budgets.PSOBudget));
		Result.Severity = EPGXBuildValidationSeverity::Warning;
		Result.RuleName = TEXT("BudgetPSOMin");
		OutResults.Add(Result);
	}

	if (Profile.Budgets.StreamingPool_MB > 0 && Profile.Budgets.StreamingPool_MB < 256)
	{
		FPGXBuildValidationResult Result;
		Result.Description = FText::FromString(FString::Printf(
			TEXT("Streaming pool budget of %lld MB may cause streaming hitches"), Profile.Budgets.StreamingPool_MB));
		Result.Severity = EPGXBuildValidationSeverity::Warning;
		Result.RuleName = TEXT("BudgetStreamingMin");
		OutResults.Add(Result);
	}
}

// ============================================================================
// ValidateFeatureConsistency
// ============================================================================

void FPGXProfileBuildValidator::ValidateFeatureConsistency(const FPGXResolvedProfile& Profile, TArray<FPGXBuildValidationResult>& OutResults)
{
	// EN: Check for inconsistent feature combinations
	// ES: Verificar combinaciones de features inconsistentes

	// VR mode should not have Nanite (performance concern)
	if (Profile.Identity.ProjectMode == EPGXProjectMode::VR
		&& Profile.Features.Nanite.Policy == EPGXFeaturePolicy::Allowed)
	{
		FPGXBuildValidationResult Result;
		Result.Description = FText::FromString(TEXT("VR project with Nanite enabled may cause performance issues. Consider FallbackRequired or Disallowed."));
		Result.Severity = EPGXBuildValidationSeverity::Warning;
		Result.RuleName = TEXT("FeatureVRNanite");
		OutResults.Add(Result);
	}

	// VR mode should not have Lumen (performance concern)
	if (Profile.Identity.ProjectMode == EPGXProjectMode::VR
		&& Profile.Features.Lumen.Policy == EPGXFeaturePolicy::Allowed)
	{
		FPGXBuildValidationResult Result;
		Result.Description = FText::FromString(TEXT("VR project with Lumen enabled may cause frame drops below 90fps. Consider ForwardShading instead."));
		Result.Severity = EPGXBuildValidationSeverity::Error;
		Result.RuleName = TEXT("FeatureVRLumen");
		OutResults.Add(Result);
	}

	// Mobile targets should not have Nanite/Lumen/RayTracing
	for (const EPGXTargetPlatform& Target : Profile.Identity.ActiveTargets)
	{
		if (Target == EPGXTargetPlatform::Mobile_iOS || Target == EPGXTargetPlatform::Mobile_Android)
		{
			if (Profile.Features.Nanite.Policy == EPGXFeaturePolicy::Allowed)
			{
				FPGXBuildValidationResult Result;
				Result.Description = FText::FromString(TEXT("Mobile target with Nanite Allowed — Nanite is not supported on mobile platforms."));
				Result.Severity = EPGXBuildValidationSeverity::Blocker;
				Result.RuleName = TEXT("FeatureMobileNanite");
				OutResults.Add(Result);
			}

			if (Profile.Features.RayTracing.Policy == EPGXFeaturePolicy::Allowed)
			{
				FPGXBuildValidationResult Result;
				Result.Description = FText::FromString(TEXT("Mobile target with RayTracing Allowed — Ray Tracing is not supported on mobile."));
				Result.Severity = EPGXBuildValidationSeverity::Blocker;
				Result.RuleName = TEXT("FeatureMobileRT");
				OutResults.Add(Result);
			}
			break; // EN: Only report once / ES: Solo reportar una vez
		}
	}

	// Switch should not have Nanite/Lumen
	for (const EPGXTargetPlatform& Target : Profile.Identity.ActiveTargets)
	{
		if (Target == EPGXTargetPlatform::Console_Switch)
		{
			if (Profile.Features.Nanite.Policy == EPGXFeaturePolicy::Allowed)
			{
				FPGXBuildValidationResult Result;
				Result.Description = FText::FromString(TEXT("Switch target with Nanite Allowed — Nanite is not supported on Switch."));
				Result.Severity = EPGXBuildValidationSeverity::Blocker;
				Result.RuleName = TEXT("FeatureSwitchNanite");
				OutResults.Add(Result);
			}

			if (Profile.Features.Lumen.Policy == EPGXFeaturePolicy::Allowed)
			{
				FPGXBuildValidationResult Result;
				Result.Description = FText::FromString(TEXT("Switch target with Lumen Allowed — Lumen is not practical on Switch."));
				Result.Severity = EPGXBuildValidationSeverity::Error;
				Result.RuleName = TEXT("FeatureSwitchLumen");
				OutResults.Add(Result);
			}
			break;
		}
	}

	// EN: Validate via Device Profile bridge
	// ES: Validar via puente de Device Profile
	TArray<FPGXDeviceProfileValidation> CVarMismatches = UPGXProfileDeviceProfileBridge::ValidateDeviceProfiles(nullptr, Profile);
	for (const auto& Mismatch : CVarMismatches)
	{
		FPGXBuildValidationResult Result;
		Result.Description = FText::FromString(FString::Printf(
			TEXT("CVar '%s' mismatch: expected=%s, actual=%s"),
			*Mismatch.CVarName, *Mismatch.ExpectedValue, *Mismatch.ActualValue));
		Result.Severity = EPGXBuildValidationSeverity::Warning;
		Result.RuleName = TEXT("CVarMismatch");
		OutResults.Add(Result);
	}
}

// ============================================================================
// ValidatePolicyCompliance
// ============================================================================

void FPGXProfileBuildValidator::ValidatePolicyCompliance(const FPGXResolvedProfile& Profile, TArray<FPGXBuildValidationResult>& OutResults)
{
	// EN: Check for contradictory policy configurations
	// ES: Verificar configuraciones de politica contradictorias

	// Save requires encryption but save data is disabled
	if (Profile.Policies.Security.bRequireEncryption && !Profile.Capabilities.bAllowSaveData)
	{
		FPGXBuildValidationResult Result;
		Result.Description = FText::FromString(TEXT("Encryption required but SaveData capability is disabled — encryption has no effect."));
		Result.Severity = EPGXBuildValidationSeverity::Warning;
		Result.RuleName = TEXT("PolicyEncryptionNoSave");
		OutResults.Add(Result);
	}

	// Persistent logs enabled but external writes disabled
	if (Profile.Capabilities.bAllowPersistentLogs && !Profile.Capabilities.bAllowExternalWrites)
	{
		FPGXBuildValidationResult Result;
		Result.Description = FText::FromString(TEXT("Persistent logs enabled but external writes disabled — logs cannot be written to disk."));
		Result.Severity = EPGXBuildValidationSeverity::Error;
		Result.RuleName = TEXT("PolicyLogNoWrite");
		OutResults.Add(Result);
	}

	// Shipping build context but dangerous commands allowed
	if (Profile.Identity.BuildContext == EPGXBuildContext::Shipping && Profile.Capabilities.bAllowDangerousCommands)
	{
		FPGXBuildValidationResult Result;
		Result.Description = FText::FromString(TEXT("Shipping build with dangerous commands allowed — this is a security risk."));
		Result.Severity = EPGXBuildValidationSeverity::Blocker;
		Result.RuleName = TEXT("PolicyShippingDangerousCmd");
		OutResults.Add(Result);
	}
}

// ============================================================================
// HasBlockers
// ============================================================================

bool FPGXProfileBuildValidator::HasBlockers(const TArray<FPGXBuildValidationResult>& Results)
{
	for (const auto& R : Results)
	{
		if (R.Severity == EPGXBuildValidationSeverity::Blocker)
		{
			return true;
		}
	}
	return false;
}

#endif // WITH_EDITOR
