// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXPSOEditorModule.h"
#include "Inspector/SPGXPSOPanel.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogMacros.h"

#define LOCTEXT_NAMESPACE "FPGXPSOEditorModule"

void FPGXPSOEditorModule::StartupModule()
{
	// EN: Register the read-only PSO panel for CDO configuration and schema status.
	//     PGXCoreEditor owns the shared style lifecycle; this module owns its tab spawner.
	// ES: Registrar el panel PSO read-only para configuracion CDO y estado de schema.
	//     PGXCoreEditor gestiona el style compartido; este modulo gestiona su tab spawner.
	PGX_LOG_INFO(LogPGXPSO, TEXT("PGXPSOEditor: Module started"));
	SPGXPSOPanel::RegisterTabSpawner();
}

void FPGXPSOEditorModule::ShutdownModule()
{
	// EN: PGXPSOEditor module shut down. Unregister local NomadTab spawner
	//     only (style set ownership remains with PGXCoreEditor).
	// ES: Modulo PGXPSOEditor detenido. Solo desregistrar el spawner NomadTab
	//     local (la propiedad del style set permanece con PGXCoreEditor).
	SPGXPSOPanel::UnregisterTabSpawner();
	PGX_LOG_INFO(LogPGXPSO, TEXT("PGXPSOEditor: Module shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPGXPSOEditorModule, PGXPSOEditor)
