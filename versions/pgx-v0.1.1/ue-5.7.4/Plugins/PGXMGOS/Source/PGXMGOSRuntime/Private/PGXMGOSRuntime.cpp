// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXMGOSRuntime.h"
#include "Logging/PGXLogMacros.h"
#include "PGXGCObserverConfig.h"
#include "Tags/PGXMGOSTags.h"
#include "Observability/PGXObservabilityRegistry.h"

DEFINE_LOG_CATEGORY(LogPGXMGOS);

#define LOCTEXT_NAMESPACE "FPGXMGOSRuntimeModule"

void FPGXMGOSRuntimeModule::StartupModule()
{
	// EN: PGXMGOS module started. GC Observability System with inference-based monitoring.
	// ES: Modulo PGXMGOS iniciado. Sistema de Observabilidad GC con monitoreo basado en inferencia.
	PGX_LOG_INFO(LogPGXMGOS, TEXT("PGXMGOSRuntime: Module started (GC Observability System)"));

	// EN: manual fallback registration of UPGXGCObserverConfig.
	// ES: registro manual fallback de UPGXGCObserverConfig.
	FPGXObservabilityRegistry::Register(UPGXGCObserverConfig::StaticClass());
}

void FPGXMGOSRuntimeModule::ShutdownModule()
{
	// EN: PGXMGOS module shut down. Cleanup GC observability resources.
	// ES: Modulo PGXMGOS detenido. Limpieza de recursos de observabilidad GC.
	PGX_LOG_INFO(LogPGXMGOS, TEXT("PGXMGOSRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXMGOSRuntimeModule, PGXMGOSRuntime)
