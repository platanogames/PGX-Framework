// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Manager/PGXSoundPool.h"
#include "PGXAudioLog.h"
#include "Logging/PGXLogMacros.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// EN: Sound instance pool implementation
// ES: Implementacion del pool de instancias de sonido

void UPGXSoundPool::Initialize(int32 InitialSize, int32 InMaxCapacity)
{
	MaxCapacity = InMaxCapacity;
	PeakUsage = 0;
	MissCount = 0;
	GrowthEvents = 0;

	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXSoundPool::Initialize — InitialSize=%d, MaxCapacity=%d"),
		InitialSize, InMaxCapacity);

	// EN: Pre-allocation happens on first AcquireComponent call (needs WorldContext)
	// ES: Pre-allocacion sucede en la primera llamada a AcquireComponent (necesita WorldContext)
}

void UPGXSoundPool::Deinitialize()
{
	// EN: Destroy all components / ES: Destruir todos los componentes
	for (UAudioComponent* Comp : AvailableComponents)
	{
		if (Comp)
		{
			Comp->DestroyComponent();
		}
	}
	AvailableComponents.Empty();

	for (UAudioComponent* Comp : ActiveComponents)
	{
		if (Comp)
		{
			Comp->Stop();
			Comp->DestroyComponent();
		}
	}
	ActiveComponents.Empty();

	PGX_LOG_INFO(LogPGXAudio, TEXT("UPGXSoundPool::Deinitialize — Pool destroyed (peak: %d, misses: %d, growths: %d)"),
		PeakUsage, MissCount, GrowthEvents);
}

UAudioComponent* UPGXSoundPool::AcquireComponent(UObject* WorldContextObject)
{
	// EN: Try to get from available pool / ES: Intentar obtener del pool disponible
	while (AvailableComponents.Num() > 0)
	{
		UAudioComponent* Comp = AvailableComponents.Pop(EAllowShrinking::No);
		if (Comp && IsValid(Comp))
		{
			ActiveComponents.Add(Comp);
			PeakUsage = FMath::Max(PeakUsage, ActiveComponents.Num());
			return Comp;
		}
	}

	// EN: Pool is empty — try to grow / ES: Pool vacio — intentar crecer
	MissCount++;

	const int32 TotalComponents = ActiveComponents.Num() + AvailableComponents.Num();
	if (MaxCapacity > 0 && TotalComponents >= MaxCapacity)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("AcquireComponent — Pool exhausted (max: %d)"), MaxCapacity);
		return nullptr;
	}

	// EN: Grow by batch of 8 / ES: Crecer en lote de 8
	const int32 GrowCount = FMath::Min(8, MaxCapacity > 0 ? MaxCapacity - TotalComponents : 8);
	GrowPool(GrowCount, WorldContextObject);

	// EN: Try again / ES: Intentar de nuevo
	if (AvailableComponents.Num() > 0)
	{
		UAudioComponent* Comp = AvailableComponents.Pop(EAllowShrinking::No);
		if (Comp && IsValid(Comp))
		{
			ActiveComponents.Add(Comp);
			PeakUsage = FMath::Max(PeakUsage, ActiveComponents.Num());
			return Comp;
		}
	}

	return nullptr;
}

void UPGXSoundPool::ReleaseComponent(UAudioComponent* Component)
{
	if (!Component)
	{
		return;
	}

	// EN: Stop and reset / ES: Detener y resetear
	if (Component->IsPlaying())
	{
		Component->Stop();
	}
	Component->SetSound(nullptr);

	ActiveComponents.Remove(Component);
	AvailableComponents.Add(Component);
}

FPGXSoundPoolStats UPGXSoundPool::GetStats() const
{
	FPGXSoundPoolStats Stats;
	Stats.TotalCapacity = AvailableComponents.Num() + ActiveComponents.Num();
	Stats.InUse = ActiveComponents.Num();
	Stats.Available = AvailableComponents.Num();
	Stats.PeakUsage = PeakUsage;
	Stats.MissCount = MissCount;
	Stats.GrowthEvents = GrowthEvents;
	return Stats;
}

FPGXAudioMemoryInfo UPGXSoundPool::GetMemoryInfo() const
{
	FPGXAudioMemoryInfo Info;
	Info.PoolAllocated = AvailableComponents.Num() + ActiveComponents.Num();
	Info.PoolInUse = ActiveComponents.Num();
	Info.PeakConcurrent = PeakUsage;
	// EN: Rough estimate: ~4KB per AudioComponent / ES: Estimacion aproximada: ~4KB por AudioComponent
	Info.EstimatedMemoryBytes = Info.PoolAllocated * 4096;
	return Info;
}

void UPGXSoundPool::GrowPool(int32 Count, UObject* WorldContextObject)
{
	UWorld* World = nullptr;
	if (WorldContextObject)
	{
		World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	}

	if (!World)
	{
		PGX_LOG_WARNING(LogPGXAudio, TEXT("GrowPool — No valid world context for component creation"));
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		// EN: Create a dormant audio component / ES: Crear un componente de audio dormido
		UAudioComponent* NewComp = UGameplayStatics::SpawnSound2D(World, nullptr, 1.0f, 1.0f, 0.0f, nullptr, false, false);
		if (NewComp)
		{
			AvailableComponents.Add(NewComp);
		}
	}

	GrowthEvents++;
	PGX_LOG_INFO(LogPGXAudio, TEXT("GrowPool — Added %d components (total: %d)"),
		Count, AvailableComponents.Num() + ActiveComponents.Num());
}
