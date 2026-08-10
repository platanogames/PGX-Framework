// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCoreDeveloper.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXCoreDeveloper, Log, All);

#define LOCTEXT_NAMESPACE "FPGXCoreDeveloperModule"

void FPGXCoreDeveloperModule::StartupModule()
{
	// EN: Initialize developer tools and debug utilities.
	//     This module is stripped from Shipping builds automatically.
	// ES: Inicializar herramientas de desarrollador y utilidades de depuracion.
	//     Este modulo se elimina de los builds de Shipping automaticamente.
	UE_LOG(LogPGXCoreDeveloper, Log, TEXT("PGXCoreDeveloper: Module started"));
}

void FPGXCoreDeveloperModule::ShutdownModule()
{
	// EN: Cleanup developer tools before unloading
	// ES: Limpiar herramientas de desarrollador antes de descargar
	UE_LOG(LogPGXCoreDeveloper, Log, TEXT("PGXCoreDeveloper: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXCoreDeveloperModule, PGXCoreDeveloper)
