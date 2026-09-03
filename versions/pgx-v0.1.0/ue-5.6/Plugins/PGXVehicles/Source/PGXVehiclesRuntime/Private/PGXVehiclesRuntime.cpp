// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXVehiclesRuntime.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "PGXVehiclesTypes.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXVehicles, Log, All);

#define LOCTEXT_NAMESPACE "FPGXVehiclesRuntimeModule"

void FPGXVehiclesRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXVehicles, TEXT("PGXVehiclesRuntime: Module started."));

	// EN: Manual observability registration for the plugin-owned DataAsset class.
	// ES: Registro manual de observabilidad para el DataAsset owned por el plugin.
	FPGXObservabilityRegistry::Register(UPGXVehicleDefinitionAsset::StaticClass());
}

void FPGXVehiclesRuntimeModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXVehicles, TEXT("PGXVehiclesRuntime: Module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXVehiclesRuntimeModule, PGXVehiclesRuntime)