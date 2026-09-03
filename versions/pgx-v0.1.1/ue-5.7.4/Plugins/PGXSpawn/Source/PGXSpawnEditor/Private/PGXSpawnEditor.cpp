// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXSpawnEditor.h"
#include "Inspector/SPGXSpawnInspector.h"

void FPGXSpawnEditorModule::StartupModule()
{
	SPGXSpawnInspector::RegisterTabSpawner();
}

void FPGXSpawnEditorModule::ShutdownModule()
{
	SPGXSpawnInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXSpawnEditorModule, PGXSpawnEditor)
