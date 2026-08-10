// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

class UWorld;

#include "CoreMinimal.h"
#include "Base/PGXGameInstanceSubsystem.h"
#include "Profile/PGXProfileTypes.h"
#include "Profile/PGXProfileDelegates.h"
#include "PGXProfileSubsystem.generated.h"

class UPGXProjectProfileConfig;
class UPGXPlatformConfig;

// ============================================================================
// EN: Profile Subsystem — the "constitutional court" of the PGX framework.
//     Discovers, resolves, and exposes the project profile as a single source of truth.
//     All subsystems query this to understand what is allowed, how to operate,
//     and what the resource limits are.
//
//     Init order: Profile → GameFlow → Log → Save → PSO → Widget → Audio
//
// ES: Subsistema de Profile — la "corte constitucional" del framework PGX.
//     Descubre, resuelve y expone el profile del proyecto como fuente unica de verdad.
//     Todos los subsistemas consultan esto para entender que esta permitido,
//     como operar, y cuales son los limites de recursos.
//
//     Orden de init: Profile → GameFlow → Log → Save → PSO → Widget → Audio
// ============================================================================

UCLASS()
class PGXCORERUNTIME_API UPGXProfileSubsystem : public UPGXGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ========================================================================
	// Dynamic Delegates (Blueprint-assignable)
	// ========================================================================

	/** EN: Fired when profile is first resolved / ES: Se dispara cuando el profile se resuelve por primera vez */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Profile|Delegates")
	FOnPGXProfileResolved OnProfileResolved;

	/** EN: Fired when profile changes / ES: Se dispara cuando el profile cambia */
	UPROPERTY(BlueprintAssignable, Category = "PGX|Profile|Delegates")
	FOnPGXProfileChanged OnProfileChanged;

	// ========================================================================
	// Native Delegates (C++ only)
	// ========================================================================

	FOnPGXProfileResolvedNative OnProfileResolvedNative;
	FOnPGXProfileChangedNative OnProfileChangedNative;
	FOnPGXProfilePreChangeNative OnProfilePreChangeNative;

	// ========================================================================
	// Query API — const, BlueprintPure
	// EN: All queries are thread-safe reads against the resolved profile.
	// ES: Todas las consultas son lecturas thread-safe contra el profile resuelto.
	// ========================================================================

	/** EN: Get the full resolved profile / ES: Obtener el profile resuelto completo */
	const FPGXResolvedProfile& GetResolvedProfile() const { return ResolvedProfile; }

	/** EN: Check if a named capability is enabled / ES: Verificar si una capacidad nombrada esta habilitada */
	bool IsCapabilityEnabled(FName CapabilityName) const;

	/** EN: Check if a named feature is allowed / ES: Verificar si un feature nombrado esta permitido */
	bool IsFeatureAllowed(FName FeatureName) const;

	/** EN: Get a named budget value (0 = unlimited) / ES: Obtener valor de presupuesto nombrado (0 = ilimitado) */
	int64 GetBudget(FName BudgetName) const;

	/** EN: Get persistence backend for a system / ES: Obtener backend de persistencia para un sistema */
	EPGXPersistenceBackend GetPersistenceBackend(FName SystemName) const;

	/** EN: Check if the profile is resolved / ES: Verificar si el profile esta resuelto */
	bool IsProfileResolved() const;

	/** EN: Get the current profile state / ES: Obtener el estado actual del profile */
	EPGXProfileState GetProfileState() const { return ResolvedProfile.State; }

	// ========================================================================
	// Platform Config API (v2.0)
	// EN: Access per-platform configuration with per-system budgets and traits.
	// ES: Acceso a configuracion por plataforma con presupuestos por sistema y traits.
	// ========================================================================

	/** EN: Get the active platform config for the current/simulated platform / ES: Obtener la config de plataforma activa para la plataforma actual/simulada */
	const UPGXPlatformConfig* GetActivePlatformConfig() const;

	/** EN: Get the platform config for a specific platform (nullptr if none) / ES: Obtener la config de plataforma para una plataforma especifica (nullptr si no hay) */
	const UPGXPlatformConfig* GetPlatformConfigFor(EPGXTargetPlatform Platform) const;

	// ========================================================================
	// Resolution Merge Helpers (public for test utility access)
	// EN: Multi-platform resolution rules: Capabilities=AND, Budgets=MIN, Features=MostRestrictive
	// ES: Reglas de resolucion multi-plataforma: Capabilities=AND, Budgets=MIN, Features=MasRestrictivo
	// ========================================================================

	static void MergeCapabilitiesAND(FPGXProfileCapabilities& Base, const FPGXProfileCapabilities& Override);
	static void MergeBudgetsMIN(FPGXProfileBudgets& Base, const FPGXProfileBudgets& Override);
	static void MergeFeaturesMostRestrictive(FPGXProfileFeatureMatrix& Base, const FPGXProfileFeatureMatrix& Override);
	static EPGXFeaturePolicy MostRestrictivePolicy(EPGXFeaturePolicy A, EPGXFeaturePolicy B);

	// ========================================================================
	// Static Access
	// EN: Cached weak pointer for fast access (pattern from PSO subsystem)
	// ES: Puntero debil cacheado para acceso rapido (patron del subsistema PSO)
	// ========================================================================

	static UPGXProfileSubsystem* GetCachedInstance() { return CachedInstance.Get(); }

	// ========================================================================
	// Editor Simulation (#if WITH_EDITOR)
	// EN: Simulate different platforms/builds without modifying the DA.
	// ES: Simular diferentes plataformas/builds sin modificar el DA.
	// ========================================================================

