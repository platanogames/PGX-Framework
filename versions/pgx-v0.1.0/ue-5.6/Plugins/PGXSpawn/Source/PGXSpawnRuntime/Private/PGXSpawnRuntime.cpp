// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSpawnRuntime.h"

#include "Observability/PGXObservabilityRegistry.h"
#include "PGXSpawnConfig.h"
#include "PGXWaveDefinition.h"
#include "Logging/PGXLogMacros.h"
#include "Subsystems/PGXLogSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXSpawn, Log, All);

#define LOCTEXT_NAMESPACE "FPGXSpawnRuntimeModule"

void FPGXSpawnRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXSpawn, TEXT("PGXSpawnRuntime: Module started."));

	FPGXObservabilityRegistry::Register(UPGXSpawnConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXWaveDefinition::StaticClass());
}

void FPGXSpawnRuntimeModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXSpawn, TEXT("PGXSpawnRuntime: Module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXSpawnRuntimeModule, PGXSpawnRuntime)
