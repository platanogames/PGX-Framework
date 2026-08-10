// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FPGXVisualHarness;

struct FPGXHarnessMetricSample
{
	double ElapsedSeconds = 0.0;
	FString Phase;
	float FPS = 0.0f;
	float FrameTimeMs = 0.0f;
	float MemoryMB = 0.0f;
	int32 SystemsInjected = 0;
	int32 TotalSystems = 0;
	int32 TotalObjects = 0;
	int32 ActionCount = 0;
	int32 ErrorCount = 0;
	int32 VerificationRuns = 0;
};

struct FPGXHarnessPhaseEvent
{
	double ElapsedSeconds = 0.0;
	FString Phase;
	FString Event;
	bool bSuccess = true;
	FString Detail;
};

/**
 * EN: Live report engine for the live SimHarness simulation.
 * ES: Motor de reportes Live para la simulacion viva de SimHarness.
 */
class PGXSIMHARNESSEDITOR_API FPGXHarnessReport
{
public:
	void Reset(const FString& InSimulationId);
	void AddPhaseEvent(double ElapsedSeconds, const FString& Phase, const FString& Event, bool bSuccess, const FString& Detail);
	void CaptureSample(double ElapsedSeconds, const FString& Phase, const FPGXVisualHarness* Harness);

	FString ExportJson(const FString& Directory) const;
	FString ExportMarkdown(const FString& Directory) const;
	TArray<FString> ExportAll(const FString& Directory) const;

	const TArray<FPGXHarnessMetricSample>& GetSamples() const { return Samples; }
	const TArray<FPGXHarnessPhaseEvent>& GetPhaseEvents() const { return PhaseEvents; }
	const FString& GetSimulationId() const { return SimulationId; }

private:
	static FString EscapeJson(const FString& Value);
	static FString SanitizeFileToken(const FString& Value);

	FString SimulationId;
	TArray<FPGXHarnessMetricSample> Samples;
	TArray<FPGXHarnessPhaseEvent> PhaseEvents;
};
