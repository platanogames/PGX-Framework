// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXGCObserverSubsystem.h"
#include "Logging/PGXLogMacros.h"
#include "PGXGCObserverConfig.h"
#include "PGXMGOSSettings.h"
#include "Utils/PGXConfigResolution.h"
#include "PGXMGOSRuntime.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectArray.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "HAL/PlatformMemory.h"
#include "HAL/IConsoleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Containers/Ticker.h"

// EN: Static instance cache / ES: Cache estatico de instancia
TWeakObjectPtr<UPGXGCObserverSubsystem> UPGXGCObserverSubsystem::CachedInstance = nullptr;

// ============================================================================
// Lifecycle
// ============================================================================

void UPGXGCObserverSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CachedInstance = this;

	// EN: Discover config DA via AssetRegistry / ES: Descubrir config DA via AssetRegistry
	DiscoverConfig();

	// EN: Init history store / ES: Inicializar store de historial
	const int32 WindowSize = ActiveConfig ? ActiveConfig->HistoryWindowSize : 64;
	HistoryStore.Init(WindowSize);

	// EN: Register GC delegates / ES: Registrar delegados GC
	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddUObject(this, &UPGXGCObserverSubsystem::OnPreGarbageCollect);
	FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &UPGXGCObserverSubsystem::OnPostGarbageCollect);

	// EN: Register engine travel delegates / ES: Registrar delegados de viaje del engine
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UPGXGCObserverSubsystem::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UPGXGCObserverSubsystem::OnPostLoadMapWithWorld);
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &UPGXGCObserverSubsystem::OnWorldCleanup);

	// EN: Set initial mode from config / ES: Establecer modo inicial desde config
	CurrentMode = ActiveConfig ? ActiveConfig->DefaultMode : EPGXGCObserverMode::Passive;

	// EN: Resolve tracked/excluded classes from config / ES: Resolver clases rastreadas/excluidas desde config
	if (ActiveConfig)
	{
		for (const TSoftClassPtr<UObject>& SoftClass : ActiveConfig->TrackedClasses)
		{
			if (UClass* LoadedClass = SoftClass.LoadSynchronous())
			{
				ResolvedTrackedClasses.Add(LoadedClass);
			}
		}
		for (const TSoftClassPtr<UObject>& SoftClass : ActiveConfig->ExcludedClasses)
		{
			if (UClass* LoadedClass = SoftClass.LoadSynchronous())
			{
				ResolvedExcludedClasses.Add(LoadedClass->GetFName());
			}
		}
	}

	// EN: Register console commands / ES: Registrar comandos de consola
	RegisterConsoleCommands();

	// EN: Start inter-cycle ticker if enabled [R5] / ES: Iniciar ticker inter-ciclo si esta habilitado [R5]
	if (ActiveConfig && ActiveConfig->bEnableInterCycleMonitoring)
	{
		LastInterCycleUObjectCount = GUObjectArray.GetObjectArrayNum();
		InterCycleTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UPGXGCObserverSubsystem::OnInterCycleCheck),
			ActiveConfig->InterCycleCheckInterval
		);
	}

	// ── Profile Integration ──
	if (auto* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
	{
		if (ProfileSS->IsProfileResolved())
		{
			ApplyProfileConstraints(ProfileSS->GetResolvedProfile());
		}
		ProfileSS->OnProfileChangedNative.AddUObject(this, &ThisClass::HandleProfileChanged);
	}

	bIsInitialized = true;

	PGX_LOG_INFO(LogPGXMGOS, TEXT("UPGXGCObserverSubsystem initialized. Mode: %s, Config: %s"),
		*UEnum::GetValueAsString(CurrentMode),
		ActiveConfig ? *ActiveConfig->GetName() : TEXT("None (defaults)"));
}

void UPGXGCObserverSubsystem::Deinitialize()
{
	bIsInitialized = false;

	// EN: Clear inter-cycle ticker [R5] / ES: Limpiar ticker inter-ciclo [R5]
	if (InterCycleTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(InterCycleTickerHandle);
		InterCycleTickerHandle.Reset();
	}

	// EN: Unregister GC delegates / ES: Desregistrar delegados GC
	FCoreUObjectDelegates::GetPreGarbageCollectDelegate().RemoveAll(this);
	FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);

	// EN: Unregister engine travel delegates / ES: Desregistrar delegados de viaje
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);

	// EN: Unregister console commands / ES: Desregistrar comandos de consola
	UnregisterConsoleCommands();

	// ── Profile Unbind ──
	if (UPGXProfileSubsystem* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
	{
		ProfileSS->OnProfileChangedNative.RemoveAll(this);
	}

	CachedInstance = nullptr;

	PGX_LOG_INFO(LogPGXMGOS, TEXT("UPGXGCObserverSubsystem deinitialized. Total cycles observed: %lld"), NextCycleID - 1);

	Super::Deinitialize();
}

// ============================================================================
// Static Access
// ============================================================================

UPGXGCObserverSubsystem* UPGXGCObserverSubsystem::GetCachedInstance()
{
	if (CachedInstance.IsValid())
	{
		return CachedInstance.Get();
	}
	if (GEngine)
	{
		UPGXGCObserverSubsystem* Sub = GEngine->GetEngineSubsystem<UPGXGCObserverSubsystem>();
		CachedInstance = Sub;
		return Sub;
	}
	return nullptr;
}

// ============================================================================
// GC Hooks
// ============================================================================