#if WITH_EDITOR
	/** EN: Simulate a specific target platform / ES: Simular una plataforma objetivo especifica */
	void SimulatePlatform(EPGXTargetPlatform Platform);

	/** EN: Simulate a specific build context / ES: Simular un contexto de build especifico */
	void SimulateBuildContext(EPGXBuildContext BuildContext);

	/** EN: Clear all simulation overrides and restore original profile / ES: Limpiar overrides de simulacion y restaurar profile original */
	void ClearSimulationOverrides();

	/** EN: Check if simulation overrides are active / ES: Verificar si hay overrides de simulacion activos */
	bool HasSimulationOverrides() const { return bSimulationActive; }
#endif

private:
	// ========================================================================
	// Internal — Discovery & Resolution
	// ========================================================================

	void DiscoverProfiles();
	FPGXResolvedProfile ResolveProfile(const UPGXProjectProfileConfig* Config) const;
	void ApplyPlatformOverrides(const UPGXProjectProfileConfig* Config, FPGXResolvedProfile& OutProfile) const;
	void ApplyBuildOverrides(const UPGXProjectProfileConfig* Config, FPGXResolvedProfile& OutProfile) const;
	void ApplyMultiPlatformResolution(FPGXResolvedProfile& OutProfile) const;
	void LockProfile();

	// ========================================================================
	// Internal — Console Commands
	// ========================================================================
#if WITH_EDITOR
#endif
	// ========================================================================
	// Internal — State
	// ========================================================================

	FPGXResolvedProfile ResolvedProfile;

	UPROPERTY()
	TArray<TObjectPtr<UPGXProjectProfileConfig>> DiscoveredConfigs;

	UPROPERTY()
	TObjectPtr<UPGXProjectProfileConfig> ActiveConfig;

	UPROPERTY()
	TMap<EPGXTargetPlatform, TObjectPtr<UPGXPlatformConfig>> LoadedPlatformConfigs;

	UPROPERTY()
	TObjectPtr<UPGXPlatformConfig> ActivePlatformConfig;

	void LoadPlatformConfigs();
	static TWeakObjectPtr<UPGXProfileSubsystem> CachedInstance;

#if WITH_EDITOR
	FPGXResolvedProfile PreSimulationProfile;
	bool bSimulationActive = false;

	void ReResolveWithSimulation();
#endif

private:
	friend class FPGXCoreRuntimeModule;
	void ExecuteConsoleCommand(const FString& CommandName, const TArray<FString>& Args, UWorld* World);
};
