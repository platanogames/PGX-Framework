// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

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