void UPGXGCObserverSubsystem::OnPreGarbageCollect()
{
	if (CurrentMode == EPGXGCObserverMode::Off) return;

	CycleStartTime = FPlatformTime::Seconds();
	PreSnapshot = CaptureSnapshot(EPGXGCPhase::PreGC);

	// EN: Broadcast GC started / ES: Notificar GC iniciado
	const int64 CurrentCycleID = static_cast<int64>(NextCycleID);
	OnGCStarted.Broadcast(CurrentCycleID);
	OnGCStartedNative.Broadcast(NextCycleID);
}

void UPGXGCObserverSubsystem::OnPostGarbageCollect()
{
	if (CurrentMode == EPGXGCObserverMode::Off) return;

	FPGXGCSnapshot PostSnapshot = CaptureSnapshot(EPGXGCPhase::PostGC);
	const double Duration = FPlatformTime::Seconds() - CycleStartTime;

	FPGXGCSnapshotDiff Diff = BuildDiff(PreSnapshot, PostSnapshot, Duration);

	HistoryStore.Push(PostSnapshot, Diff);

	// EN: Baseline auto-capture after warmup / ES: Auto-captura de baseline despues de warmup
	if (CurrentBaseline.BaselineState == EPGXGCBaselineState::Uninitialized ||
		CurrentBaseline.BaselineState == EPGXGCBaselineState::Warmup)
	{
		const int32 Warmup = ActiveConfig ? ActiveConfig->WarmupCycles : 16;
		if (HistoryStore.Count >= Warmup)
		{
			TryCaptureBaseline(PostSnapshot);
		}
		else if (CurrentBaseline.BaselineState == EPGXGCBaselineState::Uninitialized)
		{
			CurrentBaseline.BaselineState = EPGXGCBaselineState::Warmup;
		}
	}

	// EN: Profile evaluation (skip if suppressed [R4]) / ES: Evaluacion de perfil (omitir si esta suprimido [R4])
	if (!bIsSuppressed && CurrentBaseline.BaselineState == EPGXGCBaselineState::Valid)
	{
		EvaluateProfile(PostSnapshot, Diff);
		CheckNonUObjectLeak(PostSnapshot);
	}

	// EN: Broadcast completion / ES: Notificar completado
	const int64 CurrentCycleID = static_cast<int64>(NextCycleID);
	OnGCCompleted.Broadcast(CurrentCycleID, Diff);
	OnGCCompletedNative.Broadcast(NextCycleID, PostSnapshot, Diff);

	NextCycleID++;
	CycleModCounter++;

	// EN: Reset inter-cycle counter [R5] / ES: Resetear contador inter-ciclo [R5]
	LastInterCycleUObjectCount = static_cast<uint32>(PostSnapshot.TotalUObjectCount);
}

// ============================================================================
// Engine Hooks
// ============================================================================

void UPGXGCObserverSubsystem::OnPreLoadMap(const FString& MapName)
{
	// EN: Invalidate baseline on map change / ES: Invalidar baseline en cambio de mapa
	if (CurrentBaseline.BaselineState == EPGXGCBaselineState::Valid)
	{
		InvalidateBaseline();
		PGX_LOG_INFO(LogPGXMGOS, TEXT("Baseline invalidated due to map load: %s"), *MapName);
	}
}

void UPGXGCObserverSubsystem::OnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
	// EN: Log new world loaded / ES: Registrar nuevo mundo cargado
	if (LoadedWorld)
	{
		PGX_LOG_VERBOSE(LogPGXMGOS, TEXT("New world loaded: %s — baseline will recapture after warmup"), *LoadedWorld->GetName());
	}
}

void UPGXGCObserverSubsystem::OnWorldCleanup(UWorld* /*World*/, bool bSessionEnded, bool /*bCleanupResources*/)
{
	if (bSessionEnded && CurrentBaseline.BaselineState == EPGXGCBaselineState::Valid)
	{
		InvalidateBaseline();
		PGX_LOG_INFO(LogPGXMGOS, TEXT("Baseline invalidated due to session end"));
	}
}

// ============================================================================
// Inter-Cycle Monitoring [R5]
// ============================================================================

bool UPGXGCObserverSubsystem::OnInterCycleCheck(float /*DeltaTime*/)
{
	if (CurrentMode == EPGXGCObserverMode::Off || !bIsInitialized) return true;

	const uint32 CurrentCount = GUObjectArray.GetObjectArrayNum();
	const int64 Delta = static_cast<int64>(CurrentCount) - static_cast<int64>(LastInterCycleUObjectCount);
	const float Threshold = ActiveConfig ? ActiveConfig->CriticalGrowthThreshold : 10000.0f;

	if (static_cast<float>(Delta) > Threshold)
	{
		RaiseIncident(EPGXGCSeverity::Info, FName("GCPressureWarning"),
			FString::Printf(TEXT("UObject growth of %lld without GC (threshold: %.0f)"), Delta, Threshold));
	}

	LastInterCycleUObjectCount = CurrentCount;
	return true; // EN: Continue ticking / ES: Continuar ticking
}

// ============================================================================
// Snapshot Builder
// ============================================================================

