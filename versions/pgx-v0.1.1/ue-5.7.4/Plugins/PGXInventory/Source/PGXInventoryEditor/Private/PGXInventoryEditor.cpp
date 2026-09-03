// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

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
