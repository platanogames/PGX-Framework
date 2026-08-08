// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Base/PGXEngineSubsystem.h"
#include "PGXMGOSTypes.h"
#include "PGXMGOSDelegates.h"
#include "PGXGCObserverSubsystem.generated.h"

class UPGXGCObserverConfig;
struct IConsoleCommand;

/**
 * EN: GC Observability Subsystem — captures pre/post GC snapshots, computes diffs,
 *     classifies behavioral patterns, and emits typed events without modifying the engine.
 *     Lives as UEngineSubsystem (entire application lifetime).
 *
 * ES: Subsistema de Observabilidad GC — captura snapshots pre/post GC, computa diffs,
 *     clasifica patrones comportamentales y emite eventos tipados sin modificar el engine.
 *     Vive como UEngineSubsystem (toda la vida de la aplicacion).
 */
UCLASS(BlueprintType)
class PGXMGOSRUNTIME_API UPGXGCObserverSubsystem : public UPGXEngineSubsystem
{
	GENERATED_BODY()

public:
	// ====================================================================
	// Lifecycle
	// ====================================================================

	//~ Begin USubsystem Interface
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	//~ End USubsystem Interface

	// ====================================================================
	// Dynamic Delegates (BP-compatible)
	// ====================================================================

	/** EN: Fired at the start of a GC cycle / ES: Se dispara al inicio de un ciclo GC */
	UPROPERTY(BlueprintAssignable, Category = "PGX|MGOS|Events")
	FOnPGXGCStarted OnGCStarted;

	/** EN: Fired when a GC cycle completes / ES: Se dispara cuando un ciclo GC completa */
	UPROPERTY(BlueprintAssignable, Category = "PGX|MGOS|Events")
	FOnPGXGCCompleted OnGCCompleted;

	/** EN: Fired when the profile state changes / ES: Se dispara cuando cambia el estado del perfil */
	UPROPERTY(BlueprintAssignable, Category = "PGX|MGOS|Events")
	FOnPGXGCProfileStateChanged OnProfileStateChanged;

	/** EN: Fired when an incident is raised / ES: Se dispara cuando se genera un incidente */
	UPROPERTY(BlueprintAssignable, Category = "PGX|MGOS|Events")
	FOnPGXGCIncidentRaised OnIncidentRaised;

	// ====================================================================
	// Native Delegates (C++ only)
	// ====================================================================

	FOnPGXGCStartedNative OnGCStartedNative;
	FOnPGXGCCompletedNative OnGCCompletedNative;
	FOnPGXGCProfileStateChangedNative OnProfileStateChangedNative;
	FOnPGXGCIncidentRaisedNative OnIncidentRaisedNative;

	// ====================================================================
	// Public API — Query
	// ====================================================================

	/** EN: Get current behavioral profile / ES: Obtener perfil comportamental actual */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query")
	FPGXGCProfile GetCurrentProfile() const { return CurrentProfile; }

	/** EN: Get all active incidents / ES: Obtener todos los incidentes activos */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query")
	const TArray<FPGXGCIncident>& GetCurrentIncidents() const { return ActiveIncidents; }

	/** EN: Get health reports for all tracked classes / ES: Obtener reportes de salud para todas las clases rastreadas */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Query")
	TArray<FPGXGCClassHealthReport> GetTrackedClassReport() const;

	/** EN: Get recent diffs as history summary / ES: Obtener diffs recientes como resumen de historial */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Query")
	TArray<FPGXGCSnapshotDiff> GetHistorySummary(int32 Count = 10) const;

	/** EN: Get current baseline data / ES: Obtener datos actuales del baseline */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query")
	FPGXGCBaseline GetCurrentBaseline() const { return CurrentBaseline; }

	// ====================================================================
	// Public API — Control
	// ====================================================================

	/** EN: Set the observer operating mode / ES: Establecer el modo de operacion del observador */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS")
	void SetMode(EPGXGCObserverMode NewMode);

	/** EN: Request immediate baseline capture / ES: Solicitar captura inmediata del baseline */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS")
	void RequestBaselineCapture();

	/** EN: Reset baseline to uninitialized / ES: Resetear baseline a no inicializado */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS")
	void ResetBaseline();

	/** EN: Set suppression mode — capture only, no analysis [R4] / ES: Establecer modo de supresion — solo captura, sin analisis [R4] */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS")
	void SetSuppressed(bool bSuppress);

	// ====================================================================
	// Public API — State
	// ====================================================================

	/** EN: Check if subsystem is initialized / ES: Verificar si el subsistema esta inicializado */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query")
	bool IsInitialized() const { return bIsInitialized; }

	/** EN: Get current mode / ES: Obtener modo actual */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query")
	EPGXGCObserverMode GetMode() const { return CurrentMode; }

	/** EN: Get current baseline state / ES: Obtener estado actual del baseline */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query")
	EPGXGCBaselineState GetBaselineState() const { return CurrentBaseline.BaselineState; }

	/** EN: Get current cycle count / ES: Obtener conteo actual de ciclos */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query")
	int64 GetCycleCount() const { return static_cast<int64>(NextCycleID - 1); }

