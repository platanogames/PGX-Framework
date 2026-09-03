// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXInputEditor.h"
#include "Inspector/SPGXInputInspector.h"

void FPGXInputEditorModule::StartupModule()
{
	SPGXInputInspector::RegisterTabSpawner();
}

void FPGXInputEditorModule::ShutdownModule()
{
	SPGXInputInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXInputEditorModule, PGXInputEditor)
