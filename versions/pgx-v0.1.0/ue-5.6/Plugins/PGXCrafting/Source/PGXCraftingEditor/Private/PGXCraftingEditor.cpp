// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXCraftingEditor.h"
#include "Inspector/SPGXCraftingInspector.h"

void FPGXCraftingEditorModule::StartupModule()
{
	SPGXCraftingInspector::RegisterTabSpawner();
}

void FPGXCraftingEditorModule::ShutdownModule()
{
	SPGXCraftingInspector::UnregisterTabSpawner();
}

IMPLEMENT_MODULE(FPGXCraftingEditorModule, PGXCraftingEditor)