	/** EN: Check if in suppressed phase [R4] / ES: Verificar si esta en fase de supresion [R4] */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query")
	bool IsInSuppressedPhase() const { return bIsSuppressed; }

	// ====================================================================
	// Static Access
	// ====================================================================

	/** EN: Get cached singleton instance / ES: Obtener instancia singleton cacheada */
	static UPGXGCObserverSubsystem* GetCachedInstance();

private:
	// ====================================================================
	// GC Hooks
	// ====================================================================

	void OnPreGarbageCollect();
	void OnPostGarbageCollect();

	// ====================================================================
	// Engine Hooks
	// ====================================================================

	void OnPreLoadMap(const FString& MapName);
	void OnPostLoadMapWithWorld(UWorld* LoadedWorld);
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	// ====================================================================
	// Inter-Cycle Monitoring [R5]
	// ====================================================================

	bool OnInterCycleCheck(float DeltaTime);

	// ====================================================================
	// Snapshot Builder
	// ====================================================================

	FPGXGCSnapshot CaptureSnapshot(EPGXGCPhase Phase);
	void CaptureTrackedClassCounts(FPGXGCSnapshot& OutSnapshot);
	float CaptureProcessMemory() const;

	// ====================================================================
	// Diff Builder
	// ====================================================================

	FPGXGCSnapshotDiff BuildDiff(const FPGXGCSnapshot& Pre, const FPGXGCSnapshot& Post, double Duration);

	// ====================================================================
	// Profile Engine
	// ====================================================================

	void EvaluateProfile(const FPGXGCSnapshot& PostSnapshot, const FPGXGCSnapshotDiff& Diff);
	void EvaluateClassHealth();
	EPGXGCProfileState ClassifyState(const FPGXGCSnapshot& Snapshot, const FPGXGCSnapshotDiff& Diff);
	bool DetectBurstClean(const FPGXGCSnapshotDiff& Diff) const;
	bool DetectPKSaturation(const FPGXGCSnapshot& Snapshot) const;
	bool DetectRootExpansion(const FPGXGCSnapshotDiff& Diff) const;
	void CheckNonUObjectLeak(const FPGXGCSnapshot& Snapshot);

	// ====================================================================
	// Baseline Management
	// ====================================================================

	void TryCaptureBaseline(const FPGXGCSnapshot& Snapshot);
	void ValidateBaseline();
	void InvalidateBaseline();

	// ====================================================================
	// Incident Management
	// ====================================================================

	void RaiseIncident(EPGXGCSeverity Severity, FName Category, const FString& Description);

	// ====================================================================
	// Utility
	// ====================================================================

	EPGXExecutionMode DetermineExecutionMode() const;
	bool IsClassExcluded(FName ClassName) const;
	void DiscoverConfig();
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	// ====================================================================
	// Internal State
	// ====================================================================

	FPGXGCHistoryStore HistoryStore;
	FPGXGCBaseline CurrentBaseline;
	FPGXGCProfile CurrentProfile;
	TArray<FPGXGCIncident> ActiveIncidents;
	TArray<FPGXGCClassHealthReport> TrackedClassReports;

	FPGXGCSnapshot PreSnapshot;
	uint64 NextCycleID = 1;
	double CycleStartTime = 0.0;

	EPGXGCObserverMode CurrentMode = EPGXGCObserverMode::Passive;

	UPROPERTY()
	TObjectPtr<UPGXGCObserverConfig> ActiveConfig;

	bool bIsInitialized = false;
	bool bIsSuppressed = false;

	/** EN: Last UObject count for inter-cycle monitoring [R5] / ES: Ultimo conteo de UObjects para monitoreo inter-ciclo [R5] */
	uint32 LastInterCycleUObjectCount = 0;

	/** EN: Ticker handle for inter-cycle checks [R5] / ES: Handle del ticker para chequeos inter-ciclo [R5] */
	FTSTicker::FDelegateHandle InterCycleTickerHandle;

	/** EN: Resolved excluded class names for fast lookup [R4] / ES: Nombres de clases excluidas resueltas para busqueda rapida [R4] */
	TSet<FName> ResolvedExcludedClasses;

	/** EN: Resolved tracked classes for TObjectIterator [R3] / ES: Clases rastreadas resueltas para TObjectIterator [R3] */
	TArray<UClass*> ResolvedTrackedClasses;

	/** EN: Console commands / ES: Comandos de consola */
	TArray<IConsoleCommand*> RegisteredCommands;

	/** EN: Cycle counter for snapshot frequency / ES: Contador de ciclos para frecuencia de snapshot */
	uint32 CycleModCounter = 0;

	// ====================================================================
	// Static
	// ====================================================================

	static TWeakObjectPtr<UPGXGCObserverSubsystem> CachedInstance;

	// ── Profile Integration ──

	void ApplyProfileConstraints(const struct FPGXResolvedProfile& Profile);
	void HandleProfileChanged(const struct FPGXResolvedProfile& OldProfile, const struct FPGXResolvedProfile& NewProfile);
};
