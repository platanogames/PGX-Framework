// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSaveRuntime.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "PGXSaveConfig.h"

#define LOCTEXT_NAMESPACE "FPGXSaveRuntimeModule"

void FPGXSaveRuntimeModule::StartupModule()
{
	// EN: PGXSaveRuntime module started. Initializes save/load systems.
	// ES: Modulo PGXSaveRuntime iniciado. Inicializa sistemas de guardado/carga.
	PGX_LOG_INFO(LogPGXSave, TEXT("PGXSaveRuntime: Module started"));

	// Register the authoring configuration with the shared observability registry.
	FPGXObservabilityRegistry::Register(UPGXSaveConfig::StaticClass());
}

void FPGXSaveRuntimeModule::ShutdownModule()
{
	// EN: PGXSaveRuntime module shut down. Cleanup save system resources.
	// ES: Modulo PGXSaveRuntime detenido. Limpieza de recursos del sistema de guardado.
	PGX_LOG_INFO(LogPGXSave, TEXT("PGXSaveRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXSaveRuntimeModule, PGXSaveRuntime)
