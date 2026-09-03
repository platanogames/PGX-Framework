// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXColonyRuntime.h"

#include "Observability/PGXObservabilityRegistry.h"
#include "PGXColonyConfig.h"

void FPGXColonyRuntimeModule::StartupModule()
{
	// EN: PGXCoreRuntime bootstraps observability before most L2 modules are loaded, so
	//     PGXColony uses the documented manual fallback for direct IPGXObservable classes.
	FPGXObservabilityRegistry::Register(UPGXColonyConfig::StaticClass());
}

void FPGXColonyRuntimeModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FPGXColonyRuntimeModule, PGXColonyRuntime)
