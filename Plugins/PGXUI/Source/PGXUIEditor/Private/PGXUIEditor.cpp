// Copyright PGX Framework. All Rights Reserved.

#include "PGXUIEditor.h"
#include "Inspector/SPGXUIInspector.h"

void FPGXUIEditorModule::StartupModule()
{
	SPGXUIInspector::RegisterTabSpawner();
}

void FPGXUIEditorModule::ShutdownModule()
{
	SPGXUIInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXUIEditorModule, PGXUIEditor)