FPGXGCSnapshot UPGXGCObserverSubsystem::CaptureSnapshot(EPGXGCPhase Phase)
{
	FPGXGCSnapshot Snapshot;
	Snapshot.CycleID = static_cast<int64>(NextCycleID);
	Snapshot.Timestamp = FPlatformTime::Seconds();
	Snapshot.Phase = Phase;
	Snapshot.ExecMode = DetermineExecutionMode();

#if UE_BUILD_DEBUG
	Snapshot.BuildConfig = FName("Debug");
#elif UE_BUILD_DEVELOPMENT
	Snapshot.BuildConfig = FName("Development");
#elif UE_BUILD_SHIPPING
	Snapshot.BuildConfig = FName("Shipping");
#else
	Snapshot.BuildConfig = FName("Unknown");
#endif

	// EN: O(1) global count / ES: Conteo global O(1)
	Snapshot.TotalUObjectCount = GUObjectArray.GetObjectArrayNum();

	// EN: Process memory [R1] / ES: Memoria del proceso [R1]
	Snapshot.ProcessMemoryMB = CaptureProcessMemory();

	if (CurrentMode == EPGXGCObserverMode::Passive)
	{
		// EN: Passive mode — no iteration / ES: Modo pasivo — sin iteracion
		return Snapshot;
	}

	// EN: Snapshot mode — targeted iteration for TrackedClasses [R3]
	// ES: Modo Snapshot — iteracion dirigida para TrackedClasses [R3]
	CaptureTrackedClassCounts(Snapshot);

	const int32 Frequency = ActiveConfig ? ActiveConfig->SnapshotFrequency : 1;
	const bool bFullCapture = (Frequency <= 1) || (CycleModCounter % static_cast<uint32>(Frequency) == 0);

	if (bFullCapture)
	{
		// EN: Count Actor and Component totals / ES: Contar totales de Actor y Component
		int64 ActorCount = 0;
		int64 ComponentCount = 0;
		int64 PendingKillCount = 0;

		if (CurrentMode == EPGXGCObserverMode::DeepTrack)
		{
			// EN: DeepTrack — full iteration for Top-K + counts
			// ES: DeepTrack — iteracion completa para Top-K + conteos
			TMap<FName, int32> ClassCountMap;
			const int32 K = ActiveConfig ? ActiveConfig->TopKClasses : 10;

			for (FThreadSafeObjectIterator It; It; ++It)
			{
				UObject* Obj = *It;
				if (!IsValid(Obj)) continue;

				if (!IsValid(Obj) || Obj->IsUnreachable())
				{
					PendingKillCount++;
					continue;
				}

				if (Obj->IsA<AActor>()) ActorCount++;
				if (Obj->IsA<UActorComponent>()) ComponentCount++;

				FName ClassName = Obj->GetClass()->GetFName();
				ClassCountMap.FindOrAdd(ClassName)++;
			}

			// EN: Sort by count descending, take top K / ES: Ordenar por conteo descendente, tomar top K
			ClassCountMap.ValueSort([](int32 A, int32 B) { return A > B; });

			int32 Added = 0;
			for (const auto& Pair : ClassCountMap)
			{
				if (Added >= K) break;
				FPGXGCClassCount Entry;
				Entry.ClassName = Pair.Key;
				Entry.InstanceCount = Pair.Value;
				Snapshot.TopClasses.Add(Entry);
				Added++;
			}
		}
		else
		{
			// EN: Snapshot mode — count Actor/Component via targeted iterators
			// ES: Modo Snapshot — contar Actor/Component via iteradores dirigidos
			for (TObjectIterator<AActor> It; It; ++It)
			{
				ActorCount++;
			}
			for (TObjectIterator<UActorComponent> It; It; ++It)
			{
				ComponentCount++;
			}
		}

		Snapshot.TotalActorCount = ActorCount;
		Snapshot.TotalComponentCount = ComponentCount;
		Snapshot.PendingKillCount = PendingKillCount;
	}

	return Snapshot;
}

void UPGXGCObserverSubsystem::CaptureTrackedClassCounts(FPGXGCSnapshot& OutSnapshot)
{
	// EN: Single pass over all UObjects, accumulate counts per tracked class [R3]
	// ES: Una sola pasada por todos los UObjects, acumular conteos por clase rastreada [R3]
	TMap<UClass*, int32> CountMap;
	CountMap.Reserve(ResolvedTrackedClasses.Num());
	for (UClass* TrackedClass : ResolvedTrackedClasses)
	{
		if (TrackedClass)
		{
			CountMap.Add(TrackedClass, 0);
		}
	}

	for (TObjectIterator<UObject> It; It; ++It)
	{
		for (const auto& Pair : CountMap)
		{
			if (It->IsA(Pair.Key))
			{
				CountMap[Pair.Key]++;
			}
		}
	}

	// EN: Build output entries / ES: Construir entradas de salida
	OutSnapshot.TopClasses.Reserve(CountMap.Num());
	for (const auto& Pair : CountMap)
	{
		FPGXGCClassCount Entry;
		Entry.ClassName = Pair.Key->GetFName();
		Entry.InstanceCount = Pair.Value;
		OutSnapshot.TopClasses.Add(Entry);
	}
}

float UPGXGCObserverSubsystem::CaptureProcessMemory() const
{
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}

// ============================================================================
// Diff Builder
// ============================================================================

