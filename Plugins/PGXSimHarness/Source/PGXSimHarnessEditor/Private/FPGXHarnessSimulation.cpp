// Copyright PGX Framework. All Rights Reserved.

#include "FPGXHarnessSimulation.h"

#include "FPGXVisualHarness.h"
#include "Logging/PGXLogMacros.h"
#include "PGXSimHarnessEditorModule.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

const TCHAR* LexToString(EPGXHarnessSimulationPhase Phase)
{
	switch (Phase)
	{
	case EPGXHarnessSimulationPhase::Setup: return TEXT("Setup");
	case EPGXHarnessSimulationPhase::Warmup: return TEXT("Warmup");
	case EPGXHarnessSimulationPhase::LevelA_Steady: return TEXT("LevelA_Steady");
	case EPGXHarnessSimulationPhase::LevelTransition: return TEXT("LevelTransition");
	case EPGXHarnessSimulationPhase::LevelB_Stress: return TEXT("LevelB_Stress");
	case EPGXHarnessSimulationPhase::Cooldown: return TEXT("Cooldown");
	case EPGXHarnessSimulationPhase::Teardown: return TEXT("Teardown");
	case EPGXHarnessSimulationPhase::Complete: return TEXT("Complete");
	default: return TEXT("Unknown");
	}
}

float FPGXSimulationConfig::GetTotalDuration() const
{
	return WarmupDuration + LevelASteadyDuration + LevelTransitionDuration + LevelBStressDuration + CooldownDuration + TeardownDuration;
}

FPGXHarnessSimulation::FPGXHarnessSimulation() = default;

FPGXHarnessSimulation::~FPGXHarnessSimulation()
{
	Stop(false);
}

bool FPGXHarnessSimulation::Start(UWorld* World, const FPGXSimulationConfig& InConfig)
{
	if (bRunning)
	{
		return false;
	}
	if (!IsValid(World))
	{
		PGX_LOG_ERROR(LogPGXSimHarness, TEXT("HarnessSimulation — cannot start without a valid world"));
		return false;
	}

	Config = InConfig;
	CachedWorld = World;
	VisualHarness = MakeUnique<FPGXVisualHarness>();
	StartTimeSeconds = FPlatformTime::Seconds();
	LastMetricTimeSeconds = StartTimeSeconds;
	bRunning = true;
	CurrentPhase = EPGXHarnessSimulationPhase::Setup;
	LastReportJson.Empty();
	LastReportMarkdown.Empty();

	const FString SimulationId = FString::Printf(TEXT("PGX_Harness_%s"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	Report.Reset(SimulationId);
	EnterPhase(EPGXHarnessSimulationPhase::Setup);

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateRaw(this, &FPGXHarnessSimulation::Tick),
		FMath::Max(0.1f, Config.MetricsInterval));

	PGX_LOG_INFO(LogPGXSimHarness, TEXT("HarnessSimulation — started %s"), *SimulationId);
	return true;
}

void FPGXHarnessSimulation::Stop(bool bExportPartialReport)
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

	if (bExportPartialReport && Report.GetSamples().Num() > 0)
	{
		const TArray<FString> Paths = ExportReport();
		LastReportJson = Paths.IsValidIndex(0) ? Paths[0] : FString();
		LastReportMarkdown = Paths.IsValidIndex(1) ? Paths[1] : FString();
	}

	if (VisualHarness.IsValid())
	{
		if (VisualHarness->IsSimulating())
		{
			VisualHarness->StopSimulation();
		}
		if (VisualHarness->IsActive())
		{
			VisualHarness->Teardown();
		}
		VisualHarness.Reset();
	}

	bRunning = false;
	CurrentPhase = EPGXHarnessSimulationPhase::Complete;
	CachedWorld.Reset();
}

