// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXMGOSEditor.h"
#include "Inspector/SPGXMGOSInspector.h"

void FPGXMGOSEditorModule::StartupModule()
{
	SPGXMGOSInspector::RegisterTabSpawner();
}

void FPGXMGOSEditorModule::ShutdownModule()
{
	SPGXMGOSInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXMGOSEditorModule, PGXMGOSEditor)
