// Copyright PGX Framework. All Rights Reserved.

#include "PGXInventoryEditor.h"
#include "Panel/FPGXInventoryPanelTabSpawner.h"

#define LOCTEXT_NAMESPACE "PGXInventory"

IMPLEMENT_MODULE(FPGXInventoryEditorModule, PGXInventoryEditor)

void FPGXInventoryEditorModule::StartupModule()
{
	FPGXInventoryPanelTabSpawner::Register();
}

void FPGXInventoryEditorModule::ShutdownModule()
{
	FPGXInventoryPanelTabSpawner::Unregister();
}

#undef LOCTEXT_NAMESPACE
