// Copyright PGX Framework. All Rights Reserved.

#include "FPGXHarnessReport.h"

#include "FPGXVisualHarness.h"
#include "HAL/PlatformMemory.h"
#include "Misc/App.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void FPGXHarnessReport::Reset(const FString& InSimulationId)
{
	SimulationId = InSimulationId;
	Samples.Reset();
	PhaseEvents.Reset();
}

void FPGXHarnessReport::AddPhaseEvent(double ElapsedSeconds, const FString& Phase, const FString& Event, bool bSuccess, const FString& Detail)
{
	FPGXHarnessPhaseEvent Entry;
	Entry.ElapsedSeconds = ElapsedSeconds;
	Entry.Phase = Phase;
	Entry.Event = Event;
	Entry.bSuccess = bSuccess;
	Entry.Detail = Detail;
	PhaseEvents.Add(MoveTemp(Entry));
}

void FPGXHarnessReport::CaptureSample(double ElapsedSeconds, const FString& Phase, const FPGXVisualHarness* Harness)
{
	FPGXHarnessMetricSample Sample;
	Sample.ElapsedSeconds = ElapsedSeconds;
	Sample.Phase = Phase;

	const float DeltaTime = FApp::GetDeltaTime();
	Sample.FrameTimeMs = DeltaTime > 0.0f ? DeltaTime * 1000.0f : 0.0f;
	Sample.FPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
	Sample.MemoryMB = static_cast<float>(FPlatformMemory::GetStats().UsedPhysical) / (1024.0f * 1024.0f);

	if (Harness)
	{
		const TArray<FPGXHarnessSystemStatus> Statuses = Harness->GetSystemStatuses();
		Sample.TotalSystems = Statuses.Num();
		for (const FPGXHarnessSystemStatus& Status : Statuses)
		{
			if (Status.bInjected)
			{
				++Sample.SystemsInjected;
			}
			Sample.TotalObjects += Status.ObjectCount;
		}

		const TArray<FPGXHarnessActionEntry>& Actions = Harness->GetActionLog();
		Sample.ActionCount = Actions.Num();
		for (const FPGXHarnessActionEntry& Action : Actions)
		{
			if (!Action.bSuccess)
			{
				++Sample.ErrorCount;
			}
		}
	}

	Samples.Add(MoveTemp(Sample));
}