FPGXGCSnapshotDiff UPGXGCObserverSubsystem::BuildDiff(const FPGXGCSnapshot& Pre, const FPGXGCSnapshot& Post, double Duration)
{
	FPGXGCSnapshotDiff Diff;
	Diff.CycleID = Post.CycleID;
	Diff.DurationSeconds = Duration;
	Diff.DeltaTotalUObject = Post.TotalUObjectCount - Pre.TotalUObjectCount;
	Diff.DeltaActorCount = Post.TotalActorCount - Pre.TotalActorCount;
	Diff.DeltaComponentCount = Post.TotalComponentCount - Pre.TotalComponentCount;
	Diff.DeltaPendingKill = Post.PendingKillCount - Pre.PendingKillCount;
	Diff.DeltaRootSet = Post.RootSetEstimate - Pre.RootSetEstimate;
	Diff.ProcessMemoryDeltaMB = Post.ProcessMemoryMB - Pre.ProcessMemoryMB;

	// EN: Build per-class deltas from TopClasses / ES: Construir deltas por clase desde TopClasses
	TMap<FName, int64> PreMap;
	for (const FPGXGCClassCount& Entry : Pre.TopClasses)
	{
		PreMap.Add(Entry.ClassName, Entry.InstanceCount);
	}
	for (const FPGXGCClassCount& Entry : Post.TopClasses)
	{
		const int64 PreCount = PreMap.Contains(Entry.ClassName) ? PreMap[Entry.ClassName] : 0;
		const int32 Delta = static_cast<int32>(static_cast<int64>(Entry.InstanceCount) - PreCount);
		if (Delta != 0)
		{
			FPGXGCClassDelta ClassDelta;
			ClassDelta.ClassName = Entry.ClassName;
			ClassDelta.DeltaCount = Delta;
			Diff.ClassDeltas.Add(ClassDelta);
		}
	}

	return Diff;
}

// ============================================================================
// Profile Engine
// ============================================================================

void UPGXGCObserverSubsystem::EvaluateProfile(const FPGXGCSnapshot& PostSnapshot, const FPGXGCSnapshotDiff& Diff)
{
	EPGXGCProfileState NewState = ClassifyState(PostSnapshot, Diff);

	if (NewState != CurrentProfile.CurrentState)
	{
		EPGXGCProfileState OldState = CurrentProfile.CurrentState;
		CurrentProfile.CurrentState = NewState;
		CurrentProfile.CyclesInState = 1;
		CurrentProfile.Confidence = 0.5f;

		// EN: Broadcast state change / ES: Notificar cambio de estado
		OnProfileStateChanged.Broadcast(NewState, OldState);
		OnProfileStateChangedNative.Broadcast(NewState, OldState);

		PGX_LOG_INFO(LogPGXMGOS, TEXT("Profile state changed: %s -> %s"),
			*UEnum::GetValueAsString(OldState), *UEnum::GetValueAsString(NewState));
	}
	else
	{
		CurrentProfile.CyclesInState++;
		// EN: Confidence increases with confirmation cycles / ES: Confianza aumenta con ciclos de confirmacion
		const int32 ConfirmCycles = ActiveConfig ? ActiveConfig->ConfirmCycles : 8;
		CurrentProfile.Confidence = FMath::Min(1.0f, static_cast<float>(CurrentProfile.CyclesInState) / static_cast<float>(ConfirmCycles));
	}

	// EN: Generate incidents for confirmed anomalous states / ES: Generar incidentes para estados anomalos confirmados
	if (ActiveConfig && ActiveConfig->bEnableIncidents)
	{
		const int32 ConfirmCycles = ActiveConfig->ConfirmCycles;
		if (CurrentProfile.CyclesInState == ConfirmCycles)
		{
			switch (CurrentProfile.CurrentState)
			{
			case EPGXGCProfileState::Accumulation:
				RaiseIncident(EPGXGCSeverity::Warning, FName("Accumulation"),
					FString::Printf(TEXT("UObject accumulation confirmed after %d cycles (drift: %lld)"),
						ConfirmCycles, PostSnapshot.TotalUObjectCount - CurrentBaseline.TotalUObjectCount));
				break;
			case EPGXGCProfileState::LeakSuspected:
				RaiseIncident(EPGXGCSeverity::Critical, FName("LeakSuspected"),
					FString::Printf(TEXT("Memory leak suspected — persistent growth confirmed after %d cycles"), ConfirmCycles));
				break;
			case EPGXGCProfileState::PendingKillSaturation:
				RaiseIncident(EPGXGCSeverity::Warning, FName("PendingKillSaturation"),
					FString::Printf(TEXT("PendingKill saturation detected — ratio: %.3f"),
						PostSnapshot.TotalUObjectCount > 0
							? static_cast<float>(PostSnapshot.PendingKillCount) / static_cast<float>(PostSnapshot.TotalUObjectCount)
							: 0.0f));
				break;
			case EPGXGCProfileState::RootExpansion:
				RaiseIncident(EPGXGCSeverity::Warning, FName("RootExpansion"),
					TEXT("Root set expanding without matching destruction"));
				break;
			default:
				break;
			}
		}
	}

	// EN: Evaluate per-class health if applicable / ES: Evaluar salud por clase si aplica
	if (CurrentMode == EPGXGCObserverMode::DeepTrack || CurrentMode == EPGXGCObserverMode::Snapshot)
	{
		EvaluateClassHealth();
	}
}

EPGXGCProfileState UPGXGCObserverSubsystem::ClassifyState(const FPGXGCSnapshot& Snapshot, const FPGXGCSnapshotDiff& Diff)
{
	if (!CurrentBaseline.bValid) return EPGXGCProfileState::Stable;

	// EN: Check for BurstClean first (high priority) / ES: Verificar BurstClean primero (alta prioridad)
	if (DetectBurstClean(Diff))
	{
		return EPGXGCProfileState::BurstClean;
	}

	// EN: Check PendingKill saturation / ES: Verificar saturacion PendingKill
	if (DetectPKSaturation(Snapshot))
	{
		return EPGXGCProfileState::PendingKillSaturation;
	}

	// EN: Check root expansion / ES: Verificar expansion del root set
	if (DetectRootExpansion(Diff))
	{
		return EPGXGCProfileState::RootExpansion;
	}

	// EN: Check for leak vs accumulation / ES: Verificar leak vs acumulacion
	const float ThresholdLeak = ActiveConfig ? ActiveConfig->ThresholdLeak : 0.01f;
	const float ThresholdAccumulation = ActiveConfig ? ActiveConfig->ThresholdAccumulation : 0.005f;
	const float EpsilonAbs = ActiveConfig ? ActiveConfig->EpsilonAbsolute : 5.0f;

	const int64 Drift = Snapshot.TotalUObjectCount - CurrentBaseline.TotalUObjectCount;
	const float DriftRatio = CurrentBaseline.TotalUObjectCount > 0
		? static_cast<float>(Drift) / static_cast<float>(CurrentBaseline.TotalUObjectCount)
		: 0.0f;

	if (static_cast<float>(FMath::Abs(Drift)) > EpsilonAbs)
	{
		if (DriftRatio > ThresholdLeak)
		{
			return EPGXGCProfileState::LeakSuspected;
		}
		if (DriftRatio > ThresholdAccumulation)
		{
			return EPGXGCProfileState::Accumulation;
		}
	}

	return EPGXGCProfileState::Stable;
}

