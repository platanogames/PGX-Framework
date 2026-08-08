// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PGXMGOSTypes.h"
#include "PGXMGOSBlueprintLibrary.generated.h"

/**
 * EN: Blueprint function library for the MGOS (GC Observability System).
 *     Provides static BP-callable functions for GC monitoring, mode control,
 *     baseline management, and incident querying.
 *     NOTE: MGOS is an Engine subsystem — no WorldContext required.
 *
 * ES: Biblioteca de funciones Blueprint para el MGOS (Sistema de Observabilidad GC).
 *     Proporciona funciones estaticas callable desde BP para monitoreo GC,
 *     control de modo, gestion de baseline y consulta de incidentes.
 *     NOTA: MGOS es un subsistema de Engine — no requiere WorldContext.
 */
UCLASS()
class PGXMGOSRUNTIME_API UPGXMGOSBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ====================================================================
	// Control — PGX|MGOS
	// ====================================================================

	/** EN: Set the MGOS observer mode / ES: Establecer el modo del observador MGOS */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS", meta = (DisplayName = "Set MGOS Mode"))
	static void SetMode(EPGXGCObserverMode NewMode);

	/** EN: Request immediate baseline capture / ES: Solicitar captura inmediata del baseline */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS", meta = (DisplayName = "Request Baseline Capture"))
	static void RequestBaselineCapture();

	/** EN: Reset baseline to uninitialized / ES: Resetear baseline a no inicializado */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS", meta = (DisplayName = "Reset Baseline"))
	static void ResetBaseline();

	/** EN: Set suppression mode [R4] / ES: Establecer modo de supresion [R4] */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS", meta = (DisplayName = "Set Suppressed"))
	static void SetSuppressed(bool bSuppress);

	// ====================================================================
	// Query — PGX|MGOS|Query
	// ====================================================================

	/** EN: Get current behavioral profile / ES: Obtener perfil comportamental actual */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query", meta = (DisplayName = "Get GC Profile"))
	static FPGXGCProfile GetCurrentProfile();

	/** EN: Get current observer mode / ES: Obtener modo actual del observador */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query", meta = (DisplayName = "Get MGOS Mode"))
	static EPGXGCObserverMode GetMode();

	/** EN: Get current baseline state / ES: Obtener estado actual del baseline */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query", meta = (DisplayName = "Get Baseline State"))
	static EPGXGCBaselineState GetBaselineState();

	/** EN: Check if MGOS subsystem is initialized / ES: Verificar si el subsistema MGOS esta inicializado */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query", meta = (DisplayName = "Is MGOS Initialized"))
	static bool IsInitialized();

	/** EN: Get total GC cycle count / ES: Obtener conteo total de ciclos GC */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query", meta = (DisplayName = "Get GC Cycle Count"))
	static int64 GetCycleCount();

	/** EN: Check if in suppressed phase [R4] / ES: Verificar si esta en fase de supresion [R4] */
	UFUNCTION(BlueprintPure, Category = "PGX|MGOS|Query", meta = (DisplayName = "Is Suppressed"))
	static bool IsInSuppressedPhase();

	// ====================================================================
	// Incidents — PGX|MGOS|Incidents
	// ====================================================================

	/** EN: Get all active incidents / ES: Obtener todos los incidentes activos */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Incidents", meta = (DisplayName = "Get Incidents"))
	static TArray<FPGXGCIncident> GetCurrentIncidents();

	/** EN: Get tracked class health reports / ES: Obtener reportes de salud de clases rastreadas */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|Incidents", meta = (DisplayName = "Get Class Health Reports"))
	static TArray<FPGXGCClassHealthReport> GetTrackedClassReport();

	// ====================================================================
	// History — PGX|MGOS|History
	// ====================================================================

	/** EN: Get recent GC history summary / ES: Obtener resumen de historial GC reciente */
	UFUNCTION(BlueprintCallable, Category = "PGX|MGOS|History", meta = (DisplayName = "Get History Summary"))
	static TArray<FPGXGCSnapshotDiff> GetHistorySummary(int32 Count = 10);

private:
	/** EN: Internal helper to get subsystem / ES: Helper interno para obtener subsistema */
	static class UPGXGCObserverSubsystem* GetSubsystem();
};