FString FPGXHarnessReport::ExportJson(const FString& Directory) const
{
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString FilePath = Directory / FString::Printf(TEXT("HarnessSimulation_%s.json"), *SanitizeFileToken(SimulationId));

	FString Json;
	Json += TEXT("{\n");
	Json += FString::Printf(TEXT("  \"simulation_id\": \"%s\",\n"), *EscapeJson(SimulationId));
	Json += FString::Printf(TEXT("  \"generated_at\": \"%s\",\n"), *EscapeJson(FDateTime::Now().ToIso8601()));

	Json += TEXT("  \"phase_events\": [\n");
	for (int32 Index = 0; Index < PhaseEvents.Num(); ++Index)
	{
		const FPGXHarnessPhaseEvent& Event = PhaseEvents[Index];
		Json += FString::Printf(TEXT("    {\"t\": %.3f, \"phase\": \"%s\", \"event\": \"%s\", \"success\": %s, \"detail\": \"%s\"}%s\n"),
			Event.ElapsedSeconds,
			*EscapeJson(Event.Phase),
			*EscapeJson(Event.Event),
			Event.bSuccess ? TEXT("true") : TEXT("false"),
			*EscapeJson(Event.Detail),
			(Index + 1 < PhaseEvents.Num()) ? TEXT(",") : TEXT(""));
	}
	Json += TEXT("  ],\n");

	Json += TEXT("  \"metrics_timeseries\": [\n");
	for (int32 Index = 0; Index < Samples.Num(); ++Index)
	{
		const FPGXHarnessMetricSample& Sample = Samples[Index];
		Json += FString::Printf(TEXT("    {\"t\": %.3f, \"phase\": \"%s\", \"fps\": %.2f, \"frame_time_ms\": %.3f, \"memory_mb\": %.2f, \"systems_injected\": %d, \"total_systems\": %d, \"objects\": %d, \"actions\": %d, \"errors\": %d, \"verification_runs\": %d}%s\n"),
			Sample.ElapsedSeconds,
			*EscapeJson(Sample.Phase),
			Sample.FPS,
			Sample.FrameTimeMs,
			Sample.MemoryMB,
			Sample.SystemsInjected,
			Sample.TotalSystems,
			Sample.TotalObjects,
			Sample.ActionCount,
			Sample.ErrorCount,
			Sample.VerificationRuns,
			(Index + 1 < Samples.Num()) ? TEXT(",") : TEXT(""));
	}
	Json += TEXT("  ]\n");
	Json += TEXT("}\n");

	FFileHelper::SaveStringToFile(Json, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	return FilePath;
}

FString FPGXHarnessReport::ExportMarkdown(const FString& Directory) const
{
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString FilePath = Directory / FString::Printf(TEXT("HarnessSimulation_%s.md"), *SanitizeFileToken(SimulationId));

	FString MD;
	MD += FString::Printf(TEXT("# PGX Harness Simulation Report\n\nGenerated: %s\n\n"), *FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S")));
	MD += FString::Printf(TEXT("Simulation ID: `%s`\n\n"), *SimulationId);
	MD += FString::Printf(TEXT("Samples: %d | Phase events: %d\n\n"), Samples.Num(), PhaseEvents.Num());

	MD += TEXT("## Phase Events\n\n| t | Phase | Event | Result | Detail |\n|---|---|---|---|---|\n");
	for (const FPGXHarnessPhaseEvent& Event : PhaseEvents)
	{
		MD += FString::Printf(TEXT("| %.1f | %s | %s | %s | %s |\n"), Event.ElapsedSeconds, *Event.Phase, *Event.Event, Event.bSuccess ? TEXT("OK") : TEXT("FAIL"), *Event.Detail);
	}

	MD += TEXT("\n## Metrics\n\n| t | Phase | FPS | Memory MB | Systems | Objects | Actions | Errors |\n|---|---|---|---|---|---|---|---|\n");
	for (const FPGXHarnessMetricSample& Sample : Samples)
	{
		MD += FString::Printf(TEXT("| %.1f | %s | %.1f | %.1f | %d/%d | %d | %d | %d |\n"),
			Sample.ElapsedSeconds, *Sample.Phase, Sample.FPS, Sample.MemoryMB, Sample.SystemsInjected, Sample.TotalSystems, Sample.TotalObjects, Sample.ActionCount, Sample.ErrorCount);
	}

	FFileHelper::SaveStringToFile(MD, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	return FilePath;
}

TArray<FString> FPGXHarnessReport::ExportAll(const FString& Directory) const
{
	TArray<FString> Paths;
	Paths.Add(ExportJson(Directory));
	Paths.Add(ExportMarkdown(Directory));
	return Paths;
}

FString FPGXHarnessReport::EscapeJson(const FString& Value)
{
	FString Out = Value;
	Out.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
	Out.ReplaceInline(TEXT("\""), TEXT("\\\""));
	Out.ReplaceInline(TEXT("\r"), TEXT("\\r"));
	Out.ReplaceInline(TEXT("\n"), TEXT("\\n"));
	Out.ReplaceInline(TEXT("\t"), TEXT("\\t"));
	return Out;
}

FString FPGXHarnessReport::SanitizeFileToken(const FString& Value)
{
	FString Out = Value;
	for (TCHAR& C : Out)
	{
		if (!FChar::IsAlnum(C) && C != TEXT('_') && C != TEXT('-'))
		{
			C = TEXT('_');
		}
	}
	return Out;
}