bool UPGXGCObserverSubsystem::DetectBurstClean(const FPGXGCSnapshotDiff& Diff) const
{
	// EN: Large negative delta in single cycle = burst destruction
	// ES: Delta negativo grande en un solo ciclo = destruccion por rafaga
	const float ZScoreThreshold = ActiveConfig ? ActiveConfig->ThresholdZScoreDestruction : 2.5f;

	if (HistoryStore.Count < 2) return false;
	if (HistoryStore.MovingAvgDelta >= 0.0) return false; // EN: Average is not negative

	// EN: Simple check — large destruction relative to history
	// ES: Verificacion simple — destruccion grande relativa al historial
	return Diff.DeltaTotalUObject < 0 &&
		static_cast<double>(FMath::Abs(Diff.DeltaTotalUObject)) > FMath::Abs(HistoryStore.MovingAvgDelta) * ZScoreThreshold;
}

bool UPGXGCObserverSubsystem::DetectPKSaturation(const FPGXGCSnapshot& Snapshot) const
{
	const float Threshold = ActiveConfig ? ActiveConfig->ThresholdPKRatio : 0.1f;
	if (Snapshot.TotalUObjectCount == 0) return false;

	const float Ratio = static_cast<float>(Snapshot.PendingKillCount) / static_cast<float>(Snapshot.TotalUObjectCount);
	return Ratio > Threshold;
}

bool UPGXGCObserverSubsystem::DetectRootExpansion(const FPGXGCSnapshotDiff& Diff) const
{
	// EN: Root set growing without proportional destruction
	// ES: Root set creciendo sin destruccion proporcional
	return Diff.DeltaRootSet > 0 && Diff.DeltaTotalUObject > 0;
}

void UPGXGCObserverSubsystem::CheckNonUObjectLeak(const FPGXGCSnapshot& Snapshot)
{
	if (!ActiveConfig || !ActiveConfig->bEnableProcessMemoryTracking) return;
	if (!CurrentBaseline.bValid) return;

	const float MemDrift = Snapshot.ProcessMemoryMB - CurrentBaseline.BaselineProcessMemoryMB;
	const int64 UObjDrift = Snapshot.TotalUObjectCount - CurrentBaseline.TotalUObjectCount;
	const float DriftThreshold = ActiveConfig->ProcessMemoryDriftThresholdMB;
	const float EpsilonAbs = ActiveConfig->EpsilonAbsolute;

	// EN: Memory growing while UObjects are stable → Non-UObject leak
	// ES: Memoria creciendo mientras UObjects estables → Leak no-UObject
	if (MemDrift > DriftThreshold && static_cast<float>(FMath::Abs(UObjDrift)) < EpsilonAbs * 10.0f)
	{
		RaiseIncident(EPGXGCSeverity::Warning, FName("NonUObjectLeakSuspected"),
			FString::Printf(TEXT("Process memory grew %.1f MB with stable UObject count (drift: %lld) — possible non-UObject leak"),
				MemDrift, UObjDrift));
	}
}

void UPGXGCObserverSubsystem::EvaluateClassHealth()
{
	if (!CurrentBaseline.bValid) return;

	TrackedClassReports.Reset();

	for (UClass* TrackedClass : ResolvedTrackedClasses)
	{
		if (!TrackedClass) continue;

		const FName ClassName = TrackedClass->GetFName();
		if (IsClassExcluded(ClassName)) continue;

		FPGXGCClassHealthReport Report;
		Report.ClassName = ClassName;

		// EN: Get current count from latest snapshot / ES: Obtener conteo actual del ultimo snapshot
		const FPGXGCSnapshot* Latest = HistoryStore.GetLatestSnapshot();
		if (Latest)
		{
			for (const FPGXGCClassCount& Entry : Latest->TopClasses)
			{
				if (Entry.ClassName == ClassName)
				{
					Report.CurrentCount = Entry.InstanceCount;
					break;
				}
			}
		}

		// EN: Get baseline count / ES: Obtener conteo baseline
		if (const int32* BaseCount = CurrentBaseline.ClassCounts.Find(ClassName))
		{
			Report.BaselineCount = *BaseCount;
		}

		// EN: Compute ratios / ES: Calcular ratios
		if (Report.BaselineCount > 0)
		{
			Report.OverBaselineRatio = static_cast<float>(Report.CurrentCount) / static_cast<float>(Report.BaselineCount);
		}

		// EN: Compute growth slope from recent history / ES: Calcular pendiente de crecimiento del historial reciente
		TArray<FPGXGCSnapshotDiff> RecentDiffs = HistoryStore.GetRecentDiffs(FMath::Min(8, HistoryStore.Count));
		float SumSlope = 0.0f;
		int32 SlopeCount = 0;
		for (const FPGXGCSnapshotDiff& D : RecentDiffs)
		{
			for (const FPGXGCClassDelta& CD : D.ClassDeltas)
			{
				if (CD.ClassName == ClassName)
				{
					SumSlope += static_cast<float>(CD.DeltaCount);
					SlopeCount++;
					break;
				}
			}
		}
		if (SlopeCount > 0)
		{
			Report.GrowthSlope = SumSlope / static_cast<float>(SlopeCount);
		}

		// EN: Classify health / ES: Clasificar salud
		const float AccThreshold = ActiveConfig ? ActiveConfig->ThresholdAccumulation : 0.005f;
		const float LeakThreshold = ActiveConfig ? ActiveConfig->ThresholdLeak : 0.01f;

		if (Report.BaselineCount > 0)
		{
			const float RelGrowth = Report.GrowthSlope / static_cast<float>(Report.BaselineCount);
			if (RelGrowth > LeakThreshold)
			{
				Report.HealthState = EPGXGCClassHealth::LeakSuspected;
			}
			else if (Report.OverBaselineRatio > 1.0f + AccThreshold * 10.0f)
			{
				Report.HealthState = EPGXGCClassHealth::PersistentOverBaseline;
			}
			else if (RelGrowth > AccThreshold)
			{
				Report.HealthState = EPGXGCClassHealth::Accumulating;
			}
		}

		TrackedClassReports.Add(Report);
	}
}

