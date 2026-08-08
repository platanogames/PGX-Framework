// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXGameFlowRuntime.h"
#include "PGXGameFlowConfig.h"
#include "PGXFlowRulesConfig.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"
#include "Observability/PGXObservabilityRegistry.h"

#define LOCTEXT_NAMESPACE "FPGXGameFlowRuntimeModule"

void FPGXGameFlowRuntimeModule::StartupModule()
{
	// EN: PGXGameFlowRuntime module started. 8-channel FSM with data-driven validation.
	// ES: Modulo PGXGameFlowRuntime iniciado. FSM de 8 canales con validacion data-driven.
	PGX_LOG_INFO(LogPGX, TEXT("PGXGameFlowRuntime: Module started (8-channel FSM)"));

	// EN: manual fallback registration.
	// ES: registro manual fallback.
	FPGXObservabilityRegistry::Register(UPGXGameFlowConfig::StaticClass());
	FPGXObservabilityRegistry::Register(UPGXFlowRulesConfig::StaticClass());
}

void FPGXGameFlowRuntimeModule::ShutdownModule()
{
	// EN: PGXGameFlowRuntime module shut down. Cleanup game flow resources.
	// ES: Modulo PGXGameFlowRuntime detenido. Limpieza de recursos de flujo de juego.
	PGX_LOG_INFO(LogPGX, TEXT("PGXGameFlowRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXGameFlowRuntimeModule, PGXGameFlowRuntime)
