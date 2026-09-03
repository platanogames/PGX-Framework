// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

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
