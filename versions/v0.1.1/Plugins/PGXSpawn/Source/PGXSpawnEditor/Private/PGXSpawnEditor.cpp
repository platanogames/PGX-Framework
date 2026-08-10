// Copyright PGX Framework. All Rights Reserved.

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
