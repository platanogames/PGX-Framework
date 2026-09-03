// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "FPGXScenarioFixtures.h"

#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"

#include "Logging/PGXLogMacros.h"
#include "PGXSimHarnessEditorModule.h"   // LogPGXSimHarness

// PGXLoadingRuntime — LevelFlow subsystem (GameInstanceSubsystem, NOT WorldSubsystem;
// resolve via GameInstance.)
#include "PGXLevelFlowSubsystem.h"
#include "PGXLevelFlowTypes.h"

namespace
{
	// EN: Scenario layout constants (cm). Loose grid so the actors are distinct.
	// ES: Constantes de layout del escenario (cm).
	constexpr float kGridStep = 250.0f;
}

// ============================================================================
// Marker spawn / destroy helpers
// ============================================================================

AActor* FPGXScenarioFixtures::SpawnMarker(UWorld* World, const FVector& Location,
	const TCHAR* Label, TArray<TWeakObjectPtr<AActor>>& OutList)
{
	if (!IsValid(World))
	{
		return nullptr;
	}

	// EN: Same lightweight pattern the VisualHarness uses for smoke actors — a bare
	//     AActor with AlwaysSpawn so editor collision never blocks the spawn.
	// ES: Mismo patrón lightweight del VisualHarness — AActor simple con AlwaysSpawn.
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(),
		FTransform(FRotator::ZeroRotator, Location), Params);
	if (!Actor)
	{
		PGX_LOG_WARNING(LogPGXSimHarness,
			TEXT("ScenarioFixtures — failed to spawn marker '%s' at %s"),
			Label, *Location.ToString());
		return nullptr;
	}

#if WITH_EDITOR
	Actor->SetActorLabel(FString::Printf(TEXT("PGXFixture_%s"), Label));
#endif

	OutList.Add(Actor);
	return Actor;
}

void FPGXScenarioFixtures::DestroyTracked(TArray<TWeakObjectPtr<AActor>>& List)
{
	for (TWeakObjectPtr<AActor>& WeakActor : List)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Actor->Destroy();
		}
	}
	List.Empty();
}

int32 FPGXScenarioFixtures::CountValid(const TArray<TWeakObjectPtr<AActor>>& List)
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AActor>& WeakActor : List)
	{
		if (WeakActor.IsValid())
		{
			++Count;
		}
	}
	return Count;
}

// ============================================================================
// Level A — light scenario
// ============================================================================

int32 FPGXScenarioFixtures::SpawnLevelA(UWorld* World)
{
	if (!IsValid(World))
	{
		PGX_LOG_ERROR(LogPGXSimHarness, TEXT("ScenarioFixtures::SpawnLevelA — no valid world"));
		return 0;
	}

	// EN: Spawn is idempotent-safe: tear down any prior Level A first.
	// ES: Idempotente: teardown de un Level A previo antes de re-spawn.
	TeardownLevelA();

	// 1 ground marker at origin.
	SpawnMarker(World, FVector::ZeroVector, TEXT("A_Ground"), LevelAActors);

	// 4 spawn points for waves (a small ring around origin).
	for (int32 i = 0; i < 4; ++i)
	{
		const FVector Loc(FMath::Cos(i * PI / 2.0f) * kGridStep * 3.0f,
			FMath::Sin(i * PI / 2.0f) * kGridStep * 3.0f, 0.0f);
		SpawnMarker(World, Loc, *FString::Printf(TEXT("A_SpawnPoint%d"), i), LevelAActors);
	}

	// 4 camera rail points (a line offset above the scene).
	for (int32 i = 0; i < 4; ++i)
	{
		const FVector Loc(i * kGridStep * 2.0f, -kGridStep * 4.0f, kGridStep);
		SpawnMarker(World, Loc, *FString::Printf(TEXT("A_CameraRail%d"), i), LevelAActors);
	}

	// 6 NPC waypoints (a patrol path).
	for (int32 i = 0; i < 6; ++i)
	{
		const FVector Loc(i * kGridStep, kGridStep * 2.0f, 0.0f);
		SpawnMarker(World, Loc, *FString::Printf(TEXT("A_Waypoint%d"), i), LevelAActors);
	}

	const int32 Spawned = LevelAActors.Num();
	PGX_LOG_INFO(LogPGXSimHarness,
		TEXT("ScenarioFixtures::SpawnLevelA — %d actors (ground + 4 spawn + 4 camera + 6 waypoint)"),
		Spawned);
	return Spawned;
}

void FPGXScenarioFixtures::TeardownLevelA()
{
	const int32 Before = LevelAActors.Num();
	DestroyTracked(LevelAActors);
	if (Before > 0)
	{
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("ScenarioFixtures::TeardownLevelA — destroyed %d actors"), Before);
	}
}

// ============================================================================
// Level B — stress scenario (A->B transition)
// ============================================================================