FPGXHarnessSimulationStatus FPGXHarnessSimulation::GetStatus() const
{
	FPGXHarnessSimulationStatus Status;
	Status.bRunning = bRunning;
	Status.Phase = CurrentPhase;
	Status.ElapsedSeconds = bRunning ? (FPlatformTime::Seconds() - StartTimeSeconds) : 0.0;
	Status.PhaseElapsedSeconds = static_cast<float>(Status.ElapsedSeconds) - GetPhaseStartSeconds(CurrentPhase);
	Status.LastReportJson = LastReportJson;
	Status.LastReportMarkdown = LastReportMarkdown;
	return Status;
}

TArray<FString> FPGXHarnessSimulation::ExportReport() const
{
	return Report.ExportAll(FPaths::ProjectSavedDir() / TEXT("PGX"));
}

FString FPGXHarnessSimulation::GetStatusText() const
{
	const FPGXHarnessSimulationStatus Status = GetStatus();
	return FString::Printf(TEXT("Running=%s Phase=%s Elapsed=%.1fs PhaseElapsed=%.1fs Samples=%d Json=%s Md=%s"),
		Status.bRunning ? TEXT("true") : TEXT("false"),
		LexToString(Status.Phase),
		Status.ElapsedSeconds,
		Status.PhaseElapsedSeconds,
		Report.GetSamples().Num(),
		*Status.LastReportJson,
		*Status.LastReportMarkdown);
}

bool FPGXHarnessSimulation::Tick(float /*DeltaTime*/)
{
	if (!bRunning)
	{
		return false;
	}

	const double Elapsed = FPlatformTime::Seconds() - StartTimeSeconds;
	const EPGXHarnessSimulationPhase NewPhase = ResolvePhase(Elapsed);
	if (NewPhase != CurrentPhase)
	{
		EnterPhase(NewPhase);
	}

	CaptureMetricsIfDue();

	if (NewPhase == EPGXHarnessSimulationPhase::Complete)
	{
		FinishSimulation();
		return false;
	}
	return true;
}

void FPGXHarnessSimulation::EnterPhase(EPGXHarnessSimulationPhase NewPhase)
{
	CurrentPhase = NewPhase;
	const double Elapsed = FPlatformTime::Seconds() - StartTimeSeconds;
	const FString PhaseName = LexToString(NewPhase);
	Report.AddPhaseEvent(Elapsed, PhaseName, TEXT("EnterPhase"), true, TEXT("phase transition"));
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("HarnessSimulation — Enter %s at %.1fs"), *PhaseName, Elapsed);

	if (!VisualHarness.IsValid())
	{
		return;
	}

	switch (NewPhase)
	{
	case EPGXHarnessSimulationPhase::Setup:
		VisualHarness->Setup(CachedWorld.Get());
		break;
	case EPGXHarnessSimulationPhase::Warmup:
		VisualHarness->VerifyAllAPIs();
		VisualHarness->StartSimulation();
		break;
	case EPGXHarnessSimulationPhase::LevelA_Steady:
		VisualHarness->BroadcastTestMessage();
		VisualHarness->CycleSaveSlot();
		VisualHarness->VerifyAllAPIs();
		break;
	case EPGXHarnessSimulationPhase::LevelTransition:
		VisualHarness->CycleGameFlowState();
		VisualHarness->VerifyAllAPIs();
		break;
	case EPGXHarnessSimulationPhase::LevelB_Stress:
		VisualHarness->ForceGarbageCollection();
		VisualHarness->GenerateLogEntries();
		VisualHarness->VerifyAllAPIs();
		break;
	case EPGXHarnessSimulationPhase::Cooldown:
		VisualHarness->StopSimulation();
		VisualHarness->ForceGarbageCollection();
		VisualHarness->VerifyAllAPIs();
		break;
	case EPGXHarnessSimulationPhase::Teardown:
		CaptureMetricsIfDue(true);
		break;
	case EPGXHarnessSimulationPhase::Complete:
		break;
	}
}

