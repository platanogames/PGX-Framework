// Copyright PGX Framework. All Rights Reserved.

#include "PGXEnvironmentRuntime.h"
#include "Logging/PGXLogMacros.h"

#include "Observability/PGXObservabilityRegistry.h"
#include "PGXEnvironmentConfig.h"
#include "PGXEnvironmentTickProfile.h"
#include "PGXEnvironmentVariable.h"
#include "PGXEnvironmentZoneDefinition.h"

DEFINE_LOG_CATEGORY(LogPGXEnvironment);

#define LOCTEXT_NAMESPACE "FPGXEnvironmentRuntimeModule"

void FPGXEnvironmentRuntimeModule::StartupModule()
{
	PGX_LOG_INFO(LogPGXEnvironment, TEXT("PGXEnvironmentRuntime: Module started."));

	// EN: PGXCoreRuntime bootstraps observability before most L2 modules are
	//     loaded, so PGXEnvironment uses the documented manual fallback for
	//     direct IPGXObservable implementers. Idempotent under hot reload.
	// ES: PGXCoreRuntime inicializa observabilidad antes de que la mayoria de
	//     modulos L2 carguen, asi que PGXEnvironment usa el fallback manual.
	FPGXObservabilityRegistry::Register(UPGXEnvironmentConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXEnvironmentVariable::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXEnvironmentZoneDefinition::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXEnvironmentTickProfile::StaticClass());
}

void FPGXEnvironmentRuntimeModule::ShutdownModule()
{
	PGX_LOG_INFO(LogPGXEnvironment, TEXT("PGXEnvironmentRuntime: Module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXEnvironmentRuntimeModule, PGXEnvironmentRuntime)
