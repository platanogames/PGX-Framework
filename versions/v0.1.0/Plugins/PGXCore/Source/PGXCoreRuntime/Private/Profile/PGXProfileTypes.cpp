// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Profile/PGXProfileTypes.h"

// ============================================================================
// FPGXResolvedProfile — Query Helpers
// ============================================================================

bool FPGXResolvedProfile::IsCapabilityAllowed(FName CapabilityName) const
{
	if (CapabilityName == TEXT("INIPersistence"))     return Capabilities.bAllowINIPersistence;
	if (CapabilityName == TEXT("SaveData"))           return Capabilities.bAllowSaveData;
	if (CapabilityName == TEXT("ExternalWrites"))     return Capabilities.bAllowExternalWrites;
	if (CapabilityName == TEXT("Exports"))            return Capabilities.bAllowExports;
	if (CapabilityName == TEXT("ConsoleCommands"))    return Capabilities.bAllowConsoleCommands;
	if (CapabilityName == TEXT("DangerousCommands"))  return Capabilities.bAllowDangerousCommands;
	if (CapabilityName == TEXT("Profiling"))          return Capabilities.bAllowProfiling;
	if (CapabilityName == TEXT("Telemetry"))          return Capabilities.bAllowTelemetry;
	if (CapabilityName == TEXT("PersistentLogs"))     return Capabilities.bAllowPersistentLogs;
	if (CapabilityName == TEXT("HotReload"))          return Capabilities.bAllowHotReload;

	// EN: Unknown capability — permissive by default
	// ES: Capacidad desconocida — permisivo por defecto
	return true;
}

bool FPGXResolvedProfile::IsFeatureAllowed(FName FeatureName) const
{
	const EPGXFeaturePolicy Policy = GetFeaturePolicy(FeatureName);
	return Policy == EPGXFeaturePolicy::Allowed;
}

EPGXFeaturePolicy FPGXResolvedProfile::GetFeaturePolicy(FName FeatureName) const
{
	if (FeatureName == TEXT("Nanite"))          return Features.Nanite.Policy;
	if (FeatureName == TEXT("Lumen"))           return Features.Lumen.Policy;
	if (FeatureName == TEXT("RayTracing"))      return Features.RayTracing.Policy;
	if (FeatureName == TEXT("VSM"))             return Features.VSM.Policy;
	if (FeatureName == TEXT("ForwardShading"))  return Features.ForwardShading.Policy;
	if (FeatureName == TEXT("MobileRenderer"))  return Features.MobileRenderer.Policy;
	if (FeatureName == TEXT("VirtualTextures")) return Features.VirtualTextures.Policy;

	// EN: Unknown feature — allowed by default
	// ES: Feature desconocido — permitido por defecto
	return EPGXFeaturePolicy::Allowed;
}

int64 FPGXResolvedProfile::GetBudget(FName BudgetName) const
{
	if (BudgetName == TEXT("RAM_MB"))           return Budgets.RAM_MB;
	if (BudgetName == TEXT("VRAM_MB"))          return Budgets.VRAM_MB;
	if (BudgetName == TEXT("Disk_MB"))          return Budgets.Disk_MB;
	if (BudgetName == TEXT("StreamingPool_MB")) return Budgets.StreamingPool_MB;
	if (BudgetName == TEXT("TextureMaxDim"))    return static_cast<int64>(Budgets.TextureMaxDim);
	if (BudgetName == TEXT("MeshMaxTris"))      return static_cast<int64>(Budgets.MeshMaxTris);
	if (BudgetName == TEXT("ShaderBudget"))     return static_cast<int64>(Budgets.ShaderBudget);
	if (BudgetName == TEXT("PSOBudget"))        return static_cast<int64>(Budgets.PSOBudget);
	if (BudgetName == TEXT("AudioMemory_MB"))   return static_cast<int64>(Budgets.AudioMemory_MB);

	// EN: Unknown budget — 0 means unlimited
	// ES: Presupuesto desconocido — 0 significa ilimitado
	return 0;
}

EPGXPersistenceBackend FPGXResolvedProfile::GetPersistenceBackend(FName SystemName) const
{
	if (SystemName == TEXT("Config")) return Policies.Persistence.ConfigBackend;
	if (SystemName == TEXT("Log"))    return Policies.Persistence.LogBackend;
	if (SystemName == TEXT("Save"))   return Policies.Persistence.SaveBackend;

	// EN: Unknown system — default to INI
	// ES: Sistema desconocido — por defecto INI
	return EPGXPersistenceBackend::INI;
}

FPGXResolvedProfile FPGXResolvedProfile::MakeDefaultPermissive()
{
	FPGXResolvedProfile Profile;

	// EN: Identity: PC Game, Development, Relaxed
	// ES: Identidad: PC Game, Development, Relaxed
	Profile.Identity.ProjectMode = EPGXProjectMode::Game;
	Profile.Identity.ActiveTargets.Add(EPGXTargetPlatform::PC);
	Profile.Identity.BuildContext = EPGXBuildContext::Development;
	Profile.Identity.RestrictionLevel = EPGXRestrictionLevel::Relaxed;

	// EN: Capabilities: all true (defaults)
	// ES: Capacidades: todas true (defaults)
	// (FPGXProfileCapabilities defaults are all true)

	// EN: Policies: default backends
	// ES: Politicas: backends por defecto
	// (FPGXProfilePolicies defaults are sensible)

	// EN: Budgets: all 0 = unlimited
	// ES: Presupuestos: todos 0 = ilimitado
	// (FPGXProfileBudgets defaults are all 0)

	// EN: Features: all Allowed (defaults)
	// ES: Features: todos Allowed (defaults)
	// (FPGXFeatureEntry defaults to Allowed)

	Profile.State = EPGXProfileState::Resolved;
	Profile.ResolvedTimestamp = FDateTime::UtcNow();

	return Profile;
}
