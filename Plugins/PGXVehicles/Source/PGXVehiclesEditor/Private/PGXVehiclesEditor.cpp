// Copyright PGX Framework. All Rights Reserved.

#include "PGXVehiclesEditor.h"
#include "Inspector/SPGXVehiclesInspector.h"

void FPGXVehiclesEditorModule::StartupModule()
{
	SPGXVehiclesInspector::RegisterTabSpawner();
}

void FPGXVehiclesEditorModule::ShutdownModule()
{
	SPGXVehiclesInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXVehiclesEditorModule, PGXVehiclesEditor)