int32 FPGXScenarioFixtures::SpawnLevelB(UWorld* World)
{
	if (!IsValid(World))
	{
		PGX_LOG_ERROR(LogPGXSimHarness, TEXT("ScenarioFixtures::SpawnLevelB — no valid world"));
		return 0;
	}

	// EN: The A->B transition = tear down Level A, verify the LevelFlow bridge responds,
	//     then spawn the denser Level B. No real .umap OpenLevel (see header §).
	// ES: Transición A->B = teardown A + smoke-check LevelFlow + spawn Level B denso.
	TeardownLevelA();
	SmokeCheckLevelFlow(World);

	// Idempotent-safe on Level B too.
	TeardownLevelB();

	// 12 denser geometry markers (a 4x3 grid).
	for (int32 x = 0; x < 4; ++x)
	{
		for (int32 y = 0; y < 3; ++y)
		{
			const FVector Loc(x * kGridStep, y * kGridStep, 0.0f);
			SpawnMarker(World, Loc, *FString::Printf(TEXT("B_Geo_%d_%d"), x, y), LevelBActors);
		}
	}

	// 4 simulated streaming-zone markers (corners — markers only, no real streaming).
	const FVector Corners[4] = {
		FVector(-kGridStep * 4.0f, -kGridStep * 4.0f, 0.0f),
		FVector(kGridStep * 4.0f, -kGridStep * 4.0f, 0.0f),
		FVector(-kGridStep * 4.0f, kGridStep * 4.0f, 0.0f),
		FVector(kGridStep * 4.0f, kGridStep * 4.0f, 0.0f)
	};
	for (int32 i = 0; i < 4; ++i)
	{
		SpawnMarker(World, Corners[i], *FString::Printf(TEXT("B_StreamZone%d"), i), LevelBActors);
	}

	const int32 Spawned = LevelBActors.Num();
	PGX_LOG_INFO(LogPGXSimHarness,
		TEXT("ScenarioFixtures::SpawnLevelB — %d actors (12 geo + 4 stream-zone), LevelFlow responded=%s"),
		Spawned, bLevelFlowResponded ? TEXT("true") : TEXT("false"));
	return Spawned;
}

void FPGXScenarioFixtures::TeardownLevelB()
{
	const int32 Before = LevelBActors.Num();
	DestroyTracked(LevelBActors);
	if (Before > 0)
	{
		PGX_LOG_INFO(LogPGXSimHarness, TEXT("ScenarioFixtures::TeardownLevelB — destroyed %d actors"), Before);
	}
}

// ============================================================================
// Full teardown + LevelFlow smoke check
// ============================================================================

void FPGXScenarioFixtures::TeardownAll()
{
	TeardownLevelA();
	TeardownLevelB();
}

void FPGXScenarioFixtures::SmokeCheckLevelFlow(UWorld* World)
{
	bLevelFlowResponded = false;

	// EN: LevelFlow is a GameInstanceSubsystem — resolve it via the GameInstance, not
	//     World->GetSubsystem (which is for WorldSubsystems). In an editor world without
	//     PIE the GameInstance may be null; that is a degraded-but-valid state, not a crash.
	// ES: LevelFlow es GameInstanceSubsystem — se resuelve por el GameInstance.
	UGameInstance* GI = IsValid(World) ? World->GetGameInstance() : nullptr;
	if (!GI)
	{
		PGX_LOG_WARNING(LogPGXSimHarness,
			TEXT("ScenarioFixtures::SmokeCheckLevelFlow — no GameInstance (editor world without PIE?); skipping bridge check"));
		return;
	}

	UPGXLevelFlowSubsystem* LevelFlow = GI->GetSubsystem<UPGXLevelFlowSubsystem>();
	if (!LevelFlow)
	{
		PGX_LOG_WARNING(LogPGXSimHarness,
			TEXT("ScenarioFixtures::SmokeCheckLevelFlow — UPGXLevelFlowSubsystem not available"));
		return;
	}

	// EN: Query-only smoke check — the subsystem answering these const queries without
	//     crashing is the "bridge responds" signal the Live plan asks for.
	// ES: Smoke check solo de queries — que el subsistema responda es la señal pedida.
	const bool bInitialized = LevelFlow->IsInitialized();
	const EPGXLevelFlowState State = LevelFlow->GetTransitionState();
	const int32 RegisteredLevels = LevelFlow->GetRegisteredLevelCount();

	bLevelFlowResponded = true;
	PGX_LOG_INFO(LogPGXSimHarness,
		TEXT("ScenarioFixtures::SmokeCheckLevelFlow — bridge responded: initialized=%s state=%d registeredLevels=%d"),
		bInitialized ? TEXT("true") : TEXT("false"), static_cast<int32>(State), RegisteredLevels);
}

// ============================================================================
// Query
// ============================================================================

int32 FPGXScenarioFixtures::GetLevelAActorCount() const
{
	return CountValid(LevelAActors);
}

int32 FPGXScenarioFixtures::GetLevelBActorCount() const
{
	return CountValid(LevelBActors);
}