// ============================================================================
// Baseline Management
// ============================================================================

void UPGXGCObserverSubsystem::TryCaptureBaseline(const FPGXGCSnapshot& Snapshot)
{
	CurrentBaseline.bValid = true;
	CurrentBaseline.BaselineCycleID = Snapshot.CycleID;
	CurrentBaseline.Timestamp = Snapshot.Timestamp;
	CurrentBaseline.TotalUObjectCount = Snapshot.TotalUObjectCount;
	CurrentBaseline.PendingKillCount = Snapshot.PendingKillCount;
	CurrentBaseline.BaselineProcessMemoryMB = Snapshot.ProcessMemoryMB;
	CurrentBaseline.BaselineState = EPGXGCBaselineState::Valid;

	// EN: Store per-class baseline counts / ES: Almacenar conteos baseline por clase
	CurrentBaseline.ClassCounts.Reset();
	for (const FPGXGCClassCount& Entry : Snapshot.TopClasses)
	{
		CurrentBaseline.ClassCounts.Add(Entry.ClassName, Entry.InstanceCount);
	}

	PGX_LOG_INFO(LogPGXMGOS, TEXT("Baseline captured at cycle %lld — UObjects: %lld, Memory: %.1f MB"),
		Snapshot.CycleID, Snapshot.TotalUObjectCount, Snapshot.ProcessMemoryMB);
}

void UPGXGCObserverSubsystem::ValidateBaseline()
{
	CurrentBaseline.BaselineState = EPGXGCBaselineState::Valid;
}

void UPGXGCObserverSubsystem::InvalidateBaseline()
{
	CurrentBaseline.BaselineState = EPGXGCBaselineState::Stale;
	CurrentBaseline.bValid = false;

	// EN: Reset profile to Stable on baseline invalidation / ES: Resetear perfil a Stable al invalidar baseline
	CurrentProfile.CurrentState = EPGXGCProfileState::Stable;
	CurrentProfile.CyclesInState = 0;
	CurrentProfile.Confidence = 0.0f;
}

// ============================================================================
// Incident Management
// ============================================================================

void UPGXGCObserverSubsystem::RaiseIncident(EPGXGCSeverity Severity, FName Category, const FString& Description)
{
	FPGXGCIncident Incident;
	Incident.CycleID = static_cast<int64>(NextCycleID);
	Incident.Severity = Severity;
	Incident.Category = Category;
	Incident.Description = Description;

	ActiveIncidents.Add(Incident);

	// EN: FIFO trim if MaxIncidentHistory is set / ES: Trim FIFO si MaxIncidentHistory esta configurado
	if (ActiveConfig && ActiveConfig->MaxIncidentHistory > 0)
	{
		while (ActiveIncidents.Num() > ActiveConfig->MaxIncidentHistory)
		{
			ActiveIncidents.RemoveAt(0);
		}
	}

	// EN: Broadcast incident / ES: Notificar incidente
	OnIncidentRaised.Broadcast(Incident);
	OnIncidentRaisedNative.Broadcast(Incident);

	// EN: Log based on severity / ES: Registrar basado en severidad
	switch (Severity)
	{
	case EPGXGCSeverity::Critical:
		PGX_LOG_ERROR(LogPGXMGOS, TEXT("[INCIDENT:%s] %s"), *Category.ToString(), *Description);
		break;
	case EPGXGCSeverity::Warning:
		PGX_LOG_WARNING(LogPGXMGOS, TEXT("[INCIDENT:%s] %s"), *Category.ToString(), *Description);
		break;
	default:
		PGX_LOG_INFO(LogPGXMGOS, TEXT("[INCIDENT:%s] %s"), *Category.ToString(), *Description);
		break;
	}
}

// ============================================================================
// Public API — Control
// ============================================================================

