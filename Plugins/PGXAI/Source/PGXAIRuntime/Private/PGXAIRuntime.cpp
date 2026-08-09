// Copyright PGX Framework. All Rights Reserved.

#include "PGXAIRuntime.h"

#include "PGXAIConfig.h"
#include "Observability/PGXObservabilityRegistry.h"
#include "Subsystems/PGXLogSubsystem.h"

PGX_DEFINE_LOG_CATEGORY(LogPGXAI);

#define LOCTEXT_NAMESPACE "FPGXAIRuntimeModule"

void FPGXAIRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXAI, TEXT("PGXAIRuntime: Module started."));

	// EN: Manual fallback registration with the canonical observability registry
	//     (per Sub-Serialization.3.A IPGXObservable contract). BootstrapDiscovery in
	//     PGXCoreRuntime auto-discovers but manual Register makes the dependency
	//     explicit and order-independent at module startup. Mirror PGXEnvironment
	//     8.3.C reference.
	// ES: Registro manual fallback con el registry de observabilidad canonical
	//     (per contrato 8.3.A IPGXObservable). BootstrapDiscovery en
	//     PGXCoreRuntime auto-descubre pero Register manual hace la dependencia
	//     explicita y order-independent en module startup. Mirror referencia
	//     PGXEnvironment 8.3.C.
	FPGXObservabilityRegistry::Register(UPGXAIConfig::StaticClass());
}

void FPGXAIRuntimeModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXAI, TEXT("PGXAIRuntime: Module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXAIRuntimeModule, PGXAIRuntime)
