// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXPSORuntimeModule.h"
#include "PGXPSOWarmUpConfig.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Observability/PGXObservabilityRegistry.h"

#define LOCTEXT_NAMESPACE "FPGXPSORuntimeModule"

void FPGXPSORuntimeModule::StartupModule()
{
	// EN: PGXPSORuntime module started. PSO warm-up subsystem will initialize per GameInstance.
	// ES: Modulo PGXPSORuntime iniciado. El subsistema de warm-up PSO se inicializara por GameInstance.
	PGX_LOG_INFO(LogPGXPSO, TEXT("PGXPSORuntime: Module started"));

	// EN: manual fallback registration.
	// ES: registro manual fallback.
	FPGXObservabilityRegistry::Register(UPGXPSOWarmUpConfig::StaticClass());
}

void FPGXPSORuntimeModule::ShutdownModule()
{
	// EN: PGXPSORuntime module shut down.
	// ES: Modulo PGXPSORuntime detenido.
	PGX_LOG_INFO(LogPGXPSO, TEXT("PGXPSORuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXPSORuntimeModule, PGXPSORuntime)
