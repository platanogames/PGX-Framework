// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXGameFlowEditor.h"
#include "Inspector/SPGXGameFlowInspector.h"

void FPGXGameFlowEditorModule::StartupModule()
{
	SPGXGameFlowInspector::RegisterTabSpawner();
}

void FPGXGameFlowEditorModule::ShutdownModule()
{
	SPGXGameFlowInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXGameFlowEditorModule, PGXGameFlowEditor)
