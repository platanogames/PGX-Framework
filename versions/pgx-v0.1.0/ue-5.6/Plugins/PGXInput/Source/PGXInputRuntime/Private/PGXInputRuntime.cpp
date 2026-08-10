// Copyright PGX Framework. All Rights Reserved.

#include "PGXInputRuntime.h"

#include "PGXInputConfig.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Observability/PGXObservabilityRegistry.h"

#define LOCTEXT_NAMESPACE "FPGXInputRuntimeModule"

void FPGXInputRuntimeModule::StartupModule()
{
	// EN: PGXInputRuntime module started. Initializes input abstraction systems.
	// ES: Modulo PGXInputRuntime iniciado. Inicializa sistemas de abstraccion de input.
	PGX_LOG_INFO(LogPGX, TEXT("PGXInputRuntime: Module started"));

	// EN: Observability support — manual fallback registration of UPGXInputConfig
	//     observable DA class with FPGXObservabilityRegistry. Mirror
	//     PGXEnvironment 8.3.B / PGXAI / PGXUI 8.3.C reference.
	// ES: Observability support — registro manual fallback de la clase DA observable
	//     UPGXInputConfig con FPGXObservabilityRegistry. Mirror referencia
	//     PGXEnvironment 8.3.B / PGXAI / PGXUI 8.3.C.
	FPGXObservabilityRegistry::Register(UPGXInputConfig::StaticClass());
}

void FPGXInputRuntimeModule::ShutdownModule()
{
	// EN: PGXInputRuntime module shut down. Cleanup input system resources.
	// ES: Modulo PGXInputRuntime detenido. Limpieza de recursos del sistema de input.
	PGX_LOG_INFO(LogPGX, TEXT("PGXInputRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXInputRuntimeModule, PGXInputRuntime)
