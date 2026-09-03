// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXInteractionRuntime.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"

#define LOCTEXT_NAMESPACE "FPGXInteractionRuntimeModule"

void FPGXInteractionRuntimeModule::StartupModule()
{
	// EN: PGXInteractionRuntime module started. Initializes interaction detection systems.
	// ES: Modulo PGXInteractionRuntime iniciado. Inicializa sistemas de deteccion de interaccion.
	PGX_LOG_INFO(LogPGX, TEXT("PGXInteractionRuntime: Module started"));
}

void FPGXInteractionRuntimeModule::ShutdownModule()
{
	// EN: PGXInteractionRuntime module shut down. Cleanup interaction resources.
	// ES: Modulo PGXInteractionRuntime detenido. Limpieza de recursos de interaccion.
	PGX_LOG_INFO(LogPGX, TEXT("PGXInteractionRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXInteractionRuntimeModule, PGXInteractionRuntime)