void UPGXGCObserverSubsystem::SetMode(EPGXGCObserverMode NewMode)
{
	// EN: Prevent DeepTrack in Shipping unless allowed / ES: Prevenir DeepTrack en Shipping salvo que este permitido
	if (NewMode == EPGXGCObserverMode::DeepTrack && DetermineExecutionMode() == EPGXExecutionMode::Shipping)
	{
		if (!ActiveConfig || !ActiveConfig->bAllowDeepTrackInShipping)
		{
			PGX_LOG_WARNING(LogPGXMGOS, TEXT("DeepTrack mode not allowed in Shipping builds"));
			return;
		}
	}

	EPGXGCObserverMode OldMode = CurrentMode;
	CurrentMode = NewMode;

	PGX_LOG_INFO(LogPGXMGOS, TEXT("Mode changed: %s -> %s"),
		*UEnum::GetValueAsString(OldMode), *UEnum::GetValueAsString(NewMode));
}

void UPGXGCObserverSubsystem::RequestBaselineCapture()
{
	const FPGXGCSnapshot* Latest = HistoryStore.GetLatestSnapshot();
	if (Latest)
	{
		TryCaptureBaseline(*Latest);
	}
	else
	{
		PGX_LOG_WARNING(LogPGXMGOS, TEXT("Cannot capture baseline — no snapshots in history"));
	}
}

void UPGXGCObserverSubsystem::ResetBaseline()
{
	CurrentBaseline = FPGXGCBaseline();
	CurrentProfile = FPGXGCProfile();
	ActiveIncidents.Reset();

	PGX_LOG_INFO(LogPGXMGOS, TEXT("Baseline reset to Uninitialized"));
}

void UPGXGCObserverSubsystem::SetSuppressed(bool bSuppress)
{
	bIsSuppressed = bSuppress;
	PGX_LOG_INFO(LogPGXMGOS, TEXT("Suppression %s"), bSuppress ? TEXT("enabled") : TEXT("disabled"));
}

// ============================================================================
// Public API — Query
// ============================================================================

TArray<FPGXGCClassHealthReport> UPGXGCObserverSubsystem::GetTrackedClassReport() const
{
	return TrackedClassReports;
}

TArray<FPGXGCSnapshotDiff> UPGXGCObserverSubsystem::GetHistorySummary(int32 Count) const
{
	return HistoryStore.GetRecentDiffs(Count);
}

// ============================================================================
// Utility
// ============================================================================

EPGXExecutionMode UPGXGCObserverSubsystem::DetermineExecutionMode() const
{
#if UE_BUILD_SHIPPING
	return EPGXExecutionMode::Shipping;
#else
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				return EPGXExecutionMode::PIE;
			}
		}
	}
	return EPGXExecutionMode::Standalone;
#endif
}

bool UPGXGCObserverSubsystem::IsClassExcluded(FName ClassName) const
{
	return ResolvedExcludedClasses.Contains(ClassName);
}

void UPGXGCObserverSubsystem::DiscoverConfig()
{
	// EN: Settings-first resolution with AssetRegistry fallback (deprecated)
	// ES: Resolucion Settings-first con fallback a AssetRegistry (deprecated)
	const UPGXMGOSSettings* Settings = GetDefault<UPGXMGOSSettings>();
	ActiveConfig = PGX::ResolveSingleConfig<UPGXGCObserverConfig>(Settings->ActiveConfig, TEXT("MGOS"));

	if (IsValid(ActiveConfig))
	{
		PGX_LOG_INFO(LogPGXMGOS, TEXT("Config DA discovered: %s"), *ActiveConfig->GetName());
	}
	else
	{
		PGX_LOG_INFO(LogPGXMGOS, TEXT("No MGOS Config DA found — using defaults"));
	}
}