EPGXHarnessSimulationPhase FPGXHarnessSimulation::ResolvePhase(double ElapsedSeconds) const
{
	if (ElapsedSeconds < Config.WarmupDuration) return EPGXHarnessSimulationPhase::Warmup;
	if (ElapsedSeconds < Config.WarmupDuration + Config.LevelASteadyDuration) return EPGXHarnessSimulationPhase::LevelA_Steady;
	if (ElapsedSeconds < Config.WarmupDuration + Config.LevelASteadyDuration + Config.LevelTransitionDuration) return EPGXHarnessSimulationPhase::LevelTransition;
	if (ElapsedSeconds < Config.WarmupDuration + Config.LevelASteadyDuration + Config.LevelTransitionDuration + Config.LevelBStressDuration) return EPGXHarnessSimulationPhase::LevelB_Stress;
	if (ElapsedSeconds < Config.WarmupDuration + Config.LevelASteadyDuration + Config.LevelTransitionDuration + Config.LevelBStressDuration + Config.CooldownDuration) return EPGXHarnessSimulationPhase::Cooldown;
	if (ElapsedSeconds < Config.GetTotalDuration()) return EPGXHarnessSimulationPhase::Teardown;
	return EPGXHarnessSimulationPhase::Complete;
}

float FPGXHarnessSimulation::GetPhaseStartSeconds(EPGXHarnessSimulationPhase Phase) const
{
	switch (Phase)
	{
	case EPGXHarnessSimulationPhase::Setup:
	case EPGXHarnessSimulationPhase::Warmup: return 0.0f;
	case EPGXHarnessSimulationPhase::LevelA_Steady: return Config.WarmupDuration;
	case EPGXHarnessSimulationPhase::LevelTransition: return Config.WarmupDuration + Config.LevelASteadyDuration;
	case EPGXHarnessSimulationPhase::LevelB_Stress: return Config.WarmupDuration + Config.LevelASteadyDuration + Config.LevelTransitionDuration;
	case EPGXHarnessSimulationPhase::Cooldown: return Config.WarmupDuration + Config.LevelASteadyDuration + Config.LevelTransitionDuration + Config.LevelBStressDuration;
	case EPGXHarnessSimulationPhase::Teardown: return Config.WarmupDuration + Config.LevelASteadyDuration + Config.LevelTransitionDuration + Config.LevelBStressDuration + Config.CooldownDuration;
	case EPGXHarnessSimulationPhase::Complete: return Config.GetTotalDuration();
	default: return 0.0f;
	}
}

void FPGXHarnessSimulation::CaptureMetricsIfDue(bool bForce)
{
	if (!Config.bCaptureMetrics && !bForce)
	{
		return;
	}
	const double Now = FPlatformTime::Seconds();
	if (!bForce && (Now - LastMetricTimeSeconds) < Config.MetricsInterval)
	{
		return;
	}
	LastMetricTimeSeconds = Now;
	Report.CaptureSample(Now - StartTimeSeconds, LexToString(CurrentPhase), VisualHarness.Get());
}

void FPGXHarnessSimulation::FinishSimulation()
{
	CaptureMetricsIfDue(true);
	const TArray<FString> Paths = ExportReport();
	LastReportJson = Paths.IsValidIndex(0) ? Paths[0] : FString();
	LastReportMarkdown = Paths.IsValidIndex(1) ? Paths[1] : FString();
	Report.AddPhaseEvent(FPlatformTime::Seconds() - StartTimeSeconds, LexToString(CurrentPhase), TEXT("ExportReport"), true, LastReportJson + TEXT(" | ") + LastReportMarkdown);

	if (VisualHarness.IsValid())
	{
		if (VisualHarness->IsSimulating())
		{
			VisualHarness->StopSimulation();
		}
		if (VisualHarness->IsActive())
		{
			VisualHarness->Teardown();
		}
	}

	bRunning = false;
	CurrentPhase = EPGXHarnessSimulationPhase::Complete;
	PGX_LOG_INFO(LogPGXSimHarness, TEXT("HarnessSimulation — complete. %s"), *GetStatusText());
}
