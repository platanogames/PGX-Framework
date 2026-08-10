// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXLevelFlowActor.h"
#include "Logging/PGXLogMacros.h"
#include "PGXLevelFlowSubsystem.h"
#include "PGXLoadingRuntime.h"

APGXLevelFlowActor::APGXLevelFlowActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APGXLevelFlowActor::BeginPlay()
{
	Super::BeginPlay();

	// EN: Register with LevelFlow subsystem
	// ES: Registrar con subsistema LevelFlow
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UPGXLevelFlowSubsystem* Sub = GI->GetSubsystem<UPGXLevelFlowSubsystem>())
		{
			Sub->RegisterLevelFlowActor(this);
		}
	}

	PGX_LOG_INFO(LogPGXLoading, TEXT("[LevelFlowActor] BeginPlay: %s (LevelTag: %s)"),
		*GetName(),
		LevelTag.IsValid() ? *LevelTag.ToString() : TEXT("(none)"));
}

void APGXLevelFlowActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// EN: Unregister from LevelFlow subsystem
	// ES: Des-registrar del subsistema LevelFlow
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (UPGXLevelFlowSubsystem* Sub = GI->GetSubsystem<UPGXLevelFlowSubsystem>())
		{
			Sub->UnregisterLevelFlowActor(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}