void UPGXGCObserverSubsystem::RegisterConsoleCommands()
{
	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.mgos.status"),
		TEXT("Print MGOS status (mode, baseline, cycles, memory)"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			const FPGXGCSnapshot* Latest = HistoryStore.GetLatestSnapshot();
			PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Status ==="));
			PGX_LOG_INFO(LogPGXMGOS, TEXT("Mode: %s"), *UEnum::GetValueAsString(CurrentMode));
			PGX_LOG_INFO(LogPGXMGOS, TEXT("Baseline: %s"), *UEnum::GetValueAsString(CurrentBaseline.BaselineState));
			PGX_LOG_INFO(LogPGXMGOS, TEXT("Profile: %s (Confidence: %.2f, Cycles: %d)"),
				*UEnum::GetValueAsString(CurrentProfile.CurrentState), CurrentProfile.Confidence, CurrentProfile.CyclesInState);
			PGX_LOG_INFO(LogPGXMGOS, TEXT("Cycles: %lld"), NextCycleID - 1);
			PGX_LOG_INFO(LogPGXMGOS, TEXT("Incidents: %d"), ActiveIncidents.Num());
			PGX_LOG_INFO(LogPGXMGOS, TEXT("Suppressed: %s"), bIsSuppressed ? TEXT("Yes") : TEXT("No"));
			if (Latest)
			{
				PGX_LOG_INFO(LogPGXMGOS, TEXT("Last UObjects: %lld, Memory: %.1f MB"), Latest->TotalUObjectCount, Latest->ProcessMemoryMB);
			}
		}),
		ECVF_Default));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.mgos.mode"),
		TEXT("Set MGOS mode (Off, Passive, Snapshot, DeepTrack)"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				PGX_LOG_INFO(LogPGXMGOS, TEXT("Current mode: %s"), *UEnum::GetValueAsString(CurrentMode));
				return;
			}
			const FString& ModeStr = Args[0];
			if (ModeStr == TEXT("Off")) SetMode(EPGXGCObserverMode::Off);
			else if (ModeStr == TEXT("Passive")) SetMode(EPGXGCObserverMode::Passive);
			else if (ModeStr == TEXT("Snapshot")) SetMode(EPGXGCObserverMode::Snapshot);
			else if (ModeStr == TEXT("DeepTrack")) SetMode(EPGXGCObserverMode::DeepTrack);
			else PGX_LOG_WARNING(LogPGXMGOS, TEXT("Unknown mode: %s (use Off, Passive, Snapshot, DeepTrack)"), *ModeStr);
		}),
		ECVF_Default));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.mgos.baseline"),
		TEXT("Print current baseline info"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Baseline ==="));
			PGX_LOG_INFO(LogPGXMGOS, TEXT("State: %s"), *UEnum::GetValueAsString(CurrentBaseline.BaselineState));
			if (CurrentBaseline.bValid)
			{
				PGX_LOG_INFO(LogPGXMGOS, TEXT("CycleID: %lld, UObjects: %lld, PK: %lld, Memory: %.1f MB"),
					CurrentBaseline.BaselineCycleID, CurrentBaseline.TotalUObjectCount,
					CurrentBaseline.PendingKillCount, CurrentBaseline.BaselineProcessMemoryMB);
				PGX_LOG_INFO(LogPGXMGOS, TEXT("Tracked classes: %d"), CurrentBaseline.ClassCounts.Num());
			}
		}),
		ECVF_Default));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.mgos.baseline.capture"),
		TEXT("Force baseline capture from latest snapshot"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			RequestBaselineCapture();
		}),
		ECVF_Default));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.mgos.baseline.reset"),
		TEXT("Reset baseline to uninitialized"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			ResetBaseline();
		}),
		ECVF_Default));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.mgos.incidents"),
		TEXT("Print all active incidents"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Incidents (%d) ==="), ActiveIncidents.Num());
			for (const FPGXGCIncident& Inc : ActiveIncidents)
			{
				PGX_LOG_INFO(LogPGXMGOS, TEXT("[Cycle %lld] %s (%s): %s"),
					Inc.CycleID, *Inc.Category.ToString(),
					*UEnum::GetValueAsString(Inc.Severity), *Inc.Description);
			}
		}),
		ECVF_Default));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.mgos.history"),
		TEXT("Print recent GC cycle history"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			TArray<FPGXGCSnapshotDiff> Recent = HistoryStore.GetRecentDiffs(10);
			PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS History (last %d cycles) ==="), Recent.Num());
			for (const FPGXGCSnapshotDiff& D : Recent)
			{
				PGX_LOG_INFO(LogPGXMGOS, TEXT("Cycle %lld: Duration=%.3fms Delta=%lld PK=%lld Mem=%.1fMB"),
					D.CycleID, D.DurationSeconds * 1000.0, D.DeltaTotalUObject, D.DeltaPendingKill, D.ProcessMemoryDeltaMB);
			}
			PGX_LOG_INFO(LogPGXMGOS, TEXT("Avg Duration: %.3fms, Avg Delta: %.1f, Avg PK: %.1f"),
				HistoryStore.MovingAvgDuration * 1000.0, HistoryStore.MovingAvgDelta, HistoryStore.MovingAvgPendingKill);
		}),
		ECVF_Default));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.mgos.export"),
		TEXT("Export MGOS data to log (for external analysis)"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			PGX_LOG_INFO(LogPGXMGOS, TEXT("=== MGOS Export ==="));
			PGX_LOG_INFO(LogPGXMGOS, TEXT("CycleID,Duration,DeltaUObj,DeltaPK,MemDeltaMB"));
			TArray<FPGXGCSnapshotDiff> All = HistoryStore.GetRecentDiffs(HistoryStore.Count);
			for (const FPGXGCSnapshotDiff& D : All)
			{
				PGX_LOG_INFO(LogPGXMGOS, TEXT("%lld,%.6f,%lld,%lld,%.2f"),
					D.CycleID, D.DurationSeconds, D.DeltaTotalUObject, D.DeltaPendingKill, D.ProcessMemoryDeltaMB);
			}
		}),
		ECVF_Default));
}

void UPGXGCObserverSubsystem::UnregisterConsoleCommands()
{
	for (IConsoleCommand* Cmd : RegisteredCommands)
	{
		IConsoleManager::Get().UnregisterConsoleObject(static_cast<IConsoleObject*>(Cmd));
	}
	RegisteredCommands.Reset();
}

// ============================================================================
// Profile Integration
// ============================================================================

void UPGXGCObserverSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& /*Profile*/)
{
	// EN: Capture and report the active profile budgets. This baseline does not
	//     resize history stores or evict tracked classes; runtime enforcement remains
	//     intentionally deferred until eviction and truncation policies are defined.
	// ES: Captura y reporta los presupuestos del perfil activo. Este baseline no
	//     redimensiona historiales ni expulsa clases; el enforcement queda diferido
	//     hasta definir las politicas de eviction y truncado.
	int32 CapturedHistoryWindow = 0;
	int32 CapturedMaxTrackedClasses = 0;
	int32 CapturedMaxIncidentHistory = 0;

	if (auto* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
	{
		if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
		{
			const auto& B = PlatformCfg->MGOSBudgets;
			CapturedHistoryWindow = B.HistoryWindowSize;
			CapturedMaxTrackedClasses = B.MaxTrackedClasses;
			CapturedMaxIncidentHistory = B.MaxIncidentHistory;
		}
	}

	PGX_LOG_INFO(LogPGXMGOS, TEXT("[GCObserverSubsystem] Profile constraints captured (runtime enforcement deferred — no resize of HistoryStore / TrackedClasses / IncidentHistory at this baseline) — HistoryWindow=%d, MaxTracked=%d, MaxIncidents=%d"),
		CapturedHistoryWindow, CapturedMaxTrackedClasses, CapturedMaxIncidentHistory);
}

void UPGXGCObserverSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}
