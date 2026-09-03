// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXInventoryRuntime.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "PGXItemDefinition.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"

#define LOCTEXT_NAMESPACE "FPGXInventoryRuntimeModule"

void FPGXInventoryRuntimeModule::StartupModule()
{
	// EN: PGXInventoryRuntime module started. Initializes item and inventory systems.
	// ES: Modulo PGXInventoryRuntime iniciado. Inicializa sistemas de items e inventario.
	PGX_LOG_INFO(LogPGX, TEXT("PGXInventoryRuntime: Module started"));

	// EN: Manual observability registration for the plugin-owned ItemDefinition DataAsset.
	// ES: Registro manual de observabilidad para el DataAsset ItemDefinition owned por el plugin.
	FPGXObservabilityRegistry::Register(UPGXItemDefinition::StaticClass());
}

void FPGXInventoryRuntimeModule::ShutdownModule()
{
	// EN: PGXInventoryRuntime module shut down. Cleanup inventory resources.
	// ES: Modulo PGXInventoryRuntime detenido. Limpieza de recursos de inventario.
	PGX_LOG_INFO(LogPGX, TEXT("PGXInventoryRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXInventoryRuntimeModule, PGXInventoryRuntime)
