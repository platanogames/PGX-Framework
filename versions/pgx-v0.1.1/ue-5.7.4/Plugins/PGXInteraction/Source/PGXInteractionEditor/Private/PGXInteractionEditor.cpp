// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXInteractionEditor.h"
#include "Inspector/SPGXInteractionPanel.h"

void FPGXInteractionEditorModule::StartupModule()
{
	// EN: Register only the local NomadTab spawner. PGXCoreEditor owns the
	//     central PGXEditorStyle set lifecycle.
	// ES: Registrar solo el spawner local del NomadTab. PGXCoreEditor es
	//     propietario del ciclo de vida del style set central PGXEditorStyle.
	SPGXInteractionPanel::RegisterTabSpawner();
}

void FPGXInteractionEditorModule::ShutdownModule()
{
	SPGXInteractionPanel::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXInteractionEditorModule, PGXInteractionEditor)
