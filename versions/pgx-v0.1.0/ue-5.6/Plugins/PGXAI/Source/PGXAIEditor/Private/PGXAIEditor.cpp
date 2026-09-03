// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAIEditor.h"
#include "Inspector/SPGXAIInspector.h"

void FPGXAIEditorModule::StartupModule()
{
	SPGXAIInspector::RegisterTabSpawner();
}

void FPGXAIEditorModule::ShutdownModule()
{
	SPGXAIInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXAIEditorModule, PGXAIEditor)
