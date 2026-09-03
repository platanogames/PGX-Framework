// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCameraRuntime.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXCamera, Log, All);

#define LOCTEXT_NAMESPACE "FPGXCameraRuntimeModule"

void FPGXCameraRuntimeModule::StartupModule()
{
	// EN: PGXCameraRuntime module started. Camera modes and modifiers available.
	// ES: Modulo PGXCameraRuntime iniciado. Modos de camara y modifiers disponibles.
	PGX_LOG_INFO(LogPGXCamera, TEXT("PGXCameraRuntime: Module started"));
}

void FPGXCameraRuntimeModule::ShutdownModule()
{
	// EN: Cleanup camera systems before unloading
	// ES: Limpiar sistemas de camara antes de descargar
	PGX_LOG_INFO(LogPGXCamera, TEXT("PGXCameraRuntime: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXCameraRuntimeModule, PGXCameraRuntime)
