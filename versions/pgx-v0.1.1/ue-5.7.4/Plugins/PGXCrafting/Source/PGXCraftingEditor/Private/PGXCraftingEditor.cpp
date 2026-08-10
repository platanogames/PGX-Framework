// Copyright PGX Framework. All Rights Reserved.

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
