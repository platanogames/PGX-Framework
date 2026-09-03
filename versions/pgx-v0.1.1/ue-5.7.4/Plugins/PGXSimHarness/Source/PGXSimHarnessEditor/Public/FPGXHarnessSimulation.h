// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "FPGXHarnessReport.h"

class FPGXVisualHarness;
class UWorld;

enum class EPGXHarnessSimulationPhase : uint8
{
	Setup,
	Warmup,
	LevelA_Steady,
	LevelTransition,
	LevelB_Stress,
	Cooldown,
	Teardown,
	Complete
};

PGXSIMHARNESSEDITOR_API const TCHAR* LexToString(EPGXHarnessSimulationPhase Phase);

struct FPGXSimulationConfig
{
	float WarmupDuration = 10.0f;
	float LevelASteadyDuration = 30.0f;
	float LevelTransitionDuration = 5.0f;
	float LevelBStressDuration = 30.0f;
	float CooldownDuration = 10.0f;
	float TeardownDuration = 5.0f;

	int32 MaxSpawnedNPCs = 20;
	int32 MaxSpawnedItems = 50;
	int32 GCStressInterval = 3;

	bool bCaptureMetrics = true;
	bool bExportJSON = true;
	bool bExportMD = true;
	float MetricsInterval = 1.0f;

	float GetTotalDuration() const;
};

struct FPGXHarnessSimulationStatus
{
	bool bRunning = false;
	EPGXHarnessSimulationPhase Phase = EPGXHarnessSimulationPhase::Complete;
	double ElapsedSeconds = 0.0;
	float PhaseElapsedSeconds = 0.0f;
	FString LastReportJson;
	FString LastReportMarkdown;
};

/**
 * EN: Live non-blocking 7-phase simulation controller.
 * ES: Controlador Live no bloqueante de simulacion en 7 fases.
 */
class PGXSIMHARNESSEDITOR_API FPGXHarnessSimulation
{
public:
	FPGXHarnessSimulation();
	~FPGXHarnessSimulation();

	bool Start(UWorld* World, const FPGXSimulationConfig& InConfig = FPGXSimulationConfig());
	void Stop(bool bExportPartialReport = true);
	bool IsRunning() const { return bRunning; }

	FPGXHarnessSimulationStatus GetStatus() const;
	TArray<FString> ExportReport() const;
	FString GetStatusText() const;

private:
	bool Tick(float DeltaTime);
	void EnterPhase(EPGXHarnessSimulationPhase NewPhase);
	EPGXHarnessSimulationPhase ResolvePhase(double ElapsedSeconds) const;
	float GetPhaseStartSeconds(EPGXHarnessSimulationPhase Phase) const;
	void CaptureMetricsIfDue(bool bForce = false);
	void FinishSimulation();

	TWeakObjectPtr<UWorld> CachedWorld;
	TUniquePtr<FPGXVisualHarness> VisualHarness;
	FPGXHarnessReport Report;
	FPGXSimulationConfig Config;
	FTSTicker::FDelegateHandle TickerHandle;

	bool bRunning = false;
	EPGXHarnessSimulationPhase CurrentPhase = EPGXHarnessSimulationPhase::Complete;
	double StartTimeSeconds = 0.0;
	double LastMetricTimeSeconds = 0.0;
	FString LastReportJson;
	FString LastReportMarkdown;
};
